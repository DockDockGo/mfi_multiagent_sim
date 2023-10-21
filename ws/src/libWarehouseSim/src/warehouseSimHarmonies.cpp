#include <libWarehouseSim/warehouseSimHarmonies.h>

// Yaml
#include <yaml-cpp/yaml.h>

// HARMONIES
#include <aws/core/client/ClientConfiguration.h>
#include <libHarmonies/Harmonies.h>

namespace libWarehouseSim {

Direction::Type convert(libHarmonies::Direction::Type direction)
{
  return (Direction::Type)direction;
}

libHarmonies::Direction::Type convert(Direction::Type direction)
{
  return (libHarmonies::Direction::Type)direction;
}

class WarehouseSimHarmoniesImpl
{
public:
  std::map<int, libHarmonies::Command> outstandingCommands;
  std::vector<libHarmonies::Command> cmds;
};

WarehouseSimHarmonies::WarehouseSimHarmonies(
  const std::string& configFile)
  : m_configFile(configFile)
  , m_client(nullptr)
  , m_metricsFile("metrics.csv")
{
  m_metricsFile << "t,newCollisions,cumulativeCollisions,stationUtilization"
                << ",driveLadenMoveTime,driveLadenStillTime,driveUnladenMoveTime,driveUnladenStillTime"
                << ",workCompleteCount,workSumCycleTime"
                << std::endl;
  m_pImpl = new WarehouseSimHarmoniesImpl;
}


WarehouseSimHarmonies::~WarehouseSimHarmonies()
{
  m_client->discard();
  delete m_client;

  Aws::SDKOptions options;
  // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
  Aws::ShutdownAPI(options);

  delete m_pImpl;
}

void WarehouseSimHarmonies::connect()
{
  YAML::Node cfg = YAML::LoadFile(m_configFile);

  Aws::SDKOptions options;
  // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
  Aws::InitAPI(options);

  Aws::Client::ClientConfiguration configuration;
  configuration.region = "us-west-2";
  configuration.scheme = Aws::Http::Scheme::HTTPS;
  m_client = new libHarmonies::HarmoniesClient(configuration, cfg["simid"].as<int>());
  try {
    m_client->discard();// discard any old simulation
  } catch (std::runtime_error& error) {

  }

  if (cfg["scenario"].as<std::string>() == "custom") {
    const auto& customCfg = cfg["customConfig"];
    m_client->start(
      customCfg["numDrives"].as<int>(),
      customCfg["numPods"].as<int>(),
      customCfg["numStations"].as<int>());
  } else {
    m_client->start(
      cfg["scenario"].as<std::string>(),
      cfg["seed"].as<int>());
  }

  libHarmonies::Configuration config = m_client->config();
  std::vector<libHarmonies::ReportMotionInfo> drives = m_client->peek("d*");
  std::vector<libHarmonies::ReportMotionInfo> pods = m_client->peek("p*");

  // fill libWarehouseSim datatypes
  int32_t xmin = std::numeric_limits<int32_t>::max();
  int32_t ymin = std::numeric_limits<int32_t>::max();
  int32_t xmax = std::numeric_limits<int32_t>::lowest();
  int32_t ymax = std::numeric_limits<int32_t>::lowest();

  for (const auto& wall : config.walls) {
    xmin = std::min(xmin, wall.first);
    xmax = std::max(xmax, wall.first);
    ymin = std::min(ymin, wall.second);
    ymax = std::max(ymax, wall.second);
  }

  std::unordered_set<Location> obstacles;
  // static obstacles are the possible position of pods
  // TODO: this should move somewhere else?!
  for (const auto& pod : pods) {
    obstacles.emplace(Location(pod.x, pod.y));
  }
  initMap(xmin + 1, ymin + 1, xmax - 1, ymax - 1, obstacles);

  for (const auto& drive : drives) {
    int id = std::stoi(drive.id.substr(1));
    m_robots.insert(std::make_pair(id,
      Robot(id,
        Location(drive.x, drive.y),
        Direction::fromAngle(drive.th),
        /*drive.attached*/-1
        )));
  }

  for (const auto& pod : pods) {
    int id = std::stoi(pod.id.substr(1));
    m_pods.insert(std::make_pair(id,
      Pod(id,
        Location(pod.x, pod.y))));
  }

  for (const auto& station : config.stations) {
    m_stations.insert(std::make_pair(station.id(),
      Station(station.id(),
        Location(station.x(), station.y()))));
  }

  // write config file
  std::ofstream harmoniesCfg("harmoniesConfig.yaml");
  harmoniesCfg << "robots:" << std::endl;
  for (const auto& r : m_robots) {
    harmoniesCfg << "  - name: d" << r.first << std::endl;
    harmoniesCfg << "    type: noPhysics" << std::endl;
    harmoniesCfg << "    position: [" << r.second.location().x / 2.0f << "," << r.second.location().y / 2.0f << "]" << std::endl;
  }
  harmoniesCfg << "pods:" << std::endl;
  for (const auto& p : m_pods) {
    harmoniesCfg << "  - name: p" << p.first << std::endl;
    harmoniesCfg << "    position: [" << p.second.location().x / 2.0f << "," << p.second.location().y / 2.0f << "]" << std::endl;
  }
  harmoniesCfg << "stations:" << std::endl;
  for (const auto& s : m_stations) {
    harmoniesCfg << "  - name: s" << s.first << std::endl;
    harmoniesCfg << "    position: [" << s.second.location().x / 2.0f << "," << s.second.location().y / 2.0f << "]" << std::endl;
  }
  harmoniesCfg << "map:" << std::endl;
  harmoniesCfg << "  xmin: " << xmin / 2.0f << std::endl;
  harmoniesCfg << "  xmax: " << xmax / 2.0f << std::endl;
  harmoniesCfg << "  ymin: " << ymin / 2.0f << std::endl;
  harmoniesCfg << "  ymax: " << ymax / 2.0f << std::endl;

  // for (const auto& job : m_client->jobs()) {
  //   m_jobs.insert(std::make_pair(job.first,
  //     Job(job.second.id(),
  //       m_pods.at(job.second.pod().id()),
  //       m_stations.at(job.second.station().id()),
  //       job.second.estimatedDurationAtStation(),
  //       Location(job.second.postStoreX(), job.second.postStoreY())
  //       )));
  // }

}

void WarehouseSimHarmonies::resume()
{
  m_client->resume();
}

void WarehouseSimHarmonies::pause()
{
  m_client->pause();
}

void WarehouseSimHarmonies::addAction(int robotId, int actionId, const Action& action, int expectedX, int expectedY, Direction::Type expectedDirection)
{
  libHarmonies::Command cmd;
  cmd.driveId = robotId;
  cmd.cmdId = /*(robotId << 24)*/1e6 * cmd.driveId + actionId;
  switch(action.type) {
    case Action::None:
      return;
    case Action::Straight:
      cmd.setStraight(action.additionalData.distance);
      break;
    case Action::Rotate:
      cmd.setRotate(convert(action.additionalData.direction));
      break;
    case Action::Attach:
      cmd.setAttach(action.additionalData.jobData.jobId, action.additionalData.jobData.podId);
      break;
    case Action::Detach:
      cmd.setDetach(action.additionalData.jobData.jobId, action.additionalData.jobData.podId);
      break;
    case Action::Yield:
      cmd.setYield(action.additionalData.jobData.jobId, action.additionalData.jobData.podId);
      break;
  }
  // std::vector<libHarmonies::Command> cmds;
  m_pImpl->cmds.push_back(cmd);
  std::cout << "Robot " << robotId << " enqueue: " << cmd << " with id: c" << actionId << std::endl;
  m_pImpl->outstandingCommands.insert(std::make_pair(cmd.cmdId, cmd));
  // m_client->command(cmds);
}

RunResult WarehouseSimHarmonies::runOnce()
{
  RunResult result;

  if (!m_pImpl->cmds.empty()) {
    m_client->command(m_pImpl->cmds);
    m_pImpl->cmds.clear();
  }

  libHarmonies::Reports reports = m_client->reports();

  for (const auto& job : reports.newJobs) {
    m_jobs.insert(std::make_pair(job.jobId,
          Job(job.jobId,
            m_pods.at(job.podId),
            m_stations.at(job.stationId),
            job.estDur,
            Location(job.postStore.first, job.postStore.second)
            )));
    result.newJobs.insert(job.jobId);
    std::cout << "New Job: " << job.jobId << " p" << job.podId << ",s" << job.stationId << std::endl;
  }

  for (const auto& cmd : reports.commands) {
    size_t cmdId = cmd.cmd - 1e6 * cmd.driveId;//cmd.cmd & 0xFFFFFF;
    result.commandsFinished[cmd.driveId].push_back(cmdId);
    result.commandFinishedInfos[cmd.driveId].push_back({(int)cmd.x, (int)cmd.y, Direction::fromAngle(cmd.th) });
    std::cout << "Robot " << cmd.driveId << " finished: " << m_pImpl->outstandingCommands.at(cmd.cmd) << " id: c" << cmdId << std::endl;
    m_pImpl->outstandingCommands.erase(cmd.cmd);
  }

  for (const auto& metric : reports.metrics) {
    m_metricsFile << metric.tStart
                  << "," << metric.newCollisions
                  << "," << metric.cumulativeCollisions
                  << "," << metric.stationUtilization
                  << "," << metric.driveLadenMoveTime
                  << "," << metric.driveLadenStillTime
                  << "," << metric.driveUnladenMoveTime
                  << "," << metric.driveUnladenStillTime
                  << "," << metric.workCompleteCount
                  << "," << metric.workSumCycleTime
                  << std::endl;
  }

  return result;
}

} // namespace libWarehouseSim


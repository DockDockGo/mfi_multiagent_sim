#include <experimental/filesystem>
#include <fstream>
#include <libWarehouseSim/warehouseSimGazebo.h>
#include <random>
// Yaml
#include <yaml-cpp/yaml.h>

#include <rpc/client.h>
#include <warehouse_gazebo_plugin/messages.h>

namespace libWarehouseSim {

static warehouse_gazebo_plugin::Orientation toPlugin(Direction::Type o) {
  switch (o) {
    case Direction::North:
      return warehouse_gazebo_plugin::Orientation::North;
    case Direction::South:
      return warehouse_gazebo_plugin::Orientation::South;
    case Direction::East:
      return warehouse_gazebo_plugin::Orientation::East;
    case Direction::West:
      return warehouse_gazebo_plugin::Orientation::West;
  }
}

static Direction::Type fromPlugin(warehouse_gazebo_plugin::Orientation o) {
  switch (o) {
    case warehouse_gazebo_plugin::Orientation::North:
      return Direction::North;
    case warehouse_gazebo_plugin::Orientation::South:
      return Direction::South;
    case warehouse_gazebo_plugin::Orientation::East:
      return Direction::East;
    case warehouse_gazebo_plugin::Orientation::West:
      return Direction::West;
  }
}

class WarehouseSimGazeboImpl {
 public:
  //
  WarehouseSimGazeboImpl(const std::string& serverAddress) : 
    client(serverAddress, 8081){
    readFromFile = true;
  }

  void connect(){
  }

  public:
  rpc::client client;
  std::string configFile;
  std::default_random_engine generator;
  std::uniform_int_distribution<int> genPod;
  bool readFromFile;
  std::vector<warehouse_gazebo_plugin::AddGoal> m_goals;
  int m_jobsPerStation;
  // std::vector<warehouse_gazebo_plugin::AddGoal> goals_copy;
};

WarehouseSimGazebo::WarehouseSimGazebo(const std::string &configFile,
                                       bool readFromFile, int jobsPerStation, const std::string &serverAddress)
    : m_pImpl(nullptr) {
  m_pImpl = new WarehouseSimGazeboImpl(serverAddress);
  m_pImpl->configFile = configFile;
  m_pImpl->readFromFile = readFromFile;
  m_pImpl->m_jobsPerStation = (jobsPerStation != -1) ? jobsPerStation : 10;
}

WarehouseSimGazebo::~WarehouseSimGazebo() { delete m_pImpl; }

void WarehouseSimGazebo::connect() {
  // initialize config from file
  int32_t xmin;
  int32_t ymin;
  int32_t xmax;
  int32_t ymax;

  try {
    YAML::Node cfg = YAML::LoadFile(m_pImpl->configFile);
    std::cout << "Loading file" << std::endl;
    // Load robots
    size_t robotId = 0;
    for (const auto &node : cfg["robots"]) {
      std::string name = node["name"].as<std::string>();
      const auto &pos = node["position"];
      float x = pos[0].as<float>();
      float y = pos[1].as<float>();
      m_robots.insert(std::make_pair(
          robotId,
          Robot(robotId, Location((int)round(x * 2), (int)round(y * 2)),
                // Location(x, y),
                Direction::East,
                /*drive.attached*/ -1)));
      ++robotId;
    }

    // Load pod models
    size_t podId = 0;
    for (const auto &node : cfg["pods"]) {
      const auto &pos = node["position"];
      // int x = pos[0].as<int>();
      // int y = pos[1].as<int>();
      float x = pos[0].as<float>();
      float y = pos[1].as<float>();
      m_pods.insert(std::make_pair(
          podId, Pod(podId, Location((int)round(x * 2), (int)round(y * 2)))));
      // Location(x, y))));
      ++podId;
    }

    // Load station models
    size_t stationId = 0;
    for (const auto &node : cfg["stations"]) {
      const auto &pos = node["position"];
      // int x = pos[0].as<int>();
      // int y = pos[1].as<int>();
      float x = pos[0].as<float>();
      float y = pos[1].as<float>();
      m_stations.insert(std::make_pair(
          stationId,
          Station(stationId, Location((int)round(x * 2), (int)round(y * 2)))));
      // Location(x, y))));
      ++stationId;
    }

    // xmin = cfg["map"]["xmin"].as<int>(); // * 2;
    // xmax = cfg["map"]["xmax"].as<int>(); // * 2;
    // ymin = cfg["map"]["ymin"].as<int>(); // * 2;
    // ymax = cfg["map"]["ymax"].as<int>(); // * 2;

    xmin = (int)round(cfg["map"]["xmin"].as<float>() * 2);
    xmax = (int)round(cfg["map"]["xmax"].as<float>() * 2);
    ymin = (int)round(cfg["map"]["ymin"].as<float>() * 2);
    ymax = (int)round(cfg["map"]["ymax"].as<float>() * 2);

  } catch (const YAML::Exception &e) {
    std::cerr << "The error is in the warehouse plugin" << std::endl;
    std::cerr << "YAML Error: " << e.what() << std::endl;
  }

  std::unordered_set<Location> obstacles;
  // static obstacles are the possible position of pods
  // TODO: this should move somewhere else?!
  for (const auto &pod : m_pods) {
    obstacles.insert(pod.second.location());
  }
  initMap(xmin + 1, ymin + 1, xmax - 1, ymax - 1, obstacles);
  std::cout << "Done" << std::endl;
  // populate some jobs
  // TODO: this should come from a file?

  if (!m_pImpl->readFromFile) {
    // std::uniform_int_distribution<int> genStation(0, m_stations.size() - 1);
    m_pImpl->genPod = std::uniform_int_distribution<int>(0, m_pods.size() - 1);
    for (size_t stationId = 0; stationId < m_stations.size(); ++stationId) {
      for (size_t jobId = 0; jobId < m_pImpl->m_jobsPerStation; ++jobId) {
        int podId = m_pImpl->genPod(m_pImpl->generator);
        m_jobs.insert(std::make_pair(
            m_jobs.size(),
            Job(m_jobs.size(), m_pods.at(podId), m_stations.at(stationId), 5.0,
                m_pods.at(podId).location())));
      }
    }
  } else {
    if (!std::ifstream("job_assignment.txt")) {
      // write the job file
      std::ofstream job_output("job_assignment.txt");
      m_pImpl->genPod =
          std::uniform_int_distribution<int>(0, m_pods.size() - 1);
      int jobCount = 0;
      for (size_t stationId = 0; stationId < m_stations.size(); ++stationId) {
        for (size_t jobId = 0; jobId < m_pImpl->m_jobsPerStation; ++jobId) {
          int podId = m_pImpl->genPod(m_pImpl->generator);
          job_output << jobCount << " " << podId << " " << stationId << " " << 5
                     << " " << m_pods.at(podId).location().x << " "
                     << m_pods.at(podId).location().y << std::endl;
          jobCount++;
        }
      }
    }
    // read the job file
    std::ifstream job_file("job_assignment.txt");
    while (true) {
      int jobId, podId, stationId, podX, podY;
      float stationDuration;
      if (job_file.eof()) break;
      job_file >> jobId >> podId >> stationId >> stationDuration >> podX >>
          podY;
      m_jobs.insert(std::make_pair(
          jobId, Job(jobId, m_pods.at(podId), m_stations.at(stationId),
                     stationDuration, m_pods.at(podId).location())));
    }
  }
}

void WarehouseSimGazebo::resume() {
  // TODO: not supported yet
}

void WarehouseSimGazebo::pause() {
  // TODO: not supported yet
}

void WarehouseSimGazebo::resetWorld() { m_pImpl->client.call("reset_world"); }

void WarehouseSimGazebo::addAction(int robotId, int actionId,
                                   const Action &action, int expectedX,
                                   int expectedY,
                                   Direction::Type expectedDirection) {
  warehouse_gazebo_plugin::AddGoal goal;
  goal.robotId = robotId;
  goal.actionId = actionId;
  switch (action.type) {
    case Action::Straight:
      goal.action = warehouse_gazebo_plugin::Action::Forward;
      break;
    case Action::Rotate:
      goal.action = warehouse_gazebo_plugin::Action::Rotate;
      break;
    case Action::Attach:
      goal.action = warehouse_gazebo_plugin::Action::LiftUp;
      break;
    case Action::Detach:
      goal.action = warehouse_gazebo_plugin::Action::LiftDown;
      break;
    case Action::Yield: {
      goal.action = warehouse_gazebo_plugin::Action::Yield;
      if (!m_pImpl->readFromFile) {
        const Job &job = m_jobs.at(action.additionalData.jobData.jobId);
        goal.stationId = job.station().id();

        // ToDO: maybe do this during detach instead?
        int podId = m_pImpl->genPod(m_pImpl->generator);

        m_jobs.insert(std::make_pair(
            m_jobs.size(),
            Job(m_jobs.size(), m_pods.at(podId), m_stations.at(goal.stationId),
                5.0, m_pods.at(podId).location())));
      }
    } break;
    default:
      std::cerr << "Action not implemented!" << std::endl;
      break;
  }
  // goal.x = expectedX; //* 0.5;
  // goal.y = expectedY; //* 0.5;

  goal.x = (double)expectedX * 0.5;
  goal.y = (double)expectedY * 0.5;
  goal.orientation = toPlugin(expectedDirection);
  goal.podId = action.additionalData.jobData.podId;

  m_pImpl->m_goals.push_back(goal);
  // goals_copy.push_back(goal);
}

RunResult WarehouseSimGazebo::runOnce() {
  RunResult result;

  if (!m_pImpl->m_goals.empty()) {
    m_pImpl->client.call("add_goals", m_pImpl->m_goals);
    std::cout << "warehouseSimGazebo.cpp: Sent the commands" << std::endl;
    m_pImpl->m_goals.clear();
  }

  std::vector<warehouse_gazebo_plugin::FinishedGoal> goals =
      m_pImpl->client.call("get_finished_goals")
          .as<std::vector<warehouse_gazebo_plugin::FinishedGoal>>();
  for (const auto &goal : goals) {
    result.commandsFinished[goal.robotId].push_back(goal.actionId);
    // std::cout << "warehouseSimGazebo.cpp: Adding goal for robotId "
    // << goal.robotId << std::endl;
    // std::cout << goal.x << " " << goal.y << std::endl;
    result.commandFinishedInfos[goal.robotId].push_back(
        // {goal.x, goal.y, fromPlugin(goal.orientation)});
        {(int)round(goal.x * 2.0), (int)round(goal.y * 2.0),
         fromPlugin(goal.orientation)});
  }
  std::vector<warehouse_gazebo_plugin::DetectedObstacle> detectedObstacles =
      m_pImpl->client.call("get_detected_obstacles")
          .as<std::vector<warehouse_gazebo_plugin::DetectedObstacle>>();
  for (const auto &obs : detectedObstacles) {
    result.detectedObstacles.push_back({

        (int)obs.robotId,
        // (obs.obstacleX), // * 2.0),
        // (obs.obstacleY), // * 2.0),
        // (obs.x),         // * 2.0),
        // (obs.y),         // * 2.0),
        // (int)obs.robotId,
        (int)round(obs.obstacleX * 2.0), (int)round(obs.obstacleY * 2.0),
        (int)round(obs.x * 2.0), (int)round(obs.y * 2.0),
        fromPlugin(obs.orientation)});
    // addObstacle(Location(obs.obstacleX, obs.obstacleY));
    addObstacle(Location((int)round(obs.obstacleX * 2.0),
                         (int)round(obs.obstacleY * 2.0)));
  }
  return result;
}

float WarehouseSimGazebo::getSimTime() {
  return m_pImpl->client.call("get_sim_time").as<float>();
}

}  // namespace libWarehouseSim

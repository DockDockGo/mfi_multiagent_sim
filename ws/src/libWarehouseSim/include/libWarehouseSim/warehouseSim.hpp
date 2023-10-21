#pragma once
#include <string>
#include <iostream>

#include <libWarehouseSim/action.hpp>
#include <libWarehouseSim/map.hpp>
#include <libWarehouseSim/robot.hpp>
#include <libWarehouseSim/job.hpp>
#include <libWarehouseSim/pod.hpp>
#include <libWarehouseSim/station.hpp>

namespace libWarehouseSim {

struct RunResult
{
  std::map<int, std::vector<int> > commandsFinished; // commandIds finished per robot

  // state after execution
  struct CommandFinishedInfo {
    float x;
    float y;
    Direction::Type direction;
  };
  std::map<int, std::vector<CommandFinishedInfo> > commandFinishedInfos;

  std::set<int> newJobs; // newly added jobids

  struct DetectedObstacle
  {
    int robotId;
    float obstacleX;
    float obstacleY;
    float robotX;
    float robotY;
    Direction::Type direction;
  };
  std::vector<DetectedObstacle> detectedObstacles;

};

class WarehouseSim
{
public:

  // connect to simulator
  virtual void connect() = 0;

  virtual void resume() = 0;
  virtual void pause() = 0;

  virtual void addAction(int robotId, int actionId, const Action& action, int expectedX, int expectedY, Direction::Type expectedDirection) = 0;

  // communicates with simulator
  // might trigger callback function regarding state changes
  virtual RunResult runOnce() = 0;

  // query functions
  const Map& map() const
  {
    return m_map;
  }

  const std::map<int, Robot>& robots() const
  {
    return m_robots;
  }

  const std::map<int, Job>& jobs() const
  {
    return m_jobs;
  }

  const std::map<int, Pod>& pods() const
  {
    return m_pods;
  }

  const std::map<int, Station>& stations() const
  {
    return m_stations;
  }

protected:
  void initMap(
    int minx,
    int miny,
    int maxx,
    int maxy,
    const std::unordered_set<Location>& obstacles)
  {
    m_map.m_minx = minx;
    m_map.m_miny = miny;
    m_map.m_maxx = maxx;
    m_map.m_maxy = maxy;
    m_map.m_obstacles = obstacles;
  }

  void addObstacle(const Location& location)
  {
    m_map.m_obstacles.insert(location);
  }

protected:
  Map m_map;
  std::map<int, Robot> m_robots;
  std::map<int, Job> m_jobs;
  std::map<int, Pod> m_pods;
  std::map<int, Station> m_stations;
  
};

} // namespace libWarehouseSim


#pragma once
#include <string>
#include <iostream>

#include <libWarehouseSim/warehouseSim.hpp>

namespace libWarehouseSim {

class WarehouseSimGazeboImpl;

// Implementation for Gazebo simulator
class WarehouseSimGazebo : public WarehouseSim
{
public:
  WarehouseSimGazebo(
    const std::string& configFile, bool readFromFile, int jobsPerStation, const std::string& serverAddress);
  virtual ~WarehouseSimGazebo();

  virtual void connect();

  virtual void resume();

  virtual void pause();

  virtual void addAction(int robotId, int actionId, const Action& action, int expectedX, int expectedY, Direction::Type expectedDirection);

  virtual RunResult runOnce();

  float getSimTime();
  void resetWorld();

private:
  WarehouseSimGazeboImpl* m_pImpl;
};

} // namespace libWarehouseSim


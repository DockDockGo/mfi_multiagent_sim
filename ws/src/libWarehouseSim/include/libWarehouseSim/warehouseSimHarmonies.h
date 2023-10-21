#pragma once
#include <string>
#include <iostream>

#include <libWarehouseSim/warehouseSim.hpp>

// forward declaration
namespace libHarmonies {
  class HarmoniesClient;
} // namespace libHarmonies

namespace libWarehouseSim {

class WarehouseSimHarmoniesImpl;

// Implementation for HARMONIES simulator
class WarehouseSimHarmonies : public WarehouseSim
{
public:
  WarehouseSimHarmonies(
    const std::string& configFile);
  virtual ~WarehouseSimHarmonies();

  virtual void connect();

  virtual void resume();

  virtual void pause();

  virtual void addAction(int robotId, int actionId, const Action& action, int expectedX, int expectedY, Direction::Type expectedDirection);

  virtual RunResult runOnce();

private:
  std::string m_configFile;
  libHarmonies::HarmoniesClient* m_client;
  std::ofstream m_metricsFile;

  WarehouseSimHarmoniesImpl* m_pImpl;

};

} // namespace libWarehouseSim


#pragma once
#include <string>
#include <iostream>

#include <libWarehouseSim/location.hpp>
#include <libWarehouseSim/pod.hpp>
#include <libWarehouseSim/station.hpp>

namespace libWarehouseSim {

class Job
{
public:
  enum State
  {
    Unassigned,   // initial state
    PodPickedUp,  // pod was picked up by a drive
    PodDelivered, // pod was delivered to station
    Finished,     // job has been finished
  };

  Job(
    int id,
    const Pod& pod,
    const Station& station,
    float estimatedDurationAtStation,
    const Location& postStore)
    : m_id(id)
    , m_pod(pod)
    , m_station(station)
    , m_estimatedDurationAtStation(estimatedDurationAtStation)
    , m_postStore(postStore)
    , m_state(Unassigned)
  {
  }

  int id() const
  {
    return m_id;
  }

  const Pod& pod() const
  {
    return m_pod;
  }

  const Station& station() const
  {
    return m_station;
  }

  float estimatedDurationAtStation() const
  {
    return m_estimatedDurationAtStation;
  }

  const Location& postStore() const
  {
    return m_postStore;
  }

  State state() const
  {
    return m_state;
  }

  friend std::ostream& operator<<(std::ostream& os, const Job& j)
  {
    os << "Job " << j.m_id;
    return os;
  }

private:
  int m_id;
  const Pod& m_pod;
  const Station& m_station;
  float m_estimatedDurationAtStation;
  Location m_postStore;
  State m_state;
};

} // namespace libWarehouseSim


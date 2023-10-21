#pragma once
#include <string>
#include <iostream>
#include <fstream>

#include <libWarehouseSim/direction.hpp>

namespace libWarehouseSim {

class Robot
{
public:
  Robot(int id,
        const Location& location,
        Direction::Type direction,
        int attachedPod = -1)
    : m_id(id)
    , m_location(location)
    , m_direction(direction)
    , m_attachedPod(attachedPod)
  {

  }

  int id() const
  {
    return m_id;
  }

  const Location& location() const
  {
    return m_location;
  }

  Direction::Type direction() const
  {
    return m_direction;
  }

  int attachedPod() const
  {
    return m_attachedPod;
  }

protected:
  int m_id;
  Location m_location;
  Direction::Type m_direction;
  int m_attachedPod;
};

} // namespace libWarehouseSim

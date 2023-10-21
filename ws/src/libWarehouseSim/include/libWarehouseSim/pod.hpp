#pragma once
#include <string>
#include <iostream>

#include <libWarehouseSim/location.hpp>

namespace libWarehouseSim {

class Pod
{
public:
  Pod(
    int id,
    const Location& location)
    : m_id(id)
    , m_location(location)
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

  friend std::ostream& operator<<(std::ostream& os, const Pod& p)
  {
    os << "Pod " << p.m_id;
    return os;
  }

private:
  int m_id;
  Location m_location;
};

} // namespace libWarehouseSim

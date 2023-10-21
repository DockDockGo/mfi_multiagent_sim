#pragma once

#include <unordered_set>

#include <libWarehouseSim/location.hpp>

namespace libWarehouseSim {

class WarehouseSim;
class Map
{
public:
  friend class WarehouseSim;
  // Map(
  //   int minx,
  //   int miny,
  //   int maxx,
  //   int maxy,
  //   std::unordered_set<Location> obstacles)
  //   : m_minx(minx)
  //   , m_miny(miny)
  //   , m_maxx(maxx)
  //   , m_maxy(maxy)
  //   , m_obstacles(obstacles)
  // {
  // }

  bool locationValid(
    const Location& s)
  {
    return    s.x >= m_minx
           && s.x <= m_maxx
           && s.y >= m_miny
           && s.y <= m_maxy
           && m_obstacles.find(s) == m_obstacles.end();
  }

  int minx() const
  {
    return m_minx;
  }

  int maxx() const
  {
    return m_maxx;
  }

  int miny() const
  {
    return m_miny;
  }

  int maxy() const
  {
    return m_maxy;
  }

  const std::unordered_set<Location>& obstacles() const
  {
    return m_obstacles;
  }

private:
  int m_minx;
  int m_miny;
  int m_maxx;
  int m_maxy;
  std::unordered_set<Location> m_obstacles;
};

} // namespace libWarehouseSim

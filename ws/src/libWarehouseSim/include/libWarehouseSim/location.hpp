#pragma once

#include <boost/functional/hash.hpp>

namespace libWarehouseSim {

struct Location
{
  Location(int x, int y) : x(x), y(y) {}
  int x;
  int y;

  bool operator<(const Location& other) const {
    return std::tie(x, y) < std::tie(other.x, other.y);
  }

  bool operator==(const Location& other) const {
    return std::tie(x, y) == std::tie(other.x, other.y);
  }

  bool operator!=(const Location& other) const {
    return std::tie(x, y) != std::tie(other.x, other.y);
  }

  friend std::ostream& operator<< ( std::ostream& os, const Location& c)
  {
    return os << "(" << c.x << "," << c.y << ")";
  }
};

} // namespace libWarehouseSim

namespace std
{
template<>
struct hash<libWarehouseSim::Location> {
    size_t operator()(const libWarehouseSim::Location& s) const {
      size_t seed = 0;
      boost::hash_combine(seed, s.x);
      boost::hash_combine(seed, s.y);
      return seed;
    }
};
}

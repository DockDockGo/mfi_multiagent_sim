#pragma once
#include <string>
#include <math.h>

namespace libWarehouseSim {

class Direction {
public:
  enum Type {
    North,
    South,
    East,
    West,
  };

  static std::string toString(Type t)
  {
    switch(t)
    {
      case North:
        return "NORTH";
      case South:
        return "SOUTH";
      case East:
        return "EAST";
      case West:
        return "WEST";
    }
  }

  static Direction::Type fromAngle(float theta)
  {
    if (fabs(smallestAngle(theta, M_PI / 2.0)) < 1e-3) {
      return Direction::North;
    }
    if (fabs(smallestAngle(theta, -M_PI / 2.0)) < 1e-3) {
      return Direction::South;
    }
    if (fabs(smallestAngle(theta, 0.0)) < 1e-3) {
      return Direction::East;
    }
    return Direction::West;
  }

  static float toAngle(Direction::Type t)
  {
    switch(t)
    {
    case Direction::North:
      return M_PI / 2.0;
    case Direction::South:
      return -M_PI / 2.0;
    case Direction::East:
      return 0;
    case Direction::West:
      return M_PI;
    }
  }

  friend std::ostream& operator<<(std::ostream& os, const Direction::Type& d)
  {
    os << Direction::toString(d);
    return os;
  }

private:
  static float smallestAngle(float angle1, float angle2)
  {
    return atan2(sin(angle1 - angle2), cos(angle1 - angle2));
  }
};

} // namespace libWarehouseSim

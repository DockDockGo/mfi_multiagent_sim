#pragma once
#include <string>
#include <vector>

#include <libWarehouseSim/direction.hpp>

namespace libWarehouseSim {

class Action {
public:
  enum Type {
    None,
    Straight,
    Rotate,
    Attach,
    Detach,
    Yield,
    Up,
    Down,
    Left,
    Right
  };

  union additionalData {
    double distance;           // for Straight
    Direction::Type direction; // for rotate
    struct {                   // for attach/detach/yield
      int podId;
      int jobId;
    } jobData;
  };

  Action() : type(None), additionalData() {}

  void setStraight(double distance) {
    type = Straight;
    additionalData.distance = distance;
  }

  void setRotate(Direction::Type targetDirection) {
    type = Rotate;
    additionalData.direction = targetDirection;
  }

  void setAttach(int jobId, int podId) {
    type = Attach;
    additionalData.jobData.jobId = jobId;
    additionalData.jobData.podId = podId;
  }

  void setDetach(int jobId, int podId) {
    type = Detach;
    additionalData.jobData.jobId = jobId;
    additionalData.jobData.podId = podId;
  }

  void setYield(int jobId, int podId) {
    type = Yield;
    additionalData.jobData.jobId = jobId;
    additionalData.jobData.podId = podId;
  }

  void setUp(double distance) {
    type = Up;
    additionalData.distance = distance;
  }

  void setDown(double distance) {
    type = Down;
    additionalData.distance = distance;
  }

  void setLeft(double distance) {
    type = Left;
    additionalData.distance = distance;
  }

  void setRight(double distance) {
    type = Right;
    additionalData.distance = distance;
  }

  friend std::ostream &operator<<(std::ostream &os, const Action &a) {
    switch (a.type) {
    case Straight:
      os << "Straight(" << a.additionalData.distance << ")";
      break;
    case Rotate:
      os << "Rotate(" << a.additionalData.direction << ")";
      break;
    case Attach:
      os << "Attach(j" << a.additionalData.jobData.jobId << ",p"
         << a.additionalData.jobData.podId << ")";
      break;
    case Detach:
      os << "Detach(j" << a.additionalData.jobData.jobId << ",p"
         << a.additionalData.jobData.podId << ")";
      break;
    case Yield:
      os << "Yield(j" << a.additionalData.jobData.jobId << ",p"
         << a.additionalData.jobData.podId << ")";
      break;
    case Up:
      os << "Up(j" << a.additionalData.jobData.jobId << ",p"
         << a.additionalData.jobData.podId << ")";
      break;
    case Down:
      os << "Down(j" << a.additionalData.jobData.jobId << ",p"
         << a.additionalData.jobData.podId << ")";
      break;
    case Left:
      os << "Left(j" << a.additionalData.jobData.jobId << ",p"
         << a.additionalData.jobData.podId << ")";
      break;
    case Right:
      os << "Right(j" << a.additionalData.jobData.jobId << ",p"
         << a.additionalData.jobData.podId << ")";
      break;
    case None:
      os << "None";
      break;
    }
    return os;
  }

  Type type;
  additionalData additionalData;
};

} // namespace libWarehouseSim

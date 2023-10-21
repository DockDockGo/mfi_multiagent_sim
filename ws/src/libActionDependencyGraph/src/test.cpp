#include <iostream>

#include "libActionDependencyGraph/types.hpp"
#include "libActionDependencyGraph/helper_functions.hpp"
#include "libActionDependencyGraph/action_dependency_graph.hpp"

enum class Orientation
{
  North,
  South,
  East,
  West,
};

std::ostream& operator<<(std::ostream& os, const Orientation& o)
{
  switch (o)
  {
    case Orientation::North:
      os << "North";
      break;
    case Orientation::South:
      os << "South";
      break;
    case Orientation::East:
      os << "East";
      break;
    case Orientation::West:
      os << "West";
      break;
  }
  return os;
}

struct State
{
  int x;
  int y;
  Orientation direction;
};

std::ostream& operator<<(std::ostream& os, const State& s)
{
  os << "(" << s.x << "," << s.y << "," << s.direction << ")";
  return os;
}

enum class Action
{
  Forward,
  TurnLeft,
  TurnRight,
};

std::ostream& operator<<(std::ostream& os, const Action& a)
{
  switch (a)
  {
    case Action::Forward:
      os << "Forward";
      break;
    case Action::TurnLeft:
      os << "TurnLeft";
      break;
    case Action::TurnRight:
      os << "TurnRight";
      break;
  }
  return os;
}


int main()
{
  using namespace libActionDependencyGraph;
  typedef ActionDependencyGraph<Action, State> ADG;

  std::vector<State> startStates(2);
  startStates[0] = {1, 1, Orientation::East};
  startStates[1] = {0, 1, Orientation::East};
  ADG adg(startStates);

  std::vector<ActionDependencyGraph<Action, State>::RobotSolution> solutions(2);

  solutions[0].actions.resize(8);
  solutions[0].actions[0] = { {1, 1, Orientation::East} , Action::Forward  , 0};
  solutions[0].actions[1] = { {2, 1, Orientation::East} , Action::TurnRight, 1};
  solutions[0].actions[2] = { {2, 1, Orientation::South}, Action::Forward  , 2};
  solutions[0].actions[3] = { {2, 0, Orientation::South}, Action::TurnRight, 3};
  solutions[0].actions[4] = { {2, 0, Orientation::West} , Action::TurnRight, 4};
  solutions[0].actions[5] = { {2, 0, Orientation::North}, Action::Forward  , 5};
  solutions[0].actions[6] = { {2, 1, Orientation::North}, Action::TurnRight, 6};
  solutions[0].actions[7] = { {2, 1, Orientation::East} , Action::Forward  , 7};
  solutions[0].finalState = {3, 1, Orientation::East};

  solutions[1].actions.resize(4);
  solutions[1].actions[0] = { {0, 1, Orientation::East} , Action::Forward  , 0};
  solutions[1].actions[1] = { {1, 1, Orientation::East} , Action::Forward  , 2};
  solutions[1].actions[2] = { {2, 1, Orientation::East} , Action::Forward  , 3};
  solutions[1].actions[3] = { {3, 1, Orientation::East} , Action::Forward  , 4};
  solutions[1].finalState = {4, 1, Orientation::East};

  adg.appendSolution(solutions);

  std::vector< std::vector< ADG::ActionInformation > > actionsToEnqueue;

  adg.getActionsToEnqueue(actionsToEnqueue, /*maxQueue*/ 100);

  for (size_t i = 0; i < actionsToEnqueue.size(); ++i) {
    std::cout << "Robot " << i << std::endl;
    for (const auto& info : actionsToEnqueue[i]) {
      std::cout << "  " << info.action << std::endl;
    }
  }

  std::vector< std::vector< std::pair<size_t,State> > > actionsFinishedPerRobot(2);
  actionsFinishedPerRobot[0].push_back( std::make_pair<>(actionsToEnqueue[0][0].actionId, actionsToEnqueue[0][0].expectedState));

  adg.markActionsFinished(actionsFinishedPerRobot);

  adg.getActionsToEnqueue(actionsToEnqueue, /*maxQueue*/ 100);

  for (size_t i = 0; i < actionsToEnqueue.size(); ++i) {
    std::cout << "Robot " << i << std::endl;
    for (const auto& info : actionsToEnqueue[i]) {
      std::cout << "  " << info.action << std::endl;
    }
  }

  return 0;
}

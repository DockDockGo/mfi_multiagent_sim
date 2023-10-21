#pragma once

#include "libActionDependencyGraph/helper_functions.hpp"
#include "libActionDependencyGraph/types.hpp"


namespace libActionDependencyGraph {

class NextVertexNotFoundException {};

template <typename Action, typename State> class ActionDependencyGraph {
public:
  struct SolutionEntry {
    State state;   // starting state
    Action action; // action to take
    double time;   // time when to execute the action
  };
  struct RobotSolution {
    std::vector<SolutionEntry>
        actions;      // starting states and actions to execute
    State finalState; // state after last action was executed
  };
  // typedef std::vector<SolutionEntry> RobotSolution;

public:
  ActionDependencyGraph(const std::vector<State> &states, bool isSIPP = false) {
    robotStates.resize(states.size());
    for (size_t i = 0; i < states.size(); ++i) {
      robotStates[i].finalState = states[i];
    }
    m_isSIPP = isSIPP;
  }

  // append new solution at the end of the graph
  // for each robot we get one solution
  void appendSolution(const std::vector<RobotSolution> &solutions) {
    assert(solutions.size() == robotStates.size());
    // std::lock_guard<std::mutex> lock(mutex);

    // add vertices and type 1 edges
    for (size_t i = 0; i < robotStates.size(); ++i) {
      Vertex<Action, State> v;
      v.robotId = i;
      // v.xpos = 0;
      const auto &solution = solutions[i];
      for (size_t j = 0; j < solution.actions.size(); ++j) {
        v.state = solution.actions[j].state;
        v.action = solution.actions[j].action;
        v.time = solution.actions[j].time;
        v.actionId = (++robotStates[i].actionId);
        vertex_t u = boost::add_vertex(v, g);
        if (robotStates[i].lastVertex) {
          // g[m_lastVertices[v.driveId]].next = u;
          auto e = boost::add_edge(robotStates[i].lastVertex, u, g);
          g[e.first].type = Edge::Type::Type1;
        } else {
          robotStates[i].firstVertex = u;
        }
        if (robotStates[i].currentVertex == nullptr) {
          robotStates[i].currentVertex = u;
        }
        if (robotStates[i].finishedVertex == nullptr) {
          robotStates[i].finishedVertex = u;
        }
        robotStates[i].lastVertex = u;
        robotStates[i].remainingActions += 1;
        // g[u].userData = new State(solution.states[j].first);
      }
      robotStates[i].finalState = solutions[i].finalState;
    }

    // type 2 edges
    
    for (size_t r1 = 0; r1 < robotStates.size(); ++r1) {
      vertex_t v1a = robotStates[r1].firstVertex;
      while (v1a) {
        vertex_t v1b = nextVertex(g, v1a);
        const State &s1a = g[v1a].state;
        const State &s1b = v1b ? g[v1b].state : robotStates[r1].finalState;
        if ((s1a.x != s1b.x || s1a.y != s1b.y)) {
          for (size_t r2 = 0; r2 < solutions.size(); ++r2) {
            if (r1 != r2) {
              vertex_t v2a = robotStates[r2].firstVertex;
              while (v2a) {
                vertex_t v2b = nextVertex(g, v2a);
                const State &s2a = g[v2a].state;
                const State &s2b =
                    v2b ? g[v2b].state : robotStates[r2].finalState;
                if (s1a.x == s2b.x && s1a.y == s2b.y &&
                    g[v1a].time <= g[v2a].time) {
                  if (!edge(v1a, v2a, g).second) {
                    auto e = add_edge(v1a, v2a, g);
                    g[e.first].type = Edge::Type::Type2;
                  }
                  // finished with this agent
                  break;
                }
                v2a = v2b;
              }
            }
          }
        }
        v1a = v1b;
      }
    }

    static int iter = 0;
    std::stringstream sstr;
    sstr << "adg_appendSolution_iter" << iter << ".dot";
    // printGraph(sstr.str());
    printGraph("/home/yjt/Documents/test.txt");
    assert(isValid(g));
    ++iter;
  }

  // find commands to send in this iteration
  // we can start all commands that have a finished type1 and type2 incoming
  // edges
  struct ActionInformation {
    size_t actionId;
    State initialState;  // state before action is executed
    Action action;       // action to execute
    State expectedState; // state after action is executed
  };
  void getActionsToEnqueue(std::vector<std::vector<ActionInformation>> &result,
                           size_t maxQueue) {
//      printf("Get actions to enqueue\n");
    result.resize(robotStates.size());
    for (size_t i = 0; i < robotStates.size(); ++i) {
      result[i].clear();
      vertex_t vertex = robotStates[i].currentVertex;
//        printf("Agent %d, has states num: %d\n", i, robotStates[i].queuedActions);
      while (vertex) {
        bool canStart = true;
        if (robotStates[i].queuedActions >= maxQueue) {
          canStart = false;
        }
//        printf("Agent %d, can start 1: %d\n", i, canStart);
        auto es = in_edges(vertex, g);
        for (auto eit = es.first; eit != es.second; ++eit) {
          edge_t e = *eit;
          if (g[e].type == Edge::Type::Type2 &&
              g[source(e, g)].executionState !=
                  Vertex<Action, State>::ExecutionState::Completed) {
            // std::cout << std::endl;
            // std::cout << "Stopping execution for robot " << i << " at state "
            // << g[vertex].state << " because of type 2 edge from "
            // << g[source(e, g)].state << std::endl;
            canStart = false;
            break;
          }
        }
//          printf("Agent %d, can start 2: %d\n", i, canStart);
        if (canStart) {
          const State &initialState = g[vertex].state;
          const Action &action = g[vertex].action;
          vertex_t n = nextVertex(g, vertex);
          const State &expectedState =
              n ? g[n].state : robotStates[i].finalState;
          result[i].push_back(
              {g[vertex].actionId, initialState, action, expectedState});
//          std::cout <<  "Action ID: " << g[vertex].actionId <<
//                      ", Action: " << action <<
//                      ", Init state: " << initialState
//                      << ", goal state: "
//                      << expectedState;
          robotStates[i].queuedActions += 1;
          assert((g[vertex].executionState ==
                  Vertex<Action, State>::ExecutionState::NotStarted));
          g[vertex].executionState =
              Vertex<Action, State>::ExecutionState::Enqueued;
          bool isCommittedVertex = vertex == robotStates[i].committedVertex;
          vertex = n;
          robotStates[i].currentVertex = vertex;
          // stop at committed vertex
          if (isCommittedVertex) {
            break;
          }
        } else {
          break;
        }
      }
    }

    static int iter = 0;
    std::stringstream sstr;
    sstr << "adg_getActionsToEnqueue_iter" << iter << ".dot";
    writeDotFile(sstr.str());
    // printGraph("adg_actions.dot");
    ++iter;
  }

  // update the graph with finished actions
  void
  markActionsFinished(const std::vector<std::vector<std::pair<size_t, State>>>
                          &actionsFinishedPerRobot) {
    assert(actionsFinishedPerRobot.size() == robotStates.size());
    for (size_t i = 0; i < robotStates.size(); ++i) {
      for (size_t j = 0; j < actionsFinishedPerRobot[i].size(); ++j) {
        vertex_t v = robotStates[i].finishedVertex;
        if (g[v].actionId != actionsFinishedPerRobot[i][j].first) {
            printf("For agent %lu, with %lu-th action, Action id in graph is: %lu, finished action is: %lu\n",
                   i, j, g[v].actionId, actionsFinishedPerRobot[i][j].first);
            printGraph("/home/yjt/Documents/test.txt");
            exit(-1);
        }

        assert(v);
        assert(g[v].actionId == actionsFinishedPerRobot[i][j].first);
        assert((g[v].executionState ==
                Vertex<Action, State>::ExecutionState::Enqueued));

        vertex_t next = nextVertex(g, v);
        const State &expectedState =
            next ? g[next].state : robotStates[i].finalState;
        // std::cout << "Expected vs received" << std::endl;

        // std::cout << expectedState.x << " " << expectedState.y << " "
        //           << expectedState.direction << "/"
        //           << actionsFinishedPerRobot[i][j].second.x << " "
        //           << actionsFinishedPerRobot[i][j].second.y << " "
        //           << actionsFinishedPerRobot[i][j].second.direction
        //           << std::endl;
        // std::cout << "Finishing action : "
        //           << actionsFinishedPerRobot[i][j].second << " for robot " <<
        //           i
        //           << std::endl;
        // std::cout << "Number of remaining actions for robot i: "
        //           << robotStates[i].remainingActions - 1 << std::endl;
        assert(expectedState.x == actionsFinishedPerRobot[i][j].second.x);
        assert(expectedState.y == actionsFinishedPerRobot[i][j].second.y);
        assert(expectedState.direction ==
               actionsFinishedPerRobot[i][j].second.direction);

        g[v].executionState = Vertex<Action, State>::ExecutionState::Completed;
        robotStates[i].finishedVertex = nextVertex(g, v);
        robotStates[i].remainingActions -= 1;
        assert(robotStates[i].remainingActions >= 0);
        robotStates[i].queuedActions -= 1;
        assert(robotStates[i].queuedActions >= 0);
      }
    }

    static int iter = 0;
    std::stringstream sstr;
    sstr << "adg_markActionsFinished_iter" << iter << ".dot";
    writeDotFile(sstr.str());
    ++iter;
  }

  // Update graph after a robot had to abort an action in its queue
  // (for example, in case of a newly detected obstacle on the route)
  // all non-finished actions for that agent will be removed
  void abortActions(size_t robotId) {
    auto &rs = robotStates[robotId];

    if (rs.finishedVertex) {
      rs.lastVertex = previousVertex(g, rs.finishedVertex);
      rs.finalState = g[rs.finishedVertex].state;
      vertex_t v = rs.finishedVertex;
      while (v) {
        vertex_t next = nextVertex(g, v);
        boost::clear_vertex(v, g);
        boost::remove_vertex(v, g);
        v = next;
      }
    }

    if (rs.firstVertex == rs.finishedVertex) {
      rs.firstVertex = nullptr;
    }
    rs.committedVertex = nullptr;
    rs.currentVertex = nullptr;
    rs.finishedVertex = nullptr;

    rs.queuedActions = 0;
    rs.remainingActions = 0;

    static int iter = 0;
    std::stringstream sstr;
    sstr << "adg_abortActions_iter" << iter << ".dot";
    writeDotFile(sstr.str());
    ++iter;
  }

  // Cleans the graph such that:
  //  a) only vertices and edges are deleted that are at least enqueued already
  //  b) the start time of all vertices is the same
  //  c) all not yet completed type2 dependencies are captured
  void removeFinished() {
    // find earliest time
    double startTime = std::numeric_limits<double>::max();
    for (const auto &rs : robotStates) {
      if (rs.finishedVertex) {
        startTime = std::min<double>(g[rs.finishedVertex].time, startTime);
      }
    }
    std::cout << "remove vertices before t=" << startTime << std::endl;

    // for all drives, remove vertices and edges before that time
    for (auto &rs : robotStates) {
      vertex_t v = rs.firstVertex;
      while (v && g[v].time < startTime) {
        if (v == rs.committedVertex) {
          rs.committedVertex = nullptr;
        }
        if (v == rs.currentVertex) {
          rs.currentVertex = nullptr;
        }
        if (v == rs.finishedVertex) {
          rs.finishedVertex = nullptr;
        }
        assert((g[v].executionState ==
                Vertex<Action, State>::ExecutionState::Completed));
        vertex_t next = nextVertex(g, v);
        boost::clear_vertex(v, g);
        boost::remove_vertex(v, g);
        v = next;
      }
      rs.firstVertex = v;
      if (!v) {
        rs.lastVertex = nullptr;
      }
    }

    static int iter = 0;
    std::stringstream sstr;
    sstr << "adg_removeFinished_iter" << iter << ".dot";
    writeDotFile(sstr.str());
    ++iter;
  }

  // finds save points for committedVertex such that
  //  a) all dependencies from start to (including) committedVertex can be
  //  fulfilled (i.e. execution is possible) b) there is a lookahead (from
  //  currentVertices) of at least the provided number c) specified drives will
  //  keep the old plan until the end
  // Returns the state the robot will be in after finishing the last action
  void findCommittedVertex(int lookahead, std::set<size_t> driveIdsToFinish,
                           std::vector<State> &statesAtStop) {
    statesAtStop.clear();
    // find initial set
    std::set<vertex_t> covered;
    for (size_t i = 0; i < robotStates.size(); ++i) {
      vertex_t vertex;
      if (driveIdsToFinish.find(i) != driveIdsToFinish.end()) {
        vertex = robotStates[i].lastVertex;
      } else {
        vertex_t v;
        // start from committedVertex or currentVertex (whichever is latest)
        if (robotStates[i].committedVertex && robotStates[i].currentVertex) {
          if (g[robotStates[i].committedVertex].time >
              g[robotStates[i].currentVertex].time) {
            v = robotStates[i].committedVertex;
          } else {
            v = robotStates[i].currentVertex;
          }
        } else if (!robotStates[i].committedVertex) {
          v = robotStates[i].currentVertex;
        } else {
          v = robotStates[i].lastVertex;
        }

        for (size_t k = 0; k < lookahead && v != nullptr; ++k) {
          assert((g[v].executionState ==
                  Vertex<Action, State>::ExecutionState::NotStarted));
          v = nextVertex(g, v);
        }
        if (v) {
          vertex = v;
        } else {
          vertex = robotStates[i].lastVertex;
        }
      }
      robotStates[i].committedVertex = vertex;
      if (vertex) {
        covered.insert(vertex);
      }
    }

    // find all dependencies of initial set
    size_t lastSize = 0;
    do {
      lastSize = covered.size();

      std::set<vertex_t> dependent;
      for (vertex_t v : covered) {
        auto es = boost::in_edges(v, g);
        for (auto eit = es.first; eit != es.second; ++eit) {
          edge_t e = *eit;
          // if (   g[e].type == Edge::Type2) {
          dependent.insert(source(e, g));
          // }
        }
      }
      covered.insert(dependent.begin(), dependent.end());

    } while (lastSize != covered.size());
    // for each robot, find the last vertex in covered w/ respect to time
    for (vertex_t v : covered) {
      int robotId = g[v].robotId;
      if (robotStates[robotId].committedVertex) {
        if (g[robotStates[robotId].committedVertex].time < g[v].time ||
            (g[robotStates[robotId].committedVertex].time == g[v].time &&
             boost::edge(robotStates[robotId].committedVertex, v, g).second)) {
          // if (robotStates[robotId].committedVertex < v) {
          robotStates[robotId].committedVertex = v;
        }
      }
    }

    // update statesAtStop based on committedVertex
    for (auto &rs : robotStates) {
      if (rs.committedVertex) {
        vertex_t v = nextVertex(g, rs.committedVertex);
        // while(v && boost::in_degree(v, g) > 1)
        // v = nextVertex(g, rs.committedVertex);
        if (v) {
          statesAtStop.push_back(g[v].state);
        } else {
          statesAtStop.push_back(rs.finalState);
        }
        // g[rs.committedVertex].flag = true;
      } else {
        // std::cout << "Not committed vertex " << g[rs.currentVertex].robotId
        // << std::endl;
        statesAtStop.push_back(rs.finalState);
      }
    }

    // check for position conflicts in statesAtStop
    // if (!m_isSIPP)
    {
      bool allUnique = true;
      do {
        std::set<std::pair<int, int>> positionsAtEnd;
        std::map<std::pair<int, int>, int> robotAtPosition;
        allUnique = true;
        for (int i = 0; i < statesAtStop.size(); i++) {
          std::pair<int, int> currentPos =
              std::make_pair(statesAtStop[i].x, statesAtStop[i].y);
          if (checkConflict(positionsAtEnd, currentPos)) {
            allUnique = false;
            std::cout << "Found a conflict in ADG commits" << std::endl;
            int clashingRobotId = robotAtPosition[currentPos];
            int minTimeRobot =
                statesAtStop[clashingRobotId].time > statesAtStop[i].time
                    ? i
                    : clashingRobotId;
            std::cout << "The clashing robots are " << clashingRobotId << " "
                      << i << std::endl;
            std::cout << "min time robot " << minTimeRobot << std::endl;
            std::cout << "The times are " << statesAtStop[clashingRobotId].time
                      << " " << statesAtStop[i].time << std::endl;
            std::cout << "The positions are " << currentPos.first << " "
                      << currentPos.second << std::endl;
            std::cout << "The graph looks like: " << std::endl;
            std::cout << "For robot " << i << std::endl;
            {
              vertex_t v = robotStates[i].committedVertex;
              while (v) {
                std::cout << g[v].state << "->";
                v = nextVertex(g, v);
              }
              std::cout << std::endl;
            }
            std::cout << "FINAL STATE " << robotStates[i].finalState
                      << std::endl;
            std::cout << "For robot " << clashingRobotId << std::endl;
            {
              vertex_t v = robotStates[clashingRobotId].committedVertex;
              while (v) {
                std::cout << g[v].state << "->";
                v = nextVertex(g, v);
              }
              std::cout << std::endl;
            }
            std::cout << "FINAL STATE "
                      << robotStates[clashingRobotId].finalState << std::endl;
            vertex_t v1 =
                nextVertex(g, robotStates[minTimeRobot].committedVertex);
            if (!v1) {
              // this is very unexpected, raise an exception
              // print the graph also
              // printGraph("AtTheCommit.dot");
              throw NextVertexNotFoundException();
            }
            vertex_t v = nextVertex(g, v1);
            if (!v)
              statesAtStop[minTimeRobot] = robotStates[minTimeRobot].finalState;
            else
              statesAtStop[minTimeRobot] = g[v].state;

            robotStates[minTimeRobot].committedVertex = v1;
            break;
          } else {
            positionsAtEnd.insert(currentPos);
            robotAtPosition[currentPos] = i;
          }
        }
      } while (!allUnique);
    }

    // // avoid same location committed vertex
    // for (int i = 0; i < statesAtStop.size(); i++) {
    //   bool diffPos = false;
    //   vertex_t v = robotStates[i].committedVertex;
    //   if (!v) continue;
    //   do {
    //     std::pair<int, int> currentPos =
    //         std::make_pair(statesAtStop[i].x, statesAtStop[i].y);

    //     std::pair<int, int> prevPos =
    //         std::make_pair(g[v].state.x, g[v].state.y);

    //     if (currentPos.first == prevPos.first &&
    //         currentPos.second == prevPos.second) {
    //       v = nextVertex(g, v);
    //       if (!v) {
    //         std::cout << "Robot " << i << " ERROR: COULD NOT FIND NEXT VERTEX
    //         TO COMMIT TO"
    //                   << std::endl;
    //         break;
    //       }
    //       vertex_t v1 = nextVertex(g, v);
    //       if (!v1) {
    //         std::cout << "Robot " << i << " ERROR: COULD NOT FIND NEXT VERTEX
    //         FOR STATES AT STOP"
    //                   << std::endl;
    //         break;
    //       }
    //       robotStates[i].committedVertex = v;
    //       statesAtStop[i] = g[v1].state;
    //     } else
    //       diffPos = true;
    //   } while (!diffPos);
    // }

    static int iter = 0;
    std::stringstream sstr;
    sstr << "adg_findCommittedVertex_iter" << iter << ".dot";
    writeDotFile(sstr.str());
    ++iter;
    std::cout << "Found committed vertex" << std::endl;
  }

  // removes all vertices after the "committed" portion of the graph, excluding
  // committedVertex
  void removeAfterCommittedVertex() {
    int k = -1;
    for (auto &rs : robotStates) {
      k += 1;
      vertex_t v = rs.committedVertex;
      if (v) {
        v = nextVertex(g, v);
      }
      if (v) {
        rs.finalState = g[v].state;
        rs.lastVertex = rs.committedVertex;

        // bool found = false;
        // auto es = boost::in_edges(v, g);
        // for (auto eit = es.first; eit != es.second; ++eit) {
        //   edge_t e = *eit;
        //   if (g[e].type == Edge::Type::Type1) {
        //     rs.lastVertex = source(e, g);
        //     found = true;
        //     break;
        //   }
        // }
        // if (!found) {
        //   rs.lastVertex = nullptr;
        //   rs.firstVertex = nullptr;
        // }

        while (v) {
          // if (v == rs.committedVertex) {
          //   rs.committedVertex = nullptr;
          // }
          // if (v == rs.currentVertex) {
          //   rs.currentVertex = nullptr;
          // }
          // if (v == rs.finishedVertex) {
          //   rs.finishedVertex = nullptr;
          // }
          assert(v != rs.committedVertex);
          assert(v != rs.currentVertex);
          assert(v != rs.finishedVertex);
          assert((g[v].executionState ==
                  Vertex<Action, State>::ExecutionState::NotStarted));

          vertex_t next = nextVertex(g, v);
          boost::clear_vertex(v, g);
          boost::remove_vertex(v, g);
          rs.remainingActions -= 1;
          // std::cout << "Removed action for committed vertex for robot: " << k
          // << " now there are " << rs.remainingActions << std::endl;
          assert(rs.remainingActions >= 0);
          rs.actionId -= 1;
          v = next;
        }
      }
    }

    static int iter = 0;
    std::stringstream sstr;
    sstr << "adg_removeAfterCommittedVertex_iter" << iter << ".dot";
    writeDotFile(sstr.str());
    ++iter;
  }

  int remainingActions(size_t robotId) {
    assert(robotStates[robotId].remainingActions >= 0);
    return robotStates[robotId].remainingActions;
  }

  // TODO: this name is a bit misleading
  bool hasActions(size_t robotId) {
    return robotStates[robotId].firstVertex != nullptr;
  }

  void printGraph(const std::string &fileName) {
    static const float DX = 1;
    static const float DY = 1;

    std::ofstream out(fileName);
    out << "digraph G {" << std::endl;
    std::map<vertex_t, size_t> vertexMap;
    auto vs = boost::vertices(g);
    size_t i = 0;
    for (auto vit = vs.first; vit != vs.second; ++vit) {
      vertex_t v = *vit;
      vertexMap[v] = i;
      out << vertexMap[v];

      out << "[label=\"";
      const auto &rs = robotStates[g[v].robotId];
      if (rs.firstVertex == v) {
        out << " FIRST ";
      }
      if (rs.lastVertex == v) {
        out << " LAST ";
      }
      if (rs.currentVertex == v) {
        out << " CURRENT ";
      }
      if (rs.finishedVertex == v) {
        out << " FINISHED ";
      }
      if (rs.committedVertex == v) {
        out << " COMMITTED ";
      }
      out << g[v].action << "/" << g[v].state << "\" pos=\"" << g[v].time * DX
          << "," << g[v].robotId * DY << "!\"";
      if (g[v].executionState ==
          Vertex<Action, State>::ExecutionState::Completed) {
        out << " color=\"green\"";
      } else if (g[v].executionState ==
                 Vertex<Action, State>::ExecutionState::Enqueued) {
        out << " color=\"yellow\"";
      }
      out << "]";

      out << ";" << std::endl;
      ++i;
    }

    auto es = boost::edges(g);
    for (auto eit = es.first; eit != es.second; ++eit) {
      edge_t e = *eit;
      vertex_t source = boost::source(e, g);
      vertex_t target = boost::target(e, g);
      if (vertexMap.find(source) != vertexMap.end() &&
          vertexMap.find(target) != vertexMap.end()) {
        out << vertexMap[source] << "->" << vertexMap[target] << " ";
        // ew(out, e);
        out << ";" << std::endl;
      }
    }

    out << "}" << std::endl;

    // std::cout << "wrote " << fileName << std::endl;
  }

  void writeDotFile(const std::string &fileName) {
    return;
    static const float DX = 400;
    static const float DY = 100;

    std::ofstream out(fileName);
    // auto &out = std::cout;

    out << "digraph G {" << std::endl;
    std::map<vertex_t, size_t> vertexMap;
    auto vs = boost::vertices(g);
    size_t i = 0;
    for (auto vit = vs.first; vit != vs.second; ++vit) {
      vertex_t v = *vit;
      vertexMap[v] = i;
      out << vertexMap[v];

      out << "[label=\"";
      const auto &rs = robotStates[g[v].robotId];
      if (rs.firstVertex == v) {
        out << " FIRST ";
      }
      if (rs.lastVertex == v) {
        out << " LAST ";
      }
      if (rs.currentVertex == v) {
        out << " CURRENT ";
      }
      if (rs.finishedVertex == v) {
        out << " FINISHED ";
      }
      if (rs.committedVertex == v) {
        out << " COMMITTED ";
      }
      out << g[v].action << "/" << g[v].state << "\" pos=\"" << g[v].time << ","
          << g[v].robotId << "!\"";
      if (g[v].executionState ==
          Vertex<Action, State>::ExecutionState::Completed) {
        out << " color=\"green\"";
      } else if (g[v].executionState ==
                 Vertex<Action, State>::ExecutionState::Enqueued) {
        out << " color=\"yellow\"";
      }
      out << "]";

      out << ";" << std::endl;
      ++i;
    }

    auto es = boost::edges(g);
    for (auto eit = es.first; eit != es.second; ++eit) {
      edge_t e = *eit;
      vertex_t source = boost::source(e, g);
      vertex_t target = boost::target(e, g);
      if (vertexMap.find(source) != vertexMap.end() &&
          vertexMap.find(target) != vertexMap.end()) {
        out << vertexMap[source] << "->" << vertexMap[target] << " ";
        // ew(out, e);
        out << ";" << std::endl;
      }
    }

    out << "}" << std::endl;

    std::cout << "wrote " << fileName << std::endl;
  }

private:
  actionDependencyGraph_t<Action, State> g;
  bool m_isSIPP;
  bool checkConflict(const std::set<std::pair<int, int>> &positionsAtEnd,
                     const std::pair<int, int> &currentPos) {
    return positionsAtEnd.find(currentPos) != positionsAtEnd.end();
  }

  struct RobotState {
    RobotState()
        : firstVertex(nullptr), lastVertex(nullptr), finalState(),
          currentVertex(nullptr), finishedVertex(nullptr),
          committedVertex(nullptr), remainingActions(0), queuedActions(0),
          actionId(0) {}

    vertex_t firstVertex; // vertex with the first action for this robot
    vertex_t lastVertex;  // vertex with the last action  for this robot
    State finalState;     // state after last action was executed

    vertex_t currentVertex;  // points to the next vertex that might be enqueued
    vertex_t finishedVertex; // points to the next vertex that might finish
    vertex_t committedVertex; // points to the last committed vertex (vertices
                              // afterwards might change)

    int remainingActions;
    int queuedActions;
    size_t actionId;
  };

  std::vector<RobotState> robotStates;
}; // namespace libActionDependencyGraph

} // namespace libActionDependencyGraph

#pragma once

#include <boost/graph/adjacency_list.hpp>

namespace libActionDependencyGraph {

typedef boost::adjacency_list_traits<boost::listS, boost::listS, boost::bidirectionalS > commandGraphTraits_t;
typedef commandGraphTraits_t::vertex_descriptor vertex_t;
typedef commandGraphTraits_t::edge_descriptor edge_t;


template <
  typename Action,
  typename State
  >
struct Vertex
{
  enum class ExecutionState
  {
    NotStarted,
    Enqueued,
    Completed,
  };

  Vertex()
    : executionState(ExecutionState::NotStarted)
    // , flag(false)
    // , next(nullptr)
  {
  }

  Action action;
  State state;
  ExecutionState executionState;
  // vertex_t next;// faster traversal

  uint32_t robotId;
  size_t actionId;
  double time;
  // int driveId;
  // int t;
  // int xpos; // for debug output
  // std::string info; // for debug output



  // bool flag;//debug

  // void* userData;
};

struct Edge
{
  enum class Type
  {
    Type1, // subsequent between the same drive
    Type2, // dependencies between different drives
  };

  Edge()
    : type(Type::Type1)
  {
  }

  Type type;
};

template <
  typename Action,
  typename State
  >
using actionDependencyGraph_t = boost::adjacency_list<
        boost::listS, boost::listS, boost::bidirectionalS,
        Vertex<Action, State>, Edge>;

} // namespace libActionDependencyGraph

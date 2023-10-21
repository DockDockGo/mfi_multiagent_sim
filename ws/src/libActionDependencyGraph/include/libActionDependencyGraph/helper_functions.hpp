#pragma once

#include "libActionDependencyGraph/types.hpp"

#include <boost/graph/graphviz.hpp>
#include <boost/graph/depth_first_search.hpp>

namespace libActionDependencyGraph {

template <typename Action, typename State>
class VertexDotWriter {
 public:
  explicit VertexDotWriter(const actionDependencyGraph_t<Action, State>& graph)
      : m_graph(graph) {}

  void operator()(std::ostream& out, vertex_t v) const {
    static const float DX = 1;
    static const float DY = 1;
    out << "[label=\"" << m_graph[v].action << "\" pos=\""
        << m_graph[v].time * DX << "," << m_graph[v].robotId * DY << "!\"";
    if (m_graph[v].executionState ==
        Vertex<Action, State>::ExecutionState::Completed) {
      out << " color=\"green\"";
    } else if (m_graph[v].executionState ==
               Vertex<Action, State>::ExecutionState::Enqueued) {
      out << " color=\"yellow\"";
    }

    // if (m_graph[v].flag) {
    //   out << " color=\"red\"";
    // }

    out << "]";
  }

 private:
  const actionDependencyGraph_t<Action, State>& m_graph;
};

template <typename Action, typename State>
class EdgeDotWriter {
 public:
  explicit EdgeDotWriter(const actionDependencyGraph_t<Action, State>& graph)
      : m_graph(graph) {}

  void operator()(std::ostream& out, edge_t e) const {
    // out << "[label=\"" << m_graph[e].weight << "\"]";
  }

 private:
  const actionDependencyGraph_t<Action, State>& m_graph;
};

template <typename Action, typename State, typename PredecessorMap>
class CycleDetector : public boost::dfs_visitor<> {
 public:
  CycleDetector(bool& has_cycle) : _has_cycle(has_cycle) {}

  template <class Edge, class Graph>
  void tree_edge(Edge e, Graph& g) {
    m_pMap[target(e, g)] = source(e, g);
  }

  template <class Edge, class Graph>
  void back_edge(Edge e, Graph& g) {
    _has_cycle = true;
    std::cout << "CYCLE DETECTED: ";
    VertexDotWriter<Action, State> vw(g);
    vw(std::cout, source(e, g));
    std::cout << " -> ";
    vw(std::cout, target(e, g));
    std::cout << std::endl;
    auto v = source(e, g);
    std::cout << "CYCLE IS: " << std::endl;
    vw(std::cout, v);
    std::cout << "<-";
    while (v != target(e, g)) {
      v = m_pMap[v];
      vw(std::cout, v);
      std::cout << "<-";
      // std::cout << std::endl;
    }
    std::cout << std::endl;
  }

 protected:
  bool& _has_cycle;
  PredecessorMap m_pMap;
};

template <typename Action, typename State>
void writeDotFile(const actionDependencyGraph_t<Action, State>& g,
                  const std::string& fileName) {
  VertexDotWriter<Action, State> vw(g);
  EdgeDotWriter<Action, State> ew(g);
#if 1
  typedef std::map<vertex_t, size_t> vertexID_t;
  vertexID_t mapVertexID;
  boost::associative_property_map<vertexID_t> propMapVertexID(mapVertexID);
  // indexing the vertices
  int i = 0;
  auto vs = boost::vertices(g);
  for (auto vit = vs.first; vit != vs.second; ++vit) {
    boost::put(propMapVertexID, *vit, i);
    ++i;
  }
  std::ofstream dotFile(fileName);
  boost::write_graphviz(dotFile, g, vw, ew, boost::default_writer(),
                        propMapVertexID);
#else
  std::ofstream out(fileName);
  out << "digraph G {" << std::endl;
  std::map<vertex_t, size_t> vertexMap;
  auto vs = boost::vertices(g);
  size_t i = 0;
  for (auto vit = vs.first; vit != vs.second; ++vit) {
    vertex_t v = *vit;
    if (g[v].driveId == 9 || g[v].driveId == 49) {
      vertexMap[v] = i;
      out << vertexMap[v];
      vw(out, v);
      out << ";" << std::endl;
      ++i;
    }
  }

  auto es = boost::edges(g);
  for (auto eit = es.first; eit != es.second; ++eit) {
    edge_t e = *eit;
    vertex_t source = boost::source(e, g);
    vertex_t target = boost::target(e, g);
    if (vertexMap.find(source) != vertexMap.end() &&
        vertexMap.find(target) != vertexMap.end()) {
      out << vertexMap[source] << "->" << vertexMap[target] << " ";
      ew(out, e);
      out << ";" << std::endl;
    }
  }

  out << "}" << std::endl;

#endif
  std::cout << "wrote " << fileName << std::endl;
}

// check for cycles in the graph
template <typename Action, typename State>
bool isValid(const actionDependencyGraph_t<Action, State>& g) {
  bool has_cycle = false;
  CycleDetector<Action, State, std::unordered_map<vertex_t, vertex_t> > vis(
      has_cycle);

  typedef std::map<vertex_t, boost::default_color_type> colorMap_t;
  colorMap_t colorMap;
  boost::associative_property_map<colorMap_t> propMapColor(colorMap);
  boost::depth_first_search(g, vis, propMapColor);
  return !has_cycle;
}

template <typename Action, typename State>
vertex_t nextVertex(const actionDependencyGraph_t<Action, State>& g,
                    vertex_t v) {
  auto es = boost::out_edges(v, g);
  for (auto eit = es.first; eit != es.second; ++eit) {
    edge_t e = *eit;
    if (g[e].type == Edge::Type::Type1) {
      return target(e, g);
    }
  }
  return nullptr;
}

template <typename Action, typename State>
vertex_t prevVertex(const actionDependencyGraph_t<Action, State>& g,
                    vertex_t v) {
  auto es = boost::in_edges(v, g);
  for (auto eit = es.first; eit != es.second; ++eit) {
    edge_t e = *eit;
    if (g[e].type == Edge::Type::Type1) {
      return source(e, g);
    }
  }
  return nullptr;
}

template <typename Action, typename State>
vertex_t previousVertex(const actionDependencyGraph_t<Action, State>& g,
                        vertex_t v) {
  auto es = boost::in_edges(v, g);
  for (auto eit = es.first; eit != es.second; ++eit) {
    edge_t e = *eit;
    if (g[e].type == Edge::Type::Type1) {
      return source(e, g);
    }
  }
  return nullptr;
}

}  // namespace libActionDependencyGraph

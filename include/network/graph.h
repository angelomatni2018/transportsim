#ifndef GRAPH
#define GRAPH

#include <unordered_map>
#include <unordered_set>

#include "base/spatial.h"
#include "network/network.h"

namespace world {

class NetworkEdge {
protected:
  WorldElement* x;
  WorldElement* y;
  int cost;

public:
  NetworkEdge(WorldElement* x, WorldElement* y, int cost) : x{x}, y{y}, cost{cost} {}

  int Cost() const { return cost; }
  std::pair<WorldElement*, WorldElement*> Nodes() const { return {x, y}; }
  WorldElement* Other(WorldElement* one) const { return x == one ? y : (y == one ? x : nullptr); }
};

class NetworkGraph {
protected:
  std::unordered_map<WorldElement*, std::unordered_map<WorldElement*, NetworkEdge*>> connections;
  Pool<NetworkEdge> pool;

  NetworkEdge* addUndirected(NetworkEdge* edge) {
    auto [one, other] = edge->Nodes();
    connections[one][other] = edge;
    connections[other][one] = edge;
    return edge;
  }

public:
  NetworkEdge* AddUndirected(WorldElement* one, WorldElement* other, int cost) {
    auto edge = pool.With(NetworkEdge(one, other, cost));
    return addUndirected(edge);
  }

  // Since edges are undirected, all connected nodes will have an entry in connections
  int NumNodes() const { return connections.size(); }

  std::vector<WorldElement*> Nodes() const {
    std::vector<WorldElement*> nodes;
    for (auto& [node, _] : connections) {
      nodes.push_back(node);
    }
    return nodes;
  }

  WorldElement* AnyNode() const { return connections.size() == 0 ? nullptr : connections.begin()->first; }

  std::vector<NetworkEdge*> Edges(WorldElement* node) const {
    if (connections.find(node) == connections.end()) {
      return {};
    }

    std::vector<NetworkEdge*> edges;
    for (auto& [other, edge] : connections.at(node)) {
      edges.push_back(edge);
    }
    return edges;
  }

  const NetworkEdge* Edge(WorldElement* one, WorldElement* other) const {
    if (connections.find(one) == connections.end())
      return nullptr;
    auto edges = connections.at(one);
    if (edges.find(other) == edges.end())
      return nullptr;
    return edges.at(other);
  }
};

} // namespace world

#endif
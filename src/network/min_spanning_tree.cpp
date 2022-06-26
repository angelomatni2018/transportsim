#include "network/min_spanning_tree.h"
#include "spdlog/spdlog.h"

using namespace world;

MinimumSpanningTree::MinimumSpanningTree(const NetworkGraph& graph)
    : queue{[](const NetworkEdge* a, const NetworkEdge* b) { return a->Cost() > b->Cost(); }} {
  prim(graph);
}

MinimumSpanningTree::MinimumSpanningTree(const NetworkGraph& graph, std::function<bool(const NetworkEdge* a, const NetworkEdge* b)> comparator)
    : queue{comparator} {
  prim(graph);
}

const std::unordered_set<const NetworkEdge*>& MinimumSpanningTree::MstEdges() const {
  return mstEdges;
}

void MinimumSpanningTree::prim(const NetworkGraph& graph) {
  auto numNodes = graph.NumNodes();
  if (numNodes == 0)
    return;
  auto firstNode = graph.AnyNode();
  mstNodes.emplace(firstNode);
  for (auto edge : graph.Edges(firstNode)) {
    queue.push(edge);
  }
  while (mstNodes.size() < numNodes) {
    if (queue.empty()) {
      spdlog::trace("MinimumSpanningTree: queue empty but MST not found; is graph connected?\n");
      throw "MinimumSpanningTree: queue empty prematurely";
    }

    auto minEdge = queue.top();
    queue.pop();

    auto [one, other] = minEdge->Nodes();
    WorldElement* next = nullptr;
    if (mstNodes.find(one) == mstNodes.end()) {
      next = one;
    } else if (mstNodes.find(other) == mstNodes.end()) {
      next = other;
    } else
      continue;

    mstEdges.emplace(minEdge);
    mstNodes.emplace(next);
    for (auto edge : graph.Edges(next)) {
      if (mstNodes.find(edge->Other(next)) == mstNodes.end()) {
        queue.push(edge);
      }
    }
  }

  if (mstNodes.size() != numNodes) {
    spdlog::trace("MinimumSpanningTree: {} mst nodes versus {} graph nodes; is graph connected?\n", mstNodes.size(), numNodes);
    throw "MinimumSpanningTree: graph nodes missing from MST";
  }
}
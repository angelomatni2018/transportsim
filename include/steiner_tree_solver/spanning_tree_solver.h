#ifndef SPANNING_TREE_SOLVER
#define SPANNING_TREE_SOLVER

#include <unordered_map>
#include <unordered_set>

#include "base/geometric_median.h"
#include "base/kmeans.h"
#include "network/min_spanning_tree.h"
#include "steiner_tree_solver/road_candidate_graph.h"
#include "steiner_tree_solver/virtual_graph.h"

namespace world {

class SpanningTreeSolver {
public:
  static std::unique_ptr<Network> ConnectStructures(const Network& network);

protected:
  static void addRoadsTo(const Network& inputNetwork, Network& outputNetwork);
};

class ShareRoadEmptyLocationPathfinder : public Pathfinder {
public:
  ShareRoadEmptyLocationPathfinder(const RoadCandidateNetworkGraph& candidateGraph) : Pathfinder() {
    // TODO: This is expensive; optimize it
    for (auto node : candidateGraph.Nodes()) {
      for (auto edge : candidateGraph.Edges(node)) {
        auto candidateEdge = static_cast<const CandidateNetworkEdge*>(edge);
        for (auto road : candidateEdge->Roads) {
          sharedRoads.emplace(road);
        }
      }
    }
  }

  std::unordered_set<Location, pair_hash> sharedRoads;

protected:
  double actualCost(const Network& network, Location neighbor) override {
    if (sharedRoads.find(neighbor) != sharedRoads.end())
      return 0;
    return 1;
  }

  bool isValidNeighborToTraverse(const Network& network, Location current, Location neighbor, Location start, Location end) override {
    // Only allow traveling to an empty location OR the end target
    return neighbor == end || !network.HasStructureAt(neighbor);
  }
};

} // namespace world

#endif
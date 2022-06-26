#include "steiner_tree_solver/road_candidate_graph.h"

using namespace world;

RoadCandidateNetworkGraph::RoadCandidateNetworkGraph(const Network& network) : NetworkGraph{} {
  EmptyLocationPathfinder pathfinder;
  int paths = 0;
  for (auto& el : network.Elements()) {
    for (auto& otherEl : network.Elements()) {
      if (el == otherEl)
        continue;
      if (Edge(el, otherEl) != nullptr)
        continue;
      auto candidatePath = pathfinder.Solve(network, el->PrimaryLocation(), otherEl->PrimaryLocation());
      // TODO: Require pathfinder to have at least one road in between buildings; then this condition would be .size() > 0
      if (candidatePath.size() > 2) {
        AddUndirected(el, otherEl, candidatePath);
        paths++;
        // spdlog::trace("Path found between {} and {} of length {}", to_string(el->PrimaryLocation()), to_string(otherEl->PrimaryLocation()),
        //               candidatePath.size());
      } else {
        // spdlog::trace("No path found between {} and {}", to_string(el->PrimaryLocation()), to_string(otherEl->PrimaryLocation()));
      }
    }
  }
  // spdlog::trace("{} paths found for {} buildings", paths, network.Elements().size());
}
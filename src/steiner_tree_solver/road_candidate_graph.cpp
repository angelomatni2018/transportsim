#include "steiner_tree_solver/road_candidate_graph.h"

using namespace world;

RoadCandidateNetworkGraph::RoadCandidateNetworkGraph(const Network& network) : NetworkGraph{} {
  EmptyLocationPathfinder pathfinder;
  int paths = 0;
  for (auto& el : network.Buildings()) {
    for (auto& otherEl : network.Buildings()) {
      if (el == otherEl)
        continue;
      if (Edge(el, otherEl) != nullptr)
        continue;
      auto candidatePath = pathfinder.Solve(network, el->PrimaryLocation(), otherEl->PrimaryLocation());
      // We require buildings to have at least one road in between them
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
  spdlog::trace("{} paths found for {} buildings", paths, network.Buildings().size());
}
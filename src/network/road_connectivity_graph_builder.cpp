#include "network/road_connectivity_graph_builder.h"

using namespace world;

void RoadConnectivityGraphBuilder::Build(NetworkGraph& emptyGraph, const Network& network) {
  for (auto& [_, el] : network.SpatialMap()) {
    for (auto& [_, otherEl] : network.SpatialMap()) {
      // For now, road connectivity is always assumed to be bi-directional
      // TODO: Approximate cost based on euclidian distance
      //  (to be accompanied by change to reflect euclidian distance between tiles in vehicle movement)
      if (network.IsAdjacentAndConnected(el, otherEl)) {
        emptyGraph.AddUndirected(el, otherEl, 1);
      }
    }
  }
}
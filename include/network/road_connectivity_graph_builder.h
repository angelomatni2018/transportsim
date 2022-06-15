#ifndef ROAD_GRAPH
#define ROAD_GRAPH

#include <unordered_map>
#include <unordered_set>

#include "network/graph.h"
#include "network/network.h"

namespace world {

class RoadConnectivityGraphBuilder {
public:
  static void Build(NetworkGraph& emptyGraph, const Network& network);
};

} // namespace world

#endif
#ifndef ROAD_CANDIDATE_GRAPH
#define ROAD_CANDIDATE_GRAPH

#include <unordered_map>
#include <unordered_set>

#include "network/graph.h"
#include "network/network.h"
#include "pathing/pathfinder.h"

namespace world {

class CandidateNetworkEdge : public NetworkEdge {
public:
  std::vector<Location> Roads;
  CandidateNetworkEdge(WorldElement* x, WorldElement* y, std::vector<Location> roadPath) : NetworkEdge{x, y, int(roadPath.size())}, Roads{roadPath} {}
};

class RoadCandidateNetworkGraph : public NetworkGraph {
public:
  RoadCandidateNetworkGraph(const Network& network);

  NetworkEdge* AddUndirected(WorldElement* one, WorldElement* other, std::vector<Location> roads) {
    return addUndirected(pool.With(CandidateNetworkEdge(one, other, roads)));
  }
};

class EmptyLocationPathfinder : public Pathfinder {
protected:
  bool isValidNeighborToTraverse(const Network& network, Location current, Location neighbor, Location start, Location end) override {
    // Only allow traveling to an empty location OR the end target
    return neighbor == end || !network.HasStructureAt(neighbor);
  }
};

} // namespace world

#endif
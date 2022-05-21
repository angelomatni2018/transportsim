#ifndef ROAD_PATHFINDER
#define ROAD_PATHFINDER

#include "base/spatial.h"
#include "network/network.h"
#include "pathing/pathfinder.h"

namespace world {

class RoadPathfinder : public Pathfinder {
public:
  RoadPathfinder(const Network& network) : Pathfinder{network} {}

  bool isValidNeighborToTraverse(const Network& network, Location current, Location neighbor) override;
};

} // namespace world

#endif
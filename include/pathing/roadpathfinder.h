#ifndef ROAD_PATHFINDER
#define ROAD_PATHFINDER

#include "base/spatial.h"
#include "network/network.h"
#include "pathing/pathfinder.h"

namespace world {

class RoadPathfinder : public Pathfinder {
public:
  bool isValidNeighborToTraverse(const Network& network, Location current, Location neighbor, Location start, Location end) override;
};

} // namespace world

#endif
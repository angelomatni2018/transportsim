#include "pathing/roadpathfinder.h"
#include "spdlog/spdlog.h"

using namespace world;

bool RoadPathfinder::isValidNeighborToTraverse(const Network& network, Location current, Location neighbor, Location start, Location end) {
  // If it is a roadway, check to add to the frontier
  if (!network.HasStructureAt(neighbor))
    return false;
  auto neighborEl = network.StructureAt(neighbor);

  // If the current location is not a road segment, treat it as allowing travel towards any direction
  if (!network.HasStructureAt(current))
    return false;
  auto currentEl = network.StructureAt(current);
  return network.IsAdjacentAndConnected(currentEl, neighborEl);
}

#include "pathing/roadpathfinder.h"
#include "spdlog/spdlog.h"

using namespace world;

bool RoadPathfinder::isValidNeighborToTraverse(const Network& network, Location current, Location neighbor) {
  // If it is a roadway, check to add to the frontier
  if (!network.HasStructureAt(neighbor))
    return false;
  auto neighborEl = network.StructureAt(neighbor);

  // If the neighbor location is not a road segment, treat it as allowing travel towards any direction
  auto neighborMask = ALL_DIRECTIONS;
  if (neighborEl->IsType(RoadSegment::Type))
    neighborMask = static_cast<const RoadSegment*>(neighborEl)->DirectionsMask();

  // If the current location is not a road segment, treat it as allowing travel towards any direction
  auto currentMask = ALL_DIRECTIONS;
  if (network.HasStructureAt(current)) {
    auto currentEl = network.StructureAt(current);
    if (currentEl->IsType(RoadSegment::Type))
      currentMask = static_cast<const RoadSegment*>(currentEl)->DirectionsMask();
  }

  return Roadway::CanDirectionsConnect(currentMask, neighborMask, neighbor - current);
}

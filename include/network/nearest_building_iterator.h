#ifndef NEAREST_BUILDING_ITERATOR
#define NEAREST_BUILDING_ITERATOR

#include "network/network.h"
#include "network/world_elements.h"

#include <queue>
#include <unordered_set>

namespace world {
class NearestBuildingIterator {
private:
  const Network& network;
  const Building* target;

  std::queue<const Building*> nearests;
  std::queue<const Roadway*> frontier;
  std::unordered_set<const Roadway*> visited;

  void checkToQueueBuilding(Location loc);
  void checkToQueueRoadway(Location loc);

public:
  NearestBuildingIterator(const Network& network, const Building* target);
  const Building* Next();
};
} // namespace world

#endif

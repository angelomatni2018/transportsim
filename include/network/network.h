#ifndef NETWORK
#define NETWORK

#include <iostream>
#include <math.h>
#include <unordered_map>
#include <vector>

#include "base/spatial.h"
#include "network/world_elements.h"

namespace world {

class Network {
private:
  PtrVec<WorldElement> elementPool;
  std::vector<Building*> buildings;
  std::vector<Roadway*> roads;

  // Every location in this map is aligned to STRUCTURE_BASE_SIZE_UNIT (see world_elements.h)
  std::unordered_map<Location, WorldElement*, pair_hash> spatialMap;

  std::pair<Location, Location> bounds;
  void expandBounds(Location outermost);
  void addToSpatialMap(WorldElement* element);
  Building* addBuilding(Building* building);
  Roadway* addRoadway(Roadway* roadway);

public:
  Network();

  static Location AlignLocation(Location unalignedLoc);

  const int Size() const;
  const std::vector<WorldElement*>& Elements() const;
  const std::vector<Building*>& Buildings() const;
  const std::vector<Roadway*>& Roads() const;

  bool HasStructureAt(Location loc) const;
  const WorldElement* StructureAt(Location loc) const;

  template <class DerivedElement>
  DerivedElement* Add(DerivedElement&& el) {
    WorldElement* elPtr = elementPool.Add<DerivedElement>(std::move(el));
    if (el.IsType(Building::Type)) {
      addBuilding(static_cast<Building*>(elPtr));
    } else if (el.IsType(Roadway::Type)) {
      addRoadway(static_cast<Roadway*>(elPtr));
    } else {
      spdlog::trace("Network does not support WorldElement of type {}", el.GetType());
      throw "Network unsupported WorldElement type";
    }
    return static_cast<DerivedElement*>(elPtr);
  }

  WorldElement* AddCopyOf(const WorldElement* el);
  void Clear();

  std::pair<Location, Location> Bounds() const { return bounds; }

  bool IsAdjacentAndConnected(const WorldElement* from, const WorldElement* to) const;
};

} // namespace world

#endif
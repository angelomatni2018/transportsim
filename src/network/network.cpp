#include "network/network.h"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <queue>

using namespace world;

Network::Network() : buildings{}, spatialMap{} {};

Location Network::AlignLocation(Location unalignedLoc) {
  auto& [x, y] = unalignedLoc;
  return Location{x - x % STRUCTURE_BASE_SIZE_UNIT, y - y % STRUCTURE_BASE_SIZE_UNIT};
}

bool Network::HasStructureAt(Location loc) const {
  return spatialMap.find(Network::AlignLocation(loc)) != spatialMap.end();
}

const WorldElement* Network::StructureAt(Location loc) const {
  return spatialMap.at(Network::AlignLocation(loc));
}

const std::vector<WorldElement*>& Network::Elements() const {
  return this->elementPool.Get();
}

const int Network::Size() const {
  return this->elementPool.Get().size();
}

void Network::addToSpatialMap(WorldElement* element) {
  auto alignedLoc = element->PrimaryLocation();
  if (alignedLoc.first % STRUCTURE_BASE_SIZE_UNIT != 0 || alignedLoc.second % STRUCTURE_BASE_SIZE_UNIT != 0) {
    spdlog::trace("Structure (of type {}) with primary location {} is not a multiple of STRUCTURE_BASE_SIZE_UNIT={}", element->GetType(),
                  to_string(alignedLoc), STRUCTURE_BASE_SIZE_UNIT);
    throw "Network structure misaligned";
  }

  for (auto loc : element->AllOccupiedLocations()) {
    if (this->spatialMap.find(loc) != this->spatialMap.end()) {
      spdlog::trace("Cannot place two entities on the same location {} on the network\n", to_string(loc));
      throw "Network structure location overlap";
    }
    this->spatialMap[loc] = element;
    // spdlog::trace("Network: spatial map entry at {} of type {}", to_string(element->PrimaryLocation()), element->GetType());
    this->expandBounds(loc);
  }
}

std::vector<Building*> const& Network::Buildings() const {
  return this->buildings;
}

std::vector<Roadway*> const& Network::Roads() const {
  return this->roads;
}

Building* Network::addBuilding(Building* building) {
  this->buildings.push_back(building);
  this->addToSpatialMap(building);
  return building;
}

Roadway* Network::addRoadway(Roadway* roadway) {
  this->roads.push_back(roadway);
  this->addToSpatialMap(roadway);
  return roadway;
}

WorldElement* Network::AddCopyOf(const WorldElement* elToCopy) {
  auto el = elToCopy->Copy(elementPool);
  if (el->IsType(Building::Type)) {
    return addBuilding(static_cast<Building*>(el));
  }
  if (el->IsType(Roadway::Type)) {
    return addRoadway(static_cast<Roadway*>(el));
  }
  spdlog::trace("Network does not support WorldElement of type {}", el->GetType());
  throw "Network unsupported WorldElement type";
}

void Network::Clear() {
  elementPool = PtrVec<WorldElement>();
  spatialMap.clear();
  buildings.clear();
  roads.clear();
  bounds = {{0, 0}, {0, 0}};
}

void Network::expandBounds(Location outermost) {
  auto lowermost = bounds.first;
  if (outermost.first < lowermost.first) {
    lowermost.first = outermost.first;
  }
  if (outermost.second < lowermost.second) {
    lowermost.second = outermost.second;
  }
  bounds.first = lowermost;

  auto uppermost = bounds.second;
  if (outermost.first > uppermost.first) {
    uppermost.first = outermost.first;
  }
  if (outermost.second > uppermost.second) {
    uppermost.second = outermost.second;
  }
  bounds.second = uppermost;
}

bool Network::IsAdjacentAndConnected(const WorldElement* from, const WorldElement* to) const {
  auto heading = to->PrimaryLocation() - from->PrimaryLocation();
  // spdlog::trace("heading from {} to {}: {}", to_string(from->PrimaryLocation()), to_string(to->PrimaryLocation()), to_string(heading));
  if (heading.first == 0 && heading.second == 0)
    return false;
  if (abs(heading.first) > STRUCTURE_BASE_SIZE_UNIT || abs(heading.second) > STRUCTURE_BASE_SIZE_UNIT)
    return false;

  auto fromMask = ALL_DIRECTIONS;
  if (from->IsType(RoadSegment::Type))
    fromMask = static_cast<const RoadSegment*>(from)->DirectionsMask();
  auto toMask = ALL_DIRECTIONS;
  if (to->IsType(RoadSegment::Type))
    toMask = static_cast<const RoadSegment*>(to)->DirectionsMask();
  // spdlog::trace("masks from/to {} {} : {}", fromMask, toMask, Roadway::CanDirectionsConnect(fromMask, toMask, heading));
  return Roadway::CanDirectionsConnect(fromMask, toMask, heading);
}
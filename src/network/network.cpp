#include "network/network.h"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <queue>

using namespace world;

Network::Network() : buildings{}, spatialMap{} {};

bool Network::HasStructureAt(Location loc) const {
  auto& [x, y] = loc;
  auto alignedLoc = Location{x - x % STRUCTURE_BASE_SIZE_UNIT, y - y % STRUCTURE_BASE_SIZE_UNIT};
  return spatialMap.find(alignedLoc) != spatialMap.end();
}

const WorldElement* Network::StructureAt(Location loc) const {
  auto& [x, y] = loc;
  auto alignedLoc = Location{x - x % STRUCTURE_BASE_SIZE_UNIT, y - y % STRUCTURE_BASE_SIZE_UNIT};
  return spatialMap.at(alignedLoc);
}

const std::unordered_map<Location, WorldElement*, pair_hash>& Network::SpatialMap() const {
  return this->spatialMap;
}

const int Network::Size() const {
  return this->spatialMap.size();
}

void Network::addToSpatialMap(WorldElement* element) {
  auto alignedLoc = element->PrimaryLocation();
  if (alignedLoc.first % STRUCTURE_BASE_SIZE_UNIT != 0 || alignedLoc.second % STRUCTURE_BASE_SIZE_UNIT != 0) {
    spdlog::error("Structure (of type {}) with primary location {} is not a multiple of STRUCTURE_BASE_SIZE_UNIT={}", element->GetType(),
                  to_string(alignedLoc), STRUCTURE_BASE_SIZE_UNIT);
    abort();
  }
  this->spatialMap[alignedLoc] = element;
  // spdlog::trace("Network: spatial map entry at {} of type {}", to_string(element->PrimaryLocation()), element->GetType());
  this->expandBounds(alignedLoc);
}

std::vector<Building*> const& Network::Buildings() const {
  return this->buildings;
}

std::vector<Roadway*> const& Network::Roads() const {
  return this->roads;
}

Building* Network::addBuilding(Building* building) {
  this->buildings.push_back(building);
  if (this->spatialMap.find(building->PrimaryLocation()) != this->spatialMap.end()) {
    spdlog::error("Cannot place two entities on the same location on the network\n");
    abort();
  }
  this->addToSpatialMap(building);
  return building;
}

Roadway* Network::addRoadway(Roadway* roadway) {
  this->roads.push_back(roadway);
  if (this->spatialMap.find(roadway->PrimaryLocation()) != this->spatialMap.end()) {
    spdlog::error("Cannot place two entities on the same location on the network\n");
    abort();
  }
  this->addToSpatialMap(roadway);
  return roadway;
}

// WorldElement* Network::Add(WorldElement&& el) {
//   if (el.GetType() == Building::Type) {
//     return addBuilding(elementPool.With<Building>(static_cast<Building&&>(el)));
//   }
//   if (el.GetType() == CommercialBuilding::Type) {
//     return addBuilding(elementPool.With<CommercialBuilding>(static_cast<CommercialBuilding&&>(el)));
//   }
//   if (el.GetType() == ResidentialBuilding::Type) {
//     return addBuilding(elementPool.With<ResidentialBuilding>(static_cast<ResidentialBuilding&&>(el)));
//   }
//   if (el.GetType() == RoadSegment::Type) {
//     return addRoadway(elementPool.With<RoadSegment>(static_cast<RoadSegment&&>(el)));
//   }
//   spdlog::error("Network does not support WorldElement of type {}", el.GetType());
//   abort();
// }

WorldElement* Network::AddCopyOf(const WorldElement* elToCopy) {
  auto el = elToCopy->Copy(elementPool);
  if (el->IsType(Building::Type)) {
    return addBuilding(static_cast<Building*>(el));
  }
  if (el->IsType(Roadway::Type)) {
    return addRoadway(static_cast<Roadway*>(el));
  }
  spdlog::error("Network does not support WorldElement of type {}", el->GetType());
  abort();
}

void Network::Clear() {
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
  auto heading = from->PrimaryLocation() - to->PrimaryLocation();
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
  return Roadway::CanDirectionsConnect(fromMask, toMask, heading);
}
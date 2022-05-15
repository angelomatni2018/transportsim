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

void Network::addToSpatialMap(WorldElement* element) {
  auto alignedLoc = element->PrimaryLocation();
  if (alignedLoc.first % STRUCTURE_BASE_SIZE_UNIT != 0 || alignedLoc.second % STRUCTURE_BASE_SIZE_UNIT != 0) {
    spdlog::error("Structure (of type {}) with primary location {} is not a multiple of STRUCTURE_BASE_SIZE_UNIT={}", element->GetType(),
                  to_string(alignedLoc), STRUCTURE_BASE_SIZE_UNIT);
    abort();
  }
  this->spatialMap[alignedLoc] = element;
  spdlog::trace("Network: spatial map entry at {} of type {}", to_string(element->PrimaryLocation()), element->GetType());
  this->expandBounds(alignedLoc);
}

std::vector<Building*> const& Network::Buildings() const {
  return this->buildings;
}

std::vector<Roadway*> const& Network::Roads() const {
  return this->roads;
}

void Network::AddBuilding(Building* building) {
  this->buildings.push_back(building);
  if (this->spatialMap.find(building->PrimaryLocation()) != this->spatialMap.end()) {
    spdlog::error("Cannot place two entities on the same location on the network\n");
    abort();
  }
  this->addToSpatialMap(building);
}

void Network::AddRoadway(Roadway* roadway) {
  this->roads.push_back(roadway);
  if (this->spatialMap.find(roadway->PrimaryLocation()) != this->spatialMap.end()) {
    spdlog::error("Cannot place two entities on the same location on the network\n");
    abort();
  }
  this->addToSpatialMap(roadway);
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
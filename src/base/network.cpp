#include "base/network.h"
#include <queue>
#include <algorithm>

using namespace world;

Network::Network() : buildings{}, spatialMap{} {};

bool Network::hasElementAt(Location loc) const {
    return spatialMap.find(loc) != spatialMap.end();
}

const WorldElement *Network::getElementAt(Location loc) const {
    return spatialMap.at(loc);
}

const std::unordered_map<Location, WorldElement *, pair_hash> &Network::getSpatialMap() const {
    return this->spatialMap;
}

void Network::addToSpatialMap(WorldElement *element) {
    this->spatialMap[element->location] = element;
    // std::cout << "Adding to spatial map: " << element->location << " " << element->getType() << "\n";
    this->expandBounds(element->location);
}

std::vector<Building *> const &Network::getBuildings() const {
    return this->buildings;
}

std::vector<Roadway *> const &Network::getRoads() const {
    return this->roads;
}

void Network::addBuilding(Building *building) {
    this->buildings.push_back(building);
    if (this->spatialMap.find(building->location) != this->spatialMap.end())
    {
        std::cout << "Cannot place two entities on the same location on the network\n";
        abort();
    }
    this->addToSpatialMap(building);
}

void Network::addRoadway(Roadway *roadway) {
    this->roads.push_back(roadway);
    if (this->spatialMap.find(roadway->location) != this->spatialMap.end())
    {
        std::cout << "Cannot place two entities on the same location on the network\n";
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
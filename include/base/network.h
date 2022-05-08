#ifndef NETWORK
#define NETWORK

#include <iostream>
#include <math.h>
#include <vector>
#include <unordered_map>

#include "base/spatial.h"
#include "base/world_elements.h"

namespace world {

    class Network
    {
    private:
        std::vector<Building *> buildings;
        std::vector<Roadway *> roads;
        std::unordered_map<Location, WorldElement *, pair_hash> spatialMap;

        std::pair<Location, Location> bounds;
        void expandBounds(Location outermost);
        void addToSpatialMap(WorldElement *element);

    public:
        Network();

        const std::unordered_map<Location, WorldElement *, pair_hash> &getSpatialMap() const;
        const std::vector<Building *> &getBuildings() const;
        const std::vector<Roadway *> &getRoads() const;

        bool hasElementAt(Location loc) const;
        const WorldElement *getElementAt(Location loc) const;

        void addBuilding(Building *building);
        void addRoadway(Roadway *roadway);

        std::pair<Location, Location> Bounds() const { return bounds; }
    };

}

#endif
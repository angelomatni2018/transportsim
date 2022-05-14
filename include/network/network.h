#ifndef NETWORK
#define NETWORK

#include <iostream>
#include <math.h>
#include <vector>
#include <unordered_map>

#include "base/spatial.h"
#include "network/world_elements.h"

namespace world {

    class Network
    {
    private:
        std::vector<Building *> buildings;
        std::vector<Roadway *> roads;

        // Every location in this map is aligned to STRUCTURE_BASE_SIZE_UNIT (see world_elements.h)
        std::unordered_map<Location, WorldElement *, pair_hash> spatialMap;

        std::pair<Location, Location> bounds;
        void expandBounds(Location outermost);
        void addToSpatialMap(WorldElement *element);

    public:
        Network();

        const std::unordered_map<Location, WorldElement *, pair_hash> &SpatialMap() const;
        const std::vector<Building *> &Buildings() const;
        const std::vector<Roadway *> &Roads() const;

        bool HasStructureAt(Location loc) const;
        const WorldElement *StructureAt(Location loc) const;

        void AddBuilding(Building *building);
        void AddRoadway(Roadway *roadway);

        std::pair<Location, Location> Bounds() const { return bounds; }
    };

}

#endif
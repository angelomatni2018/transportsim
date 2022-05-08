#ifndef NEAREST_BUILDING_ITERATOR
#define NEAREST_BUILDING_ITERATOR

#include "base/network.h"
#include "base/world_elements.h"

#include <queue>

namespace world {
    class NearestBuildingIterator {
    private:
        const Network &network;
        int maxRadiusToCheck;
        const Building *target;

        std::queue<const Building *> nearests;
        int radiusToCheck;

        void checkToQueueBuilding(Location loc);

    public:
        NearestBuildingIterator(const Network &network, const Building *target);
        const Building *next();
    };
}

#endif

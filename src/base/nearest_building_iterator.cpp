#include "base/nearest_building_iterator.h"

using namespace world;

NearestBuildingIterator::NearestBuildingIterator(const Network &network, const Building *target)
    : network{network}, target{target}, nearests{} {
    maxRadiusToCheck = std::max(
        network.Bounds().second.first - network.Bounds().first.first,
        network.Bounds().second.second - network.Bounds().second.first
    );
    radiusToCheck = 1;
}

const Building *NearestBuildingIterator::next() {
    auto targetLoc = target->getLocation();
    // Check all coordinates a manhattan distance of radius away from target
    for (; nearests.empty() && radiusToCheck <= maxRadiusToCheck; ++radiusToCheck) {
        // Check the leftmost and rightmost coordinates
        checkToQueueBuilding(targetLoc + std::make_pair(-radiusToCheck, 0));
        checkToQueueBuilding(targetLoc + std::make_pair(radiusToCheck, 0));
        // Exclusive range i in (-radius, radius), j being such that abs(i + j) = radiusToCheck
        // i + j = radiusToCheck OR i + j = -radiusToCheck ==> j = +/-radiusToCheck - i
        for (int i = -radiusToCheck + 1; i < radiusToCheck; ++i) {
            checkToQueueBuilding(targetLoc + std::make_pair(i, radiusToCheck - i));
            checkToQueueBuilding(targetLoc + std::make_pair(i, -radiusToCheck - i));
        }
    }

    if (nearests.empty()) return nullptr;
    auto nearest = nearests.front();
    nearests.pop();
    return nearest;
}

void NearestBuildingIterator::checkToQueueBuilding(Location loc) {
    if (network.hasElementAt(loc)) {
        auto element = network.getElementAt(loc);
        if (auto building = dynamic_cast<const Building *>(element); building != nullptr) {
            nearests.push(building);
        }
    }
}
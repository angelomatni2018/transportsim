#include "network/nearest_building_iterator.h"
#include "spdlog/spdlog.h"

using namespace world;

NearestBuildingIterator::NearestBuildingIterator(const Network &network, const Building *target)
    : network{network}, target{target}, nearests{}, frontier{}, visited{} {

    auto targetLoc = target->PrimaryLocation();
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            auto loc = targetLoc + STRUCTURE_BASE_SIZE_UNIT * Location{i, j};
            checkToQueueRoadway(loc);
        }
    }
}

const Building *NearestBuildingIterator::Next() {
    while (nearests.empty() && !frontier.empty()) {
        int iters = 10;
        while (--iters >= 0 && !frontier.empty()) {
            auto roadway = frontier.front();
            frontier.pop();
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    auto loc = roadway->PrimaryLocation() + STRUCTURE_BASE_SIZE_UNIT * Location{i, j};
                    // spdlog::trace("Checking {}", to_string(loc));
                    checkToQueueRoadway(loc);
                    checkToQueueBuilding(loc);
                }
            }
        }
    }

    if (nearests.empty()) return nullptr;
    auto nearest = nearests.front();
    nearests.pop();
    return nearest;
}

void NearestBuildingIterator::checkToQueueRoadway(Location loc) {
    if (network.HasStructureAt(loc)) {
        auto element = network.StructureAt(loc);
        // spdlog::trace("Structure of type {} at {}", element->GetType(), to_string(element->PrimaryLocation()));
        if (element->IsType(Roadway::Type)) {
            auto roadway = (const Roadway *)element;
            if (visited.find(roadway) == visited.end()) {
                frontier.push(roadway);
                visited.emplace(roadway);
                // spdlog::trace("Roadway on frontier at {}", to_string(roadway->PrimaryLocation()));
            }
        }
    }
}

void NearestBuildingIterator::checkToQueueBuilding(Location loc) {
    if (network.HasStructureAt(loc)) {
        auto element = network.StructureAt(loc);
        if (element->IsType(Building::Type)) {
            nearests.push((const Building *)element);
        }
    }
}
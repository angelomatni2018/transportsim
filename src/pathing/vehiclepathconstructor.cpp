#include "pathing/vehiclepathconstructor.h"

using namespace world;

VehiclePathConstructor::VehiclePathConstructor(
    const Network &network,
    std::pair<CommercialBuilding *, ResidentialBuilding *> visit,
    double startTime,
    double timeInterval
) : currentTime{startTime}, timeInterval{timeInterval} {
    path = new Path();
    Pathfinder pathfinder(network);
    auto &[comm, res] = visit;
    auto pathLocs = pathfinder.solve(network, res->PrimaryLocation(), comm->PrimaryLocation());
    spdlog::trace("Path found from ({}) -> ({})", to_string(pathLocs[0]), to_string(pathLocs[pathLocs.size() - 1]));

    Location beforeResidentialBuilding = pathLocs[0] - (pathLocs[1] - pathLocs[0]);
    appendEventsThrough(beforeResidentialBuilding, pathLocs[0], pathLocs[1]);

    // residential building to commercial building
    auto lastIdx = pathLocs.size() - 1;
    for (int i = 1; i < lastIdx; ++i) {
        appendEventsThrough(pathLocs[i-1], pathLocs[i], pathLocs[i+1]);
    }

    // u-turn
    appendEventsThrough(pathLocs[lastIdx - 1], pathLocs[lastIdx], pathLocs[lastIdx - 1]);

    // commercial building to residential building
    for (int i = lastIdx; i > 1; --i) {
        appendEventsThrough(pathLocs[i], pathLocs[i-1], pathLocs[i-2]);
    }

    appendEventsThrough(pathLocs[1], pathLocs[0], beforeResidentialBuilding);
}

VehiclePathConstructor::~VehiclePathConstructor() {
    // delete path;
}

void VehiclePathConstructor::appendEventsThrough(Location structureFrom, Location structureAt, Location structureTo) {
    auto primaryLocs = RoadSegment(structureAt).LocationsThrough(structureFrom, structureTo);
    for (auto primaryLoc : primaryLocs) {
        appendEvent({primaryLoc});
    }
}

void VehiclePathConstructor::appendEvent(std::vector<Location> locations) {
    path->Append(locations, currentTime);
    currentTime += timeInterval;
}

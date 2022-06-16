#include "pathing/vehiclepathconstructor.h"

using namespace world;

bool VehiclePathConstructor::Construct(Path* path, const Network& network, int id, std::pair<CommercialBuilding*, ResidentialBuilding*> visit,
                                       double startTime, double timeInterval) {
  auto currentTime = startTime;

  auto appendEvent = [&](std::vector<Location> locations) {
    path->Append(locations, currentTime);
    currentTime += timeInterval;
  };
  auto appendEventsThrough = [&](Location structureFrom, Location structureAt, Location structureTo) {
    auto primaryLocs = RoadSegment(structureAt).LocationsThrough(structureFrom, structureTo);
    for (auto primaryLoc : primaryLocs) {
      appendEvent({primaryLoc});
    }
  };

  RoadPathfinder pathfinder;
  auto& [comm, res] = visit;
  // spdlog::trace("Path finding from ({}) -> ({})", to_string(res->PrimaryLocation()), to_string(comm->PrimaryLocation()));
  auto pathLocs = pathfinder.Solve(network, res->PrimaryLocation(), comm->PrimaryLocation());
  if (pathLocs.size() < 3) {
    return false;
  }
  // spdlog::trace("Path found from ({}) -> ({})", to_string(pathLocs[0]), to_string(pathLocs[pathLocs.size() - 1]));

  // Prepend and append a unique location so that if multiple vehicles are spawned at the same "point"
  // they funnel in and don't cause the reconciler to fail
  auto uniqueLocInMiddleOfNowhere = Location{INT32_MIN + id, 0};
  appendEvent({uniqueLocInMiddleOfNowhere});

  Location beforeResidentialBuilding = pathLocs[0] - (pathLocs[1] - pathLocs[0]);
  appendEventsThrough(beforeResidentialBuilding, pathLocs[0], pathLocs[1]);

  // residential building to commercial building
  auto lastIdx = pathLocs.size() - 1;
  for (int i = 1; i < lastIdx; ++i) {
    appendEventsThrough(pathLocs[i - 1], pathLocs[i], pathLocs[i + 1]);
  }

  // u-turn
  appendEventsThrough(pathLocs[lastIdx - 1], pathLocs[lastIdx], pathLocs[lastIdx - 1]);

  // commercial building to residential building
  for (int i = lastIdx; i > 1; --i) {
    appendEventsThrough(pathLocs[i], pathLocs[i - 1], pathLocs[i - 2]);
  }

  appendEventsThrough(pathLocs[1], pathLocs[0], beforeResidentialBuilding);

  // Prepend and append a unique location so that if multiple vehicles are spawned at the same "point"
  // they funnel in and don't cause the reconciler to fail
  appendEvent({uniqueLocInMiddleOfNowhere});

  for (auto i = 2; i < path->orderedPathEvents->size() - 1; ++i) {
    path->orderedPathEvents[i]->locations.push_back(path->orderedPathEvents[i - 1]->locations[0]);
  }
  return true;
}

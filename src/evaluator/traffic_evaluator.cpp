#include "evaluator/traffic_evaluator.h"

#include "spdlog/spdlog.h"

using namespace world;

TrafficEvaluator::TrafficEvaluator(const Network& network, double spawnInterval, double vehicleSpeed)
    : network{network}, spawnInterval{spawnInterval}, spawner{spawnInterval}, vehicleSecondsPerUnit{1.0 / vehicleSpeed / STRUCTURE_BASE_SIZE_UNIT} {}

bool TrafficEvaluator::canContinueToSpawn(double secondsElapsed) {
  auto spawns = spawner.Spawn(network, spawnInterval);
  // spdlog::trace("Spawning at {}: {}", secondsElapsed, spawns.size());
  auto backlogs = spawner.CommercialBuildingsWithBacklog();
  if (backlogs.size() > 0) {
    // spdlog::trace("TrafficEvaluator backlogs for commercial building(s):");
    // for (auto &[comm, backlogCount] : backlogs) {
    //     spdlog::trace("{}: {}", to_string(comm->PrimaryLocation()), backlogCount);
    // }
    return false;
  }

  for (auto& [comm, res] : spawns) {
    auto path = this->activeVehiclePaths.Add(Path());
    if (!VehiclePathConstructor::Construct(path, this->network, nextPathId++, {comm, res}, secondsElapsed, vehicleSecondsPerUnit)) {
      spdlog::trace("TrafficEvaluator failed to path; this should not happen");
      return false;
    }
    this->pathToVisit[path] = {comm, res};
    auto pathLength = path->orderedPathEvents->size();
    // spdlog::trace("Path spawned: res at {} to comm at {} (path length {}) time range: {} to {}",
    //     to_string(res->PrimaryLocation()), to_string(comm->PrimaryLocation()), pathLength,
    //     path->orderedPathEvents[0]->timeAtPoint, path->orderedPathEvents.back()->timeAtPoint);
  }

  if (spawns.size() > 0) {
    if (!PathReconciler().Reconcile(this->activeVehiclePaths.Get())) {
      spdlog::trace("TrafficEvaluator failed to reconcile; this should not happen");
      return false;
    }
  }
  return true;
}

double TrafficEvaluator::TimeBeforeOverload(int maxSeconds) {
  auto cleanupPaths = [&](std::unordered_set<Path*> cleanups) {
    for (auto toCleanup : cleanups) {
      auto& [comm, res] = this->pathToVisit[toCleanup];
      // spdlog::trace("Path ended: res at {} time completed: {}",
      //     to_string(res->PrimaryLocation()), toCleanup->orderedPathEvents.back()->timeAtPoint);
      comm->RemoveOccupant();
      res->RemoveOccupant();
      this->pathToVisit.erase(toCleanup);
      this->activeVehiclePaths.Remove(toCleanup);
    }
  };

  this->activeVehiclePaths = PtrSet<Path>();
  double secondsElapsed = 0.0;
  while (secondsElapsed < maxSeconds && canContinueToSpawn(secondsElapsed)) {
    secondsElapsed += spawnInterval;

    std::unordered_set<Path*> deletes;
    for (auto path : this->activeVehiclePaths.Get()) {
      if (ltEqualish(path->orderedPathEvents->back()->timeAtPoint, secondsElapsed)) {
        deletes.emplace(path);
      }
    }
    cleanupPaths(deletes);
  }

  if (equalish(secondsElapsed, maxSeconds)) {
    spdlog::debug("TrafficEvaluator::TimeBeforeOverload reached max time threshold of {} seconds", maxSeconds);
  }
  cleanupPaths(std::unordered_set<Path*>(this->activeVehiclePaths.Get()));
  return secondsElapsed;
}
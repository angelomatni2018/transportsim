#ifndef TRAFFIC_EVALUATOR
#define TRAFFIC_EVALUATOR

#include "base/spatial.h"
#include "network/network.h"
#include "pathing/pathreconciler.h"
#include "pathing/vehiclepathconstructor.h"
#include "simulator/visit_spawner.h"

namespace world {

class TrafficEvaluator {
private:
  const Network& network;
  VisitSpawner spawner;
  const double spawnInterval;
  const double vehicleSecondsPerUnit;
  std::unordered_set<Path*> activeVehiclePaths;
  std::unordered_map<Path*, std::pair<CommercialBuilding*, ResidentialBuilding*>> pathToVisit;
  int nextPathId = 0;

  bool canContinueToSpawn(double secondsElapsed);

public:
  TrafficEvaluator(const Network& network, double spawnInterval, double vehicleSpeed);

  double TimeBeforeOverload(int maxSeconds = MAX_SECONDS);

  static constexpr int MAX_SECONDS = 3600 * 24;
};

} // namespace world

#endif
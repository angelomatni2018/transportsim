#include "evaluator/traffic_evaluator.h"

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "external/catch.hpp"

#include "spdlog/spdlog.h"

using namespace world;

bool StraightLineOverwhelmed() {
  auto scaled = [](Location loc) { return STRUCTURE_BASE_SIZE_UNIT * loc; };
  Network network;
  auto comm = network.Add(CommercialBuilding(10, scaled({1, 1}), scaled({5, 0})));
  auto res = network.Add(ResidentialBuilding(5, scaled({1, 1}), scaled({0, 0})));
  RoadSegment* road1 = static_cast<RoadSegment*>(network.Add(RoadSegment(scaled({1, 0}))));
  RoadSegment* road2 = static_cast<RoadSegment*>(network.Add(RoadSegment(scaled({2, 0}))));
  RoadSegment* road3 = static_cast<RoadSegment*>(network.Add(RoadSegment(scaled({3, 0}))));
  RoadSegment* road4 = static_cast<RoadSegment*>(network.Add(RoadSegment(scaled({4, 0}))));

  auto check = [](TrafficEvaluator& evaluator, double expected = 100) -> bool {
    auto actual = evaluator.TimeBeforeOverload(100);
    if (!equalish(actual, expected)) {
      spdlog::error("StraightLineOverwhelmed: time to overwhelm: expected {} actual {}", expected, actual);
      return false;
    }
    return true;
  };

  double vehicleTilesPerSecond = 2.15;
  double secondsBetweenVisits = 0.999;
  TrafficEvaluator evaluatorTooFast(network, secondsBetweenVisits, vehicleTilesPerSecond);

  vehicleTilesPerSecond = 2.15;
  secondsBetweenVisits = 1.0;
  TrafficEvaluator evaluatorJustRight(network, secondsBetweenVisits, vehicleTilesPerSecond);

  return check(evaluatorJustRight) && check(evaluatorTooFast, 0.999 * 5);
}

TEST_CASE("Straight line road with spawns faster than travel time is overwhelmed", "[]") {
  AddSegfaultHandler();
  spdlog::set_level(spdlog::level::trace);
  REQUIRE(StraightLineOverwhelmed() == true);
}

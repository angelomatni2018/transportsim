#include "pathing/roadpathfinder.h"
#include <assert.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "external/catch.hpp"

#include "spdlog/spdlog.h"

using namespace world;

bool SimplePathFound() {
  auto scaled = [](Location loc) { return STRUCTURE_BASE_SIZE_UNIT * loc; };
  Network network;
  auto comm = network.Add(CommercialBuilding(10, scaled({1, 1}), scaled({5, 0})));
  auto res = network.Add(ResidentialBuilding(5, scaled({1, 1}), scaled({0, 0})));
  RoadSegment* road1 = static_cast<RoadSegment*>(network.Add(RoadSegment(scaled({1, 0}))));
  RoadSegment* road2 = static_cast<RoadSegment*>(network.Add(RoadSegment(scaled({2, 0}))));
  RoadSegment* road3 = static_cast<RoadSegment*>(network.Add(RoadSegment(scaled({3, 0}))));
  RoadSegment* road4 = static_cast<RoadSegment*>(network.Add(RoadSegment(scaled({4, 0}))));
  for (auto road : std::initializer_list<RoadSegment*>{road1, road2, road3, road4}) {
    road->SetDirections(Direction::West | Direction::East);
  }

  RoadPathfinder pathfinder;
  auto solution = pathfinder.Solve(network, scaled({0, 0}), scaled({5, 0}));
  if (solution.size() != 6) {
    return false;
  }

  // A road in the middle of the path no longer has eastwardly outgoing direction
  road3->SetDirections(Direction::West);
  solution = pathfinder.Solve(network, scaled({0, 0}), scaled({5, 0}));
  if (solution.size() != 0) {
    return false;
  }

  return true;
}

TEST_CASE("Simple path is found", "[]") {
  // spdlog::set_level(spdlog::level::trace);
  REQUIRE(SimplePathFound() == true);
}
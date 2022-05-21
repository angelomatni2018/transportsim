#include "pathing/roadpathfinder.h"
#include <assert.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "external/catch.hpp"

#include "spdlog/spdlog.h"

using namespace world;

bool SimplePathFound() {
  auto scaled = [](Location loc) { return STRUCTURE_BASE_SIZE_UNIT * loc; };
  Network network;
  CommercialBuilding comm(10, scaled({1, 1}), scaled({5, 0}));
  network.AddBuilding(&comm);
  ResidentialBuilding res(5, scaled({1, 1}), scaled({0, 0}));
  network.AddBuilding(&res);
  RoadSegment road1(scaled({1, 0}));
  RoadSegment road2(scaled({2, 0}));
  RoadSegment road3(scaled({3, 0}));
  RoadSegment road4(scaled({4, 0}));
  for (auto road : std::initializer_list<RoadSegment*>{&road1, &road2, &road3, &road4}) {
    network.AddRoadway(road);
    road->SetDirections(Direction::West | Direction::East);
  }

  RoadPathfinder pathfinder(network);
  auto solution = pathfinder.Solve(network, scaled({0, 0}), scaled({5, 0}));
  if (solution.size() != 6) {
    return false;
  }

  // A road in the middle of the path no longer has eastwardly outgoing direction
  road3.SetDirections(Direction::West);
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
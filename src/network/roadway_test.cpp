#include "network/world_elements.h"
#include <assert.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "external/catch.hpp"

#include "spdlog/spdlog.h"

using namespace world;

bool RoadSegmentsDirectionsAligned() {
  RoadSegment seg1({0, 0});
  RoadSegment seg2({1, 0});
  auto connects = [&]() { return seg1.CanDirectionsConnect(&seg2); };
  if (!connects()) {
    spdlog::trace("RoadSegment::CanDirectionsConnect default segments should connect to all adjacent segments");
    return false;
  }

  seg1.SetDirections(Direction::East);
  seg2.SetDirections(Direction::West);
  if (!connects()) {
    spdlog::trace("RoadSegment::CanDirectionsConnect east outgoing -> west incoming should connect");
    return false;
  }

  seg1.SetDirections(Direction::East);
  seg2.SetDirections(Direction::NorthWest);
  if (connects()) {
    spdlog::trace("RoadSegment::CanDirectionsConnect east outgoing -> northwest incoming should NOT connect");
    return false;
  }

  seg1.SetDirections(Direction::SouthEast);
  seg2.SetDirections(Direction::SouthWest);
  if (!connects()) {
    spdlog::trace("RoadSegment::CanDirectionsConnect zig zag eastwardly (southeast outgoing -> southwest incoming) should connect");
    return false;
  }

  seg1 = RoadSegment({0, 0});
  seg2 = RoadSegment({1, -1});
  seg1.SetDirections(Direction::South);
  seg2.SetDirections(Direction::West);
  if (connects()) {
    spdlog::trace("RoadSegment::CanDirectionsConnect zig zag south-eastwardly, diagonal instead of a general cardinal direction, should NOT connect");
    return false;
  }

  return true;
}

TEST_CASE("Road segments next to each other are only 'connected' if their directions are aligned", "[]") {
  spdlog::set_level(spdlog::level::trace);
  REQUIRE(RoadSegmentsDirectionsAligned() == true);
}
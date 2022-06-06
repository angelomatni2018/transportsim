#include "base/geometric_median.h"
#include <assert.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "external/catch.hpp"

#include "spdlog/spdlog.h"

using namespace world;

bool ThreePoints() {
  auto points = std::unordered_set<Point*>{new Point{0, -1}, new Point{0, 1}, new Point{1, 0}};
  auto median = GeometricMedian::Median(points);
  for (auto point : points)
    delete point;
  points.clear();

  // Median will form a 60 angle between x axis and line to 0,1 (and likewise for 0,-1)
  // Therefore, tan(60) = 1 / (distance along x-axis from 0)
  auto expectedMedian = Point{1 / tan(M_PI / 3), 0};
  if (!equalish(median, expectedMedian)) {
    spdlog::trace("ThreePoints: median expected {} got {}", to_string(expectedMedian), to_string(median));
    return false;
  }
  return true;
}

TEST_CASE("Geometric median for 3 points", "[]") {
  // spdlog::set_level(spdlog::level::trace);
  REQUIRE(ThreePoints() == true);
}

bool FourTerminalsConnectedWithTwoSteinerPoints() {
  auto steiner1Expected = Point{0.3, 0.5};
  auto steiner2Expected = Point{1.0, 0.5};
  auto steiner1Actual = new Point{0, 0.5};
  auto steiner2Actual = new Point{1.3, 0.5};
  auto cluster1 = std::unordered_set<Point*>{new Point{0, 0}, new Point{0, 1}, steiner2Actual};
  auto cluster2 = std::unordered_set<Point*>{new Point{1.3, 0}, new Point{1.3, 1}, steiner1Actual};

  PointToPosition steiner1{cluster1, steiner1Actual};
  PointToPosition steiner2{cluster2, steiner2Actual};
  GeometricMedian::PositionAtMediansToMinimizeSumOfDistances({&steiner1, &steiner2});
  if (euclidianDistance(*steiner1Actual, steiner1Expected) > 0.1) {
    spdlog::trace("FourTerminalsConnectedWithTwoSteinerPoints: steiner 1 expected {} got {}", to_string(steiner1Expected),
                  to_string(*steiner1Actual));
    return false;
  }
  if (euclidianDistance(*steiner2Actual, steiner2Expected) > 0.1) {
    spdlog::trace("FourTerminalsConnectedWithTwoSteinerPoints: steiner 2 expected {} got {}", to_string(steiner2Expected),
                  to_string(*steiner2Actual));
    return false;
  }
  return true;
}

TEST_CASE("Approximate 2 steiner point's placement when connecting 4 points", "[]") {
  // spdlog::set_level(spdlog::level::trace);
  REQUIRE(FourTerminalsConnectedWithTwoSteinerPoints() == true);
}
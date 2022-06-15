#include "base/geometric_median.h"
#include <assert.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "external/catch.hpp"

#include "spdlog/spdlog.h"

using namespace world;

bool ThreePoints() {
  Pool<Point> p;
  auto points = std::unordered_set<Point*>{p.With(Point(0.f, -1.f)), p.With(Point(0.f, 1.f)), p.With(Point(1.f, 0.f))};
  auto median = GeometricMedian::Median(points);

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
  Pool<Point> p;
  auto steiner1Expected = p.With(Point{0.3, 0.5});
  auto steiner2Expected = p.With(Point{1.0, 0.5});
  auto steiner1Actual = p.With(Point{0, 0.5});
  auto steiner2Actual = p.With(Point{1.3, 0.5});
  auto cluster1 = {p.With(Point{0, 0}), p.With(Point{0, 1}), steiner2Actual};
  auto cluster2 = {p.With(Point{1.3, 0}), p.With(Point{1.3, 1}), steiner1Actual};

  Pool<PointToPosition> ptp;
  auto points =
      std::unordered_set<PointToPosition*>{ptp.With(PointToPosition{cluster1, steiner1Actual}), ptp.With(PointToPosition{cluster2, steiner2Actual})};
  GeometricMedian::PositionAtMediansToMinimizeSumOfDistances(points);
  if (euclidianDistance(*steiner1Actual, *steiner1Expected) > 0.1) {
    spdlog::trace("FourTerminalsConnectedWithTwoSteinerPoints: steiner 1 expected {} got {}", to_string(*steiner1Expected),
                  to_string(*steiner1Actual));
    return false;
  }
  if (euclidianDistance(*steiner2Actual, *steiner2Expected) > 0.1) {
    spdlog::trace("FourTerminalsConnectedWithTwoSteinerPoints: steiner 2 expected {} got {}", to_string(*steiner2Expected),
                  to_string(*steiner2Actual));
    return false;
  }
  return true;
}

TEST_CASE("Approximate 2 steiner point's placement when connecting 4 points", "[]") {
  // spdlog::set_level(spdlog::level::trace);
  REQUIRE(FourTerminalsConnectedWithTwoSteinerPoints() == true);
}
#include "base/geometric_median.h"
#include "spdlog/spdlog.h"

using namespace world;

Point GeometricMedian::Median(std::unordered_set<Point*>& cluster, float threshold) {
  // Initially position at mean of cluster
  Point newMedian;
  positionAtMean(cluster, newMedian);
  Point oldMedian = Point{INFINITY, INFINITY};
  int iters = 0;
  while (++iters < MAX_ITERATIONS && euclidianDistance(oldMedian, newMedian) > threshold) {
    oldMedian = newMedian;
    newMedian = weiszfeldStep(oldMedian, cluster);
    spdlog::trace("{} -> {}", to_string(oldMedian), to_string(newMedian));
  }

  if (iters == MAX_ITERATIONS) {
    spdlog::error("GeometricMedian::Median did not converge");
    abort();
  }

  return newMedian;
}

void GeometricMedian::PositionAtMediansToMinimizeSumOfDistances(std::unordered_set<PointToPosition*>& pointsToPosition, float threshold) {
  for (auto& pointToPosition : pointsToPosition) {
    positionAtMean(pointToPosition->clusterPoints, *pointToPosition->pointToPositionAtMedian);
  }

  double maxAdjustment = INFINITY;
  int iters = 0;
  while (++iters < MAX_ITERATIONS && maxAdjustment > threshold) {
    // Compute adjustments
    maxAdjustment = 0.0;
    for (auto pointToPosition : pointsToPosition) {
      pointToPosition->proposedAdjustedPosition = weiszfeldStep(*pointToPosition->pointToPositionAtMedian, pointToPosition->clusterPoints);
      // for (auto& p : pointToPosition->clusterPoints) {
      // spdlog::trace("Cluster point {}", to_string(*p));
      // }
      // spdlog::trace("Proposed {}", to_string(pointToPosition->proposedAdjustedPosition));
      auto adjustment = euclidianDistance(pointToPosition->proposedAdjustedPosition, *pointToPosition->pointToPositionAtMedian);
      if (maxAdjustment < adjustment) {
        maxAdjustment = adjustment;
      }
    }

    // Simultaneously apply the adjustments
    for (auto pointToPosition : pointsToPosition) {
      pointToPosition->pointToPositionAtMedian->first = pointToPosition->proposedAdjustedPosition.first;
      pointToPosition->pointToPositionAtMedian->second = pointToPosition->proposedAdjustedPosition.second;
    }
  }

  if (iters == MAX_ITERATIONS) {
    spdlog::error("GeometricMedian::PositionAtMediansToMinimizeSumOfDistances did not converge");
    abort();
  }
}

void GeometricMedian::positionAtMean(std::unordered_set<Point*>& cluster, Point& mean) {
  mean.first = 0;
  mean.second = 0;
  for (auto point : cluster) {
    mean.first += point->first;
    mean.second += point->second;
  }
  mean.first = mean.first / cluster.size();
  mean.second = mean.second / cluster.size();
}

Point GeometricMedian::weiszfeldStep(Point currentMedian, std::unordered_set<Point*>& cluster) {
  float sumOfInverseDistances = 0.0;
  Point sumOfWeightedPointsByDistanceToMedian{0, 0};
  for (auto point : cluster) {
    auto dist = euclidianDistance(*point, currentMedian);
    sumOfInverseDistances += (1.0 / std::max(dist, FLOAT_EPSILON));
    sumOfWeightedPointsByDistanceToMedian = sumOfWeightedPointsByDistanceToMedian + *point / std::max(dist, FLOAT_EPSILON);
  }
  return sumOfWeightedPointsByDistanceToMedian / sumOfInverseDistances;
}

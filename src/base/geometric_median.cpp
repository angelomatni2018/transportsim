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
    newMedian = weiszfeldStep(oldMedian, cluster, threshold);
    spdlog::trace("{} -> {}", to_string(oldMedian), to_string(newMedian));
  }

  if (iters == MAX_ITERATIONS) {
    spdlog::trace("GeometricMedian::Median did not converge");
    throw "GeometricMedian::Median did not converge";
  }

  return newMedian;
}

void GeometricMedian::PositionAtMediansToMinimizeSumOfDistances(std::unordered_set<PointToPosition*>& pointsToPosition, float threshold) {
  for (auto& pointToPosition : pointsToPosition) {
    if (pointToPosition->clusterPoints.size() == 0) {
      spdlog::trace("GeometricMedian: No empty clusters allowed");
      throw "GeometricMedian: No empty clusters allowed";
    }
    positionAtMean(pointToPosition->clusterPoints, *pointToPosition->pointToPositionAtMedian);
  }

  double maxAdjustment = INFINITY;
  int iters = 0;
  while (++iters < MAX_ITERATIONS && maxAdjustment > threshold) {
    // Compute adjustments
    maxAdjustment = 0.0;
    for (auto pointToPosition : pointsToPosition) {
      pointToPosition->proposedAdjustedPosition = weiszfeldStep(*pointToPosition->pointToPositionAtMedian, pointToPosition->clusterPoints, threshold);
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
    spdlog::trace("GeometricMedian::PositionAtMediansToMinimizeSumOfDistances did not converge");
    throw "GeometricMedian::PositionAtMediansToMinimizeSumOfDistances did not converge";
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

// Point logSumExp(std::unordered_set<Point*> points) {
//   if (points.size() == 0) {
//     return {0, 0};
//   }
//   Point maxVals(**points.begin());
//   for (auto pt : points) {
//     maxVals = {std::max(maxVals.first, pt->first), std::max(maxVals.first, pt->first)};
//   }
//   Point result{0, 0};
//   for (auto pt : points) {
//     auto offset = *pt - maxVals;
//     result = result + Point{exp(offset.first), exp(offset.second)};
//   }
//   return maxVals + Point{log(result.first), log(result.second)};
// }

Point GeometricMedian::weiszfeldStep(Point currentMedian, std::unordered_set<Point*>& cluster, float smallestDistance) {
  float sumOfInverseDistances = 0.0;
  Point sumOfWeightedPointsByDistanceToMedian{0, 0};
  for (auto point : cluster) {
    auto dist = euclidianDistance(*point, currentMedian);
    sumOfInverseDistances += (1.0 / std::max(dist, smallestDistance));
    sumOfWeightedPointsByDistanceToMedian = sumOfWeightedPointsByDistanceToMedian + *point / std::max(dist, smallestDistance);
  }
  auto result = sumOfWeightedPointsByDistanceToMedian / sumOfInverseDistances;
  if (isnanf(result.first) || isnanf(result.second)) {
    spdlog::trace("GeometricMedian: weiszfeldStep overflow/underflow for median {}", to_string(currentMedian));
    for (auto point : cluster) {
      spdlog::trace("GeometricMedian: \tcluster point {}", to_string(*point));
    }
    throw "GeometricMedian: weiszfeldStep overflow/underflow";
  }
  return result;
}

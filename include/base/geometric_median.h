#ifndef GEOMETRIC_MEDIAN
#define GEOMETRIC_MEDIAN

#include "base/spatial.h"

namespace world {

class PointToPosition {
public:
  std::unordered_set<Point*> clusterPoints;
  Point* pointToPositionAtMedian;

  PointToPosition(Point* pointToPosition) : clusterPoints{}, pointToPositionAtMedian{pointToPosition} {}

  PointToPosition(std::unordered_set<Point*> points, Point* pointToPosition) : clusterPoints{points}, pointToPositionAtMedian{pointToPosition} {}

private:
  Point proposedAdjustedPosition;

  friend class GeometricMedian;
};

class GeometricMedian {
public:
  // Computed using Weiszfeld's algorithm.
  static Point Median(std::unordered_set<Point*>& cluster, float threshold = FLOAT_EPSILON / 10);

  // Performed using a modified Weiszfeld's algorithm, where iterations are applied simultaneously to all clusters until convergence.
  // The assumption is that points to position at the median can be members of other clusters as well,
  // so positioning each at its median sequentially would "over-adjust" the earlier adjusted clusters and not minimize the sum of distances
  static void PositionAtMediansToMinimizeSumOfDistances(std::unordered_set<PointToPosition*>& pointsToPosition, float threshold = FLOAT_EPSILON / 10);

private:
  static void positionAtMean(std::unordered_set<Point*>& cluster, Point& mean);
  static Point weiszfeldStep(Point currentMedian, std::unordered_set<Point*>& cluster);
};

} // namespace world

#endif
#ifndef KMEANS
#define KMEANS

#include "base/spatial.h"
#include <queue>

namespace world {

struct Cluster;

class KMeans {
private:
  int numClusters;
  std::unordered_map<Location, int, pair_hash> locToCluster;
  std::priority_queue<std::pair<float, Location>> remoteLocations;
  PtrSet<Cluster> clusters;

  struct Grouping {
    std::vector<int> numMembers;
    std::vector<Point> centroids;
  };
  Grouping grouping;

  void computeGroupingFromClusterMapping(int iteration);

  bool assignPointsToClosestCluster();

  static constexpr int MAX_ITERATIONS = 100000;

  void verifyInputs(int numPoints);
  void solve();

public:
  KMeans(const std::unordered_set<Location, pair_hash> locs, int c);
  KMeans(const std::vector<Location> locs, int c);

  const std::unordered_set<Cluster*>& Get();
};

struct Cluster {
  std::unordered_set<Location, pair_hash> members;
  Point centroid;

  Cluster() {}
  Cluster(std::unordered_set<Location, pair_hash> members, Point centroid) : members{members}, centroid{centroid} {}
};

}; // namespace world

#endif
#ifndef KMEANS
#define KMEANS

#include "base/spatial.h"

namespace world {

struct Cluster;

class KMeans {
private:
  int numClusters;
  std::unordered_map<Location, int, pair_hash> locToCluster;
  std::unordered_set<Cluster*> clusters;

  struct Grouping {
    std::vector<int> numMembers;
    std::vector<Point> centroids;
  };
  Grouping grouping;

  void computeGroupingFromClusterMapping(Grouping& g);

  bool assignPointsToClosestCluster();

public:
  KMeans(const std::unordered_set<Location, pair_hash> locs, int c);
  ~KMeans();

  std::unordered_set<Cluster*> Get();
};

struct Cluster {
  std::unordered_set<Location, pair_hash> members;
  Point centroid;

  Cluster() {}
  Cluster(std::unordered_set<Location, pair_hash> members, Point centroid) : members{members}, centroid{centroid} {}
};

}; // namespace world

#endif
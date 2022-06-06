#ifndef KMEANS
#define KMEANS

#include <iostream>
#include <math.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base/spatial.h"

namespace world {

struct Cluster;

class KMeans {
private:
  int numClusters;
  std::unordered_map<Location, int, pair_hash> locToCluster;

  struct Grouping {
    std::vector<int> numMembers;
    std::vector<Point> centroids;
  };
  Grouping grouping;

  void computeGroupingFromClusterMapping(Grouping& g);

  bool assignPointsToClosestCluster();

public:
  KMeans(std::unordered_set<Location, pair_hash> locs, int c);

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
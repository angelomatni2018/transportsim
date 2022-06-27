#ifndef AGGLOMERATIVE_CLUSTERING
#define AGGLOMERATIVE_CLUSTERING

#include "base/cluster.h"
#include "base/spatial.h"

namespace world {

class AgglomerativeClustering {
private:
  int numGoalClusters;
  std::unordered_map<Location, int, pair_hash> locToCluster;
  std::vector<Point> clusterCentroids;
  PtrSet<Cluster> clusters;

  void solve();

public:
  AgglomerativeClustering(const std::unordered_set<Location, pair_hash> locs, int c);
  AgglomerativeClustering(const std::vector<Location> locs, int c);

  const std::unordered_set<Cluster*>& Get();
};

} // namespace world

#endif
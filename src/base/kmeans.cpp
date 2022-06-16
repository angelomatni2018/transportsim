#include "base/kmeans.h"

using namespace world;

KMeans::KMeans(const std::unordered_set<Location, pair_hash> locs, int c) : numClusters{c}, locToCluster{} {
  // Initial cluster assignment (randomly chosen locations)
  auto clusterIdx = 0;
  for (auto loc : locs) {
    auto idx = (clusterIdx++ % numClusters);
    locToCluster[loc] = idx;
  }
  computeGroupingFromClusterMapping(grouping);

  while (assignPointsToClosestCluster()) {
    computeGroupingFromClusterMapping(grouping);
  }

  std::vector<std::unordered_set<Location, pair_hash>> clusterLocs;
  clusterLocs.resize(numClusters);
  for (auto& [loc, clusterIdx] : locToCluster) {
    clusterLocs[clusterIdx].emplace(loc);
  }
  for (auto locs : clusterLocs) {
    Point p{0.0, 0.0};
    for (auto& loc : locs) {
      p = p + Point{loc};
    }
    p = Point{p.first / locs.size(), p.second / locs.size()};
    clusters.Add(Cluster(locs, p));
  }
}

void KMeans::computeGroupingFromClusterMapping(Grouping& g) {
  for (int i = 0; i < numClusters; ++i) {
    g.centroids.push_back(Point(0, 0));
    g.numMembers.push_back(0);
  }

  for (auto& [loc, idx] : locToCluster) {
    g.centroids[idx] = g.centroids[idx] + Point{loc};
    g.numMembers[idx]++;
  }

  for (int i = 0; i < numClusters; ++i) {
    g.centroids[i] = (1.0 / g.numMembers[i]) * g.centroids[i];
  }
}

const std::unordered_set<Cluster*>& KMeans::Get() {
  return this->clusters.Get();
}

bool KMeans::assignPointsToClosestCluster() {
  auto changed = false;
  for (auto& [loc, oldIdx] : locToCluster) {
    float minDist = INFINITY;
    int minIdx = -1;
    for (int i = 0; i < numClusters; ++i) {
      auto distI = euclidianDistance(Point{loc}, grouping.centroids[i]);
      if (minDist > distI) {
        minDist = distI;
        minIdx = i;
      }
    }
    if (minIdx != oldIdx) {
      changed = true;
      locToCluster[loc] = minIdx;
    }
  }
  return changed;
}

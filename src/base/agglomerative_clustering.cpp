#include "base/agglomerative_clustering.h"

using namespace world;

AgglomerativeClustering::AgglomerativeClustering(const std::unordered_set<Location, pair_hash> locs, int c) : numGoalClusters{c} {
  int i = -1;
  clusterCentroids.resize(locs.size());
  for (auto loc : locs) {
    locToCluster[loc] = ++i;
    clusterCentroids[i] = Point(loc);
  }
}

AgglomerativeClustering::AgglomerativeClustering(const std::vector<Location> locs, int c) : numGoalClusters{c} {
  int i = -1;
  clusterCentroids.resize(locs.size());
  for (auto loc : locs) {
    locToCluster[loc] = ++i;
    clusterCentroids[i] = Point(loc);
  }
}

const std::unordered_set<Cluster*>& AgglomerativeClustering::Get() {
  if (clusters.Get().size() == 0) {
    solve();
  }
  return clusters.Get();
}

void AgglomerativeClustering::solve() {
  int numCurrentClusters = locToCluster.size();
  if (numCurrentClusters < numGoalClusters) {
    throw "AgglomerativeClustering: fewer points than clusters";
  }

  while (numCurrentClusters > numGoalClusters) {
    --numCurrentClusters;
    // spdlog::error("Aggregating clusters down to {}", numCurrentClusters);

    std::pair<int, int> minPair;
    float minDist = INFINITY;
    for (int i = 0; i < clusterCentroids.size(); ++i) {
      if (clusterCentroids[i].first == INFINITY)
        continue;
      for (int j = i + 1; j < clusterCentroids.size(); ++j) {
        if (clusterCentroids[j].first == INFINITY)
          continue;
        float dist = euclidianDistance(clusterCentroids[i], clusterCentroids[j]);
        if (dist < minDist) {
          minDist = dist;
          minPair = {i, j};
        }
      }
    }
    if (minDist == INFINITY) {
      spdlog::trace("AgglomerativeClustering: no closest pair of clusters? Current # clusters: {}", numCurrentClusters + 1);
      throw "AgglomerativeClustering: no closest pair of clusters?";
    }
    // spdlog::error("New pair to merge: {} and {}", minPair.first, minPair.second);

    Point newCentroid{0, 0};
    int numInCluster = 0;
    std::vector<Location> locsToChange;
    for (auto& [loc, idx] : locToCluster) {
      if (idx == minPair.first || idx == minPair.second) {
        newCentroid = newCentroid + Point(loc);
        ++numInCluster;
      }
      if (idx == minPair.second) {
        locsToChange.push_back(loc);
      }
    }
    for (auto loc : locsToChange) {
      locToCluster[loc] = minPair.first;
    }

    clusterCentroids[minPair.first] = newCentroid / numInCluster;
    clusterCentroids[minPair.second] = Point{INFINITY, INFINITY};
    // spdlog::error("Merged {} and {} into centroid {}", minPair.first, minPair.second, to_string(clusterCentroids[minPair.first]));
  }

  // spdlog::error("Building {} final clusters", numGoalClusters);
  std::vector<std::unordered_set<Location, pair_hash>> clusterLocs;
  clusterLocs.resize(locToCluster.size());
  for (auto& [loc, clusterIdx] : locToCluster) {
    // spdlog::error("Cluster {} has {}", clusterIdx, to_string(loc));
    clusterLocs[clusterIdx].emplace(loc);
  }
  for (int i = 0; i < clusterLocs.size(); ++i) {
    auto& locs = clusterLocs[i];
    if (locs.size() != 0) {
      clusters.Add(Cluster(locs, clusterCentroids[i]));
    }
  }
}
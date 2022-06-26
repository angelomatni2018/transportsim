#include "base/kmeans.h"

using namespace world;

KMeans::KMeans(const std::unordered_set<Location, pair_hash> locs, int c) : numClusters{c}, locToCluster{} {
  verifyInputs(locs.size());

  // Initial cluster assignment (randomly chosen locations)
  auto clusterIdx = 0;
  for (auto loc : locs) {
    auto idx = (clusterIdx++ % numClusters);
    locToCluster[loc] = idx;
  }
  solve();
}

KMeans::KMeans(const std::vector<Location> locs, int c) : numClusters{c}, locToCluster{} {
  verifyInputs(locs.size());

  // Initial cluster assignment (in-order chosen locations)
  auto clusterIdx = 0;
  for (auto loc : locs) {
    auto idx = (clusterIdx++ % numClusters);
    locToCluster[loc] = idx;
  }
  solve();
}

void KMeans::verifyInputs(int numPoints) {
  if (numPoints == 0) {
    spdlog::trace("KMeans requires at least one point");
    throw "KMeans requires at least one point";
  }
  if (numClusters > numPoints) {
    spdlog::trace("KMeans too many clusters {} requested for {} locations", numClusters, numPoints);
    throw "KMeans too many clusters for given locations";
  }
}

void KMeans::solve() {
  int iters = 0;
  computeGroupingFromClusterMapping(iters);
  while (++iters < MAX_ITERATIONS && assignPointsToClosestCluster()) {
    computeGroupingFromClusterMapping(iters);
  }
  if (iters == MAX_ITERATIONS) {
    spdlog::trace("KMeans did not converge");
    throw "KMeans did not converge";
  }

  std::vector<std::unordered_set<Location, pair_hash>> clusterLocs;
  clusterLocs.resize(numClusters);
  for (auto& [loc, clusterIdx] : locToCluster) {
    clusterLocs[clusterIdx].emplace(loc);
  }
  for (int i = 0; i < clusterLocs.size(); ++i) {
    auto& locs = clusterLocs[i];
    Point p{0.0, 0.0};
    for (auto& loc : locs) {
      p = p + Point{loc};
    }
    if (locs.size() == 0) {
      spdlog::trace("KMeans empty cluster {} in result", i);
      throw "KMeans empty cluster";
    }
    p = Point{p.first / locs.size(), p.second / locs.size()};
    clusters.Add(Cluster(locs, p));
  }
}

void KMeans::computeGroupingFromClusterMapping(int iteration) {
  // spdlog::trace("ITERATION");
  grouping = Grouping();
  for (int i = 0; i < numClusters; ++i) {
    grouping.centroids.push_back(Point(0, 0));
    grouping.numMembers.push_back(0);
  }

  for (auto& [loc, idx] : locToCluster) {
    // spdlog::trace("\tKMeans point {} in cluster {}", to_string(loc), idx);
    grouping.centroids[idx] = grouping.centroids[idx] + Point{loc};
    grouping.numMembers[idx]++;
  }

  for (int i = 0; i < numClusters; ++i) {
    if (grouping.numMembers[i] == 0) {
      if (remoteLocations.size() == 0) {
        spdlog::trace("KMeans remoteLocations should have numClusters={} # of points when first computing grouping", numClusters);
        throw "KMeans internal error";
      }
      auto [_, currentOutlier] = remoteLocations.top();
      remoteLocations.pop();
      grouping.centroids[i] = currentOutlier;
      locToCluster[currentOutlier] = i;
      grouping.numMembers[i]++;
      grouping.numMembers[locToCluster[currentOutlier]]--;
      continue;
    }
    grouping.centroids[i] = (1.0 / grouping.numMembers[i]) * grouping.centroids[i];
    // spdlog::trace("\tKMeans centroid {} for cluster {}", to_string(grouping.centroids[i]), i);
  }
}

const std::unordered_set<Cluster*>& KMeans::Get() {
  return this->clusters.Get();
}

bool KMeans::assignPointsToClosestCluster() {
  auto changed = false;
  std::unordered_map<Location, int, pair_hash> locToClusterNew;
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
    }
    locToClusterNew[loc] = minIdx;

    remoteLocations.push({minDist, loc});
    if (remoteLocations.size() > numClusters) {
      remoteLocations.pop();
    }
  }
  locToCluster = locToClusterNew;
  return changed;
}

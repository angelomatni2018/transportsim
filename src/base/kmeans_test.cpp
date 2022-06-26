#include "base/kmeans.h"
#include <assert.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "external/catch.hpp"

#include "spdlog/spdlog.h"

using namespace world;

bool verifyTwoClusters(Cluster* cluster1, Cluster* cluster2, Point mean1, Point mean2, int clusterSize1, int clusterSize2) {
  if (!(equalish(cluster1->centroid, mean1) ^ equalish(cluster2->centroid, mean1))) {
    spdlog::trace("TwoClusters: cluster centroids {} and {}; one must be at {}", to_string(cluster1->centroid), to_string(cluster2->centroid),
                  to_string(mean1));
    return false;
  }
  if (!(equalish(cluster1->centroid, mean2) ^ equalish(cluster2->centroid, mean2))) {
    spdlog::trace("TwoClusters: cluster centroids {} and {}; one must be at {}", to_string(cluster1->centroid), to_string(cluster2->centroid),
                  to_string(mean2));
    return false;
  }
  int c1 = cluster1->members.size();
  int c2 = cluster2->members.size();
  if (c1 != clusterSize1 || c2 != clusterSize2) {
    spdlog::trace("TwoClusters: cluster point counts {} and {} should be {} and {}", c1, c2, clusterSize1, clusterSize2);
    return false;
  }
  return true;
}

bool TwoClusters() {
  std::unordered_set<Location, pair_hash> locs;
  for (auto i = 0; i < 10; ++i) {
    locs.emplace(Location{0, i});
  }
  for (auto i = 0; i < 10; ++i) {
    locs.emplace(Location{10, i});
  }

  auto kmeans = KMeans(locs, 2);
  auto clusters = kmeans.Get();
  if (clusters.size() != 2) {
    spdlog::trace("TwoClusters: num clusters {}", clusters.size());
    return false;
  }

  auto cluster1 = *clusters.begin();
  auto cluster2 = *(++clusters.begin());
  auto mean1 = Point{0, 4.5};
  auto mean2 = Point{10, 4.5};
  return verifyTwoClusters(cluster1, cluster2, mean1, mean2, 10, 10);
}

TEST_CASE("Two clusters are correctly computed", "[]") {
  // spdlog::set_level(spdlog::level::trace);
  REQUIRE(TwoClusters() == true);
}

bool verifyNClusters(std::unordered_set<Cluster*> clusters, std::unordered_set<Point, pair_hash> means) {
  int meansFound = 0;
  for (auto cluster : clusters) {
    bool found = false;
    for (auto mean : means) {
      if (equalish(mean.first, cluster->centroid.first) && equalish(mean.second, cluster->centroid.second)) {
        meansFound++;
        found = true;
        break;
      }
    }
    if (!found) {
      spdlog::trace("KMeans: cluster centroid not expected: {}", to_string(cluster->centroid));
    }
  }

  if (meansFound == means.size()) {
    return true;
  }
  spdlog::trace("KMeans: expected means:");
  for (auto mean : means) {
    spdlog::trace("KMeans:\t {}", to_string(mean));
  }
  spdlog::trace("KMeans: actual means");
  for (auto cluster : clusters) {
    spdlog::trace("KMeans:\t {}", to_string(cluster->centroid));
  }
  return false;
}

bool ThreeClustersBadStartingSeed() {
  std::vector<Location> locs = {{1, 0}, {10, 0}, {11, 0}, {20, 0}, {20, 1}, {21, 0}};

  auto kmeans = KMeans(locs, 3);
  auto clusters = kmeans.Get();

  auto mean1 = Point{7.333333, 0};
  auto mean2 = Point{20, 0};
  auto mean3 = Point{20.5, 0.5};
  return verifyNClusters(clusters, {mean1, mean2, mean3});
}

TEST_CASE("Three clusters are correctly computed despite bad starting seed", "[]") {
  REQUIRE(ThreeClustersBadStartingSeed() == true);
}

TEST_CASE("More clusters than points requested results in exception", "[]") {
  // spdlog::set_level(spdlog::level::trace);
  try {
    auto kmeans = KMeans(std::vector{Location{0, 0}}, 2);
    REQUIRE(false == true);
  } catch (const char* msg) {
    spdlog::trace("Got expected error: {}", msg);
  }
}
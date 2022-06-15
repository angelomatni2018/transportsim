#include "base/kmeans.h"
#include <assert.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "external/catch.hpp"

#include "spdlog/spdlog.h"

using namespace world;

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
  if (cluster1->members.size() != 10 || cluster2->members.size() != 10) {
    spdlog::trace("TwoClusters: cluster point counts {} and {}", cluster1->members.size(), cluster2->members.size());
    return false;
  }

  return true;
}

TEST_CASE("Two clusters are correctly computed", "[]") {
  // spdlog::set_level(spdlog::level::trace);
  REQUIRE(TwoClusters() == true);
}

#ifndef CLUSTER
#define CLUSTER

#include "base/spatial.h"

namespace world {

struct Cluster {
  std::unordered_set<Location, pair_hash> members;
  Point centroid;

  Cluster() {}
  Cluster(std::unordered_set<Location, pair_hash> members, Point centroid) : members{members}, centroid{centroid} {}
};

}; // namespace world

#endif
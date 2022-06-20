#ifndef STEINER_POINT_SEARCH_BY_CLUSTERING
#define STEINER_POINT_SEARCH_BY_CLUSTERING

#include <unordered_map>
#include <unordered_set>

#include "base/geometric_median.h"
#include "base/kmeans.h"
#include "network/min_spanning_tree.h"
#include "pathing/vehiclepathconstructor.h"
#include "steiner_tree_solver/road_candidate_graph.h"
#include "steiner_tree_solver/virtual_graph.h"

namespace world {

class SteinerPointSearchByClustering {
public:
  static std::unique_ptr<Network> ConnectStructures(const Network& network);

private:
  static void addRoadsTo(const Network& inputNetwork, Network& outputNetwork);
};

} // namespace world

#endif
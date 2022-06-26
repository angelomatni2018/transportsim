#ifndef STEINER_POINT_SEARCH_BY_CLUSTERING
#define STEINER_POINT_SEARCH_BY_CLUSTERING

#include <unordered_map>
#include <unordered_set>

#include "base/geometric_median.h"
#include "base/kmeans.h"
#include "network/min_spanning_tree.h"
#include "steiner_tree_solver/road_candidate_graph.h"
#include "steiner_tree_solver/spanning_tree_solver.h"
#include "steiner_tree_solver/virtual_graph.h"

namespace world {

class SteinerPointSearchByClustering : public SpanningTreeSolver {
public:
  static std::unique_ptr<Network> ConnectStructures(const Network& network);
};

} // namespace world

#endif
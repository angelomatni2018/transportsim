#ifndef MIN_SPANNING_TREE
#define MIN_SPANNING_TREE

#include "base/spatial.h"
#include "network/graph.h"
#include <queue>

namespace world {

class MinimumSpanningTree {
public:
  MinimumSpanningTree(const NetworkGraph& graph);
  MinimumSpanningTree(const NetworkGraph& graph, std::function<bool(const NetworkEdge* a, const NetworkEdge* b)> comparator);

  const std::unordered_set<const NetworkEdge*>& MstEdges() const;

private:
  void prim(const NetworkGraph& graph);

  std::unordered_set<const WorldElement*> mstNodes;
  std::unordered_set<const NetworkEdge*> mstEdges;
  std::priority_queue<const NetworkEdge*, std::vector<const NetworkEdge*>, std::function<bool(const NetworkEdge*, const NetworkEdge*)>> queue;
};

} // namespace world

#endif

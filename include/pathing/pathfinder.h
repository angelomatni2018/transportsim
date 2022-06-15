#ifndef PATHFINDER
#define PATHFINDER

#include <algorithm>
#include <iostream>
#include <math.h>
#include <queue>
#include <unordered_map>
#include <vector>

#include "base/spatial.h"
#include "network/network.h"

namespace world {

class Pathfinder {
private:
  std::unordered_map<Location, std::unordered_map<Location, std::vector<Location>, pair_hash>, pair_hash> shortestPathFromLocToLoc;

protected:
  virtual double heuristic(Location start, Location end);

  virtual double actualCost(const Network& network, Location neighbor);

  virtual bool isValidNeighborToTraverse(const Network& network, Location current, Location neighbor, Location start, Location end);

  std::vector<Location> neighbors(const Network& network, Location current);

  std::vector<Location> retrace(Location start, Location end, std::unordered_map<Location, Location, pair_hash> connections);

public:
  // TODO: Move this to a eager-loading pathfinder derived class:
  // const std::vector<Location>& ShortestPath(const Location& from, const Location& to);

  std::vector<Location> Solve(const Network& network, Location start, Location end);
};

} // namespace world

#endif
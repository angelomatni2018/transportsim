#include "pathing/pathfinder.h"
#include "spdlog/spdlog.h"

using namespace world;

// Pathfinder::Pathfinder() : shortestPathFromLocToLoc{} {
// TODO: Move this to an eager-loading pathfinder derived class
// for (auto fromEl : network.Elements()) {
//   auto from = fromEl->PrimaryLocation();
//   for (auto toEl : network.Elements()) {
//     auto to = toEl->PrimaryLocation();
//     auto path = Solve(network, from, to);
//     if (path.size() > 0)
//       shortestPathFromLocToLoc[from][to] = path;
//   }
// }
// }

// TODO: Move this to an eager-loading pathfinder derived class
// const std::vector<Location>& Pathfinder::ShortestPath(const Location& from, const Location& to) {
//   if (this->shortestPathFromLocToLoc.find(from) == this->shortestPathFromLocToLoc.end()) {
//     spdlog::trace("No element at 'from' location\n");
//     throw "No element at 'from' location\n";
//   }
//   if (this->shortestPathFromLocToLoc[from].find(to) == this->shortestPathFromLocToLoc[from].end()) {
//     spdlog::trace("No element at 'to' location\n");
//     throw "No element at 'to' location\n";
//   }
//   return shortestPathFromLocToLoc[from][to];
// }

typedef std::pair<int, Location> WeightedLocation;
struct WeightedLocCmp {
  int operator()(const WeightedLocation& a, const WeightedLocation& b) {
    if (a.first == b.first) {
      if (a.second.first == b.second.first) {
        return a.second.second > b.second.second;
      }
      return a.second.first > b.second.first;
    }
    return a.first > b.first;
  }
};

double Pathfinder::heuristic(Location start, Location end) {
  return manhattanDistance(start, end);
}

double Pathfinder::actualCost(const Network& network, Location neighbor) {
  return 1;
}

std::vector<Location> Pathfinder::neighbors(const Network& network, Location current) {
  std::vector<Location> n = {};
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      if (i == 0 && j == 0)
        continue;
      // FIXME: This is absolutely incorrect for any other structure than a minimum sized one
      Location p = current + STRUCTURE_BASE_SIZE_UNIT * Location(i, j);
      n.push_back(p);
    }
  }
  return n;
}

std::vector<Location> Pathfinder::retrace(Location start, Location end, std::unordered_map<Location, Location, pair_hash> connections) {
  std::vector<Location> path = {end};
  auto& current = end;
  // for (auto &[a, b] : connections) {
  //     std::cout << "C: " << a << " -> " << b << "\n";
  // }
  while (current != start) {
    // std::cout << current << "\n";
    path.push_back(current = connections.at(current));
  }
  std::reverse(path.begin(), path.end());
  return path;
}

bool Pathfinder::isValidNeighborToTraverse(const Network& network, Location current, Location neighbor, Location start, Location end) {
  return true;
}

// Returns path between start and end, inclusive at both ends
std::vector<Location> Pathfinder::Solve(const Network& network, Location start, Location end) {
  // std::cout << "Solving: " << start << " to " << end << "\n";
  std::unordered_map<Location, int, pair_hash> guesses = {{start, heuristic(start, end)}};
  std::unordered_map<Location, int, pair_hash> scores = {{start, 0}};
  std::unordered_map<Location, Location, pair_hash> connections;
  std::priority_queue<WeightedLocation, std::vector<WeightedLocation>, WeightedLocCmp> frontier;
  frontier.push(WeightedLocation(guesses[start], start));

  std::unordered_map<int, int> radiusToTilesCheckedMap;
  // We count the end as already having been checked for the boundary condition below
  radiusToTilesCheckedMap[0] = 1;

  int iters = 0;
  while (++iters < MAX_ITERATIONS && !frontier.empty()) {
    auto weightLocPair = frontier.top();
    auto weight = weightLocPair.first;
    auto loc = weightLocPair.second;
    frontier.pop();
    // if (loc == end) std::cout << "Num checked: " << iters << " " << weight << "\n";
    // std::cout << "Num checked: " << iters << " " << weight << " " << loc << "\n";
    if (loc == end)
      return retrace(start, end, connections);
    for (auto neighbor : neighbors(network, loc)) {
      if (!isValidNeighborToTraverse(network, loc, neighbor, start, end))
        continue;

      if (neighbor == end) {
        connections.emplace(neighbor, loc);
        return retrace(start, end, connections);
      }

      // If for two adjacent radii, every tile at those radii around the end has already been checked once, then the end cannot be reached
      int radius = manhattanDistance(neighbor, end) / STRUCTURE_BASE_SIZE_UNIT; // Neighbor != end, radius > 0
      auto isChecked = [&radiusToTilesCheckedMap](int radius) { return radiusToTilesCheckedMap[radius] >= radius * 4; };
      if (isChecked(radius) && (isChecked(radius - 1) || isChecked(radius + 1))) {
        return {};
      }
      ++(radiusToTilesCheckedMap[radius]);

      auto nextscore = scores[loc] + actualCost(network, neighbor);
      if (scores.find(neighbor) != scores.end() && scores.at(neighbor) <= nextscore)
        continue;
      connections.emplace(neighbor, loc);
      scores[neighbor] = nextscore;
      guesses.emplace(neighbor, nextscore + heuristic(neighbor, end));
      // Determine if it is feasible to remove all previous pushed neighbor entries in the frontier
      auto next = WeightedLocation(heuristic(neighbor, end), neighbor);
      frontier.push(next);
      // std::cout << "Pushing: " << next.first << " " << next.second << " Weight: " << nextscore << " Size: " << frontier.size() << "\n";
    }
  }

  if (iters == MAX_ITERATIONS) {
    spdlog::trace("Pathfinder::Solve did not converge");
    spdlog::trace("{} to {}", to_string(start), to_string(end));
    for (auto el : network.Elements()) {
      spdlog::trace("Structure {}", to_string(el->PrimaryLocation()));
    }
    throw "Pathfinder::Solve did not converge";
  }

  // std::cout << "Failed to solve: " << start << " to " << end << "\n";
  return {};
}

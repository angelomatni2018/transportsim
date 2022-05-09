#ifndef PATHFINDER
#define PATHFINDER

#include <iostream>
#include <math.h>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <queue>

#include "base/spatial.h"
#include "base/network.h"

namespace world
{

    class Pathfinder
    {
    private:
        std::unordered_map<Location, std::unordered_map<Location, std::vector<Location>, pair_hash>, pair_hash> shortestPathFromLocToLoc;

    public:
        Pathfinder(const Network &network);

        const std::vector<Location> &ShortestPath(const Location &from, const Location &to);

        virtual double heuristic(Location start, Location end);

        virtual double actualCost(const Network &network, Location neighbor);

        virtual bool isValidNeighborToTraverse(const Network &network, Location neighbor);

        std::vector<Location> neighbors(const Network &network, Location current);

        std::vector<Location> retrace(Location start, Location end, std::unordered_map<Location, Location, pair_hash> connections);

        std::vector<Location> solve(const Network &network, Location start, Location end);
    };

}

#endif
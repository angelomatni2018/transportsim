#include "base/spatial.h"

using namespace world;

int world::manhattanDistance(const Location &loc1, const Location &loc2) { return abs(loc1.first - loc2.first) + abs(loc1.second - loc2.second); }

Direction world::getDirectionBetweenLocations(Location currentLocation, Location nextLocation)
{
    int we_diff = nextLocation.first - currentLocation.first;
    int sn_diff = nextLocation.second - currentLocation.second;
    switch (we_diff)
    {
    case -1:
        switch (sn_diff)
        {
        case 1:
            return Direction::NORTHWEST;
        case -1:
            return Direction::SOUTHWEST;
        case 0:
            return Direction::WEST;
        default:
            return Direction::NO_DIR;
        }
    case 0:
        switch (sn_diff)
        {
        case 1:
            return Direction::NORTH;
        case -1:
            return Direction::SOUTH;
        default:
            return Direction::NO_DIR;
        }
    case 1:
        switch (sn_diff)
        {
        case 1:
            return Direction::NORTHEAST;
        case -1:
            return Direction::SOUTHEAST;
        case 0:
            return Direction::EAST;
        default:
            return Direction::NO_DIR;
        }
    default:
        return Direction::NO_DIR;
    }
}

bool world::equalWrtEpsilon(double a, double b) {
    return std::abs(a - b) < EPSILON;
}

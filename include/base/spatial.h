#ifndef SPATIAL
#define SPATIAL

#include <iostream>
#include <math.h>
#include <vector>
#include <unordered_map>

namespace world
{

    struct pair_hash
    {
        template <typename T1, typename T2>
        std::size_t operator()(const std::pair<T1, T2> &pair) const
        {
            return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
        }

        template <class T2>
        std::size_t operator()(const std::pair<std::pair<int, int>, T2> &pair) const
        {
            // TODO: Use cantor pairing function
            return std::hash<int>()(pair.first.first) ^ std::hash<int>()(pair.first.second) ^ std::hash<T2>()(pair.second);
        }
    };

    template <typename T, typename U>
    std::pair<T, U> operator+(const std::pair<T, U> &l, const std::pair<T, U> &r)
    {
        return {l.first + r.first, l.second + r.second};
    }

    template <typename T, typename U>
    std::pair<T, U> operator-(const std::pair<T, U> &l, const std::pair<T, U> &r)
    {
        return {l.first - r.first, l.second - r.second};
    }

    template <typename T, typename U>
    std::ostream &operator<<(std::ostream &strm, const std::pair<T, U> &pair)
    {
        strm << pair.first << "," << pair.second;
        return strm;
    }

    enum BiDirectionality
    {
        NO_BIDIR = 0,
        N_S = 1,
        W_E = 2,
        SW_NE = 4,
        NW_SE = 8
    };

    enum Direction
    {
        NO_DIR = 0,
        NORTH,
        SOUTH,
        WEST,
        EAST,
        NORTHWEST,
        NORTHEAST,
        SOUTHWEST,
        SOUTHEAST
    };

    typedef std::pair<int, int> Location;
    typedef std::pair<Location, Direction> LocDir;

    int manhattanDistance(const Location &loc1, const Location &loc2);
    Direction getDirectionBetweenLocations(Location currentLocation, Location nextLocation);

    constexpr std::pair<int, int> NO_LOCATION = std::make_pair(INT32_MAX, INT32_MAX);

    constexpr double EPSILON = 0.000001;
    bool equalWrtEpsilon(double a, double b);

} // end namespace world

#endif

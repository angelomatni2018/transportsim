#ifndef SPATIAL
#define SPATIAL

#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <sstream>
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

    template <typename T, typename U, typename V>
    std::pair<T, U> operator*(const V scalar, const std::pair<T, U> &l)
    {
        return {scalar * l.first, scalar * l.second};
    }

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
    std::ostream &operator<<(std::ostream &strm, const std::pair<T, U> &pair) {
        strm << pair.first << "," << pair.second;
        return strm;
    }

    template <typename T, typename U>
    std::ostream &operator<<(std::ostream &strm, std::pair<T, U> &pair) {
        strm << pair.first << "," << pair.second;
        return strm;
    }

    template <typename T, typename U>
    std::string to_string(const std::pair<T, U>& v) {
        std::stringstream strm; strm << v; return strm.str();
    }

    typedef std::pair<int, int> Location;

    int manhattanDistance(const Location &loc1, const Location &loc2);

    constexpr std::pair<int, int> NO_LOCATION = std::make_pair(INT32_MAX, INT32_MAX);

    constexpr double EPSILON = 0.000001;
    bool equalWrtEpsilon(double a, double b);

} // end namespace world

#endif

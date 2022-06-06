#ifndef SPATIAL
#define SPATIAL

#include <iostream>
#include <math.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/utilities.h"

namespace world {

struct pair_hash {
  template <typename T1, typename T2>
  std::size_t operator()(const std::pair<T1, T2>& pair) const {
    return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
  }

  template <class T2>
  std::size_t operator()(const std::pair<std::pair<int, int>, T2>& pair) const {
    // TODO: Use cantor pairing function
    return std::hash<int>()(pair.first.first) ^ std::hash<int>()(pair.first.second) ^ std::hash<T2>()(pair.second);
  }
};

template <typename T, typename U, typename V>
std::pair<T, U> operator*(const V scalar, const std::pair<T, U>& l) {
  return {scalar * l.first, scalar * l.second};
}

template <typename T, typename U>
std::pair<T, U> operator+(const std::pair<T, U>& l, const std::pair<T, U>& r) {
  return {l.first + r.first, l.second + r.second};
}

template <typename T, typename U>
std::pair<T, U> operator-(const std::pair<T, U>& l, const std::pair<T, U>& r) {
  return {l.first - r.first, l.second - r.second};
}

template <typename T, typename U, typename V>
std::pair<T, U> operator/(const std::pair<T, U>& l, const V scalar) {
  return {l.first / scalar, l.second / scalar};
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& strm, const std::pair<T, U>& pair) {
  strm << pair.first << "," << pair.second;
  return strm;
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& strm, std::pair<T, U>& pair) {
  strm << pair.first << "," << pair.second;
  return strm;
}

template <typename T, typename U>
std::string to_string(const std::pair<T, U>& v) {
  std::stringstream strm;
  strm << v;
  return strm.str();
}

typedef std::pair<int, int> Location;
typedef std::pair<float, float> Point;
// A delta/vector between two locations:
typedef std::pair<int, int> Heading;

enum Direction { North = 1, NorthEast = 2, East = 4, SouthEast = 8, South = 16, SouthWest = 32, West = 64, NorthWest = 128 };
static constexpr uint8_t ALL_DIRECTIONS = 255U;

template <typename T>
T manhattanDistance(const std::pair<T, T>& pair1, const std::pair<T, T>& pair2) {
  return abs(pair1.first - pair2.first) + abs(pair1.second - pair2.second);
}

template <typename T>
T euclidianDistance(const std::pair<T, T>& pair1, const std::pair<T, T>& pair2) {
  auto x = (pair1.first - pair2.first);
  auto y = (pair1.second - pair2.second);
  return sqrt(x * x + y * y);
}

constexpr std::pair<int, int> NO_LOCATION = std::make_pair(INT32_MAX, INT32_MAX);

constexpr float FLOAT_EPSILON = 0.0001f;
bool equalish(float a, float b);
bool ltEqualish(float a, float b);
bool gtEqualish(float a, float b);

constexpr double EPSILON = 0.000001;
bool equalish(double a, double b);
bool ltEqualish(double a, double b);
bool gtEqualish(double a, double b);

template <typename T>
bool equalish(std::pair<T, T> a, std::pair<T, T> b) {
  return equalish(a.first, b.first) && equalish(a.second, b.second);
}

} // end namespace world

#endif

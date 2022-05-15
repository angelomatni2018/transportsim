#include "base/spatial.h"

using namespace world;

int world::manhattanDistance(const Location& loc1, const Location& loc2) {
  return abs(loc1.first - loc2.first) + abs(loc1.second - loc2.second);
}

bool world::equalish(double a, double b) {
  return std::abs(a - b) < EPSILON;
}

bool world::ltEqualish(double a, double b) {
  return a < b || equalish(a, b);
}

bool world::gtEqualish(double a, double b) {
  return a > b || equalish(a, b);
}

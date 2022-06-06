#include "base/spatial.h"

using namespace world;

bool world::equalish(float a, float b) {
  return std::abs(a - b) < FLOAT_EPSILON;
}

bool world::ltEqualish(float a, float b) {
  return a < b || equalish(a, b);
}

bool world::gtEqualish(float a, float b) {
  return a > b || equalish(a, b);
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

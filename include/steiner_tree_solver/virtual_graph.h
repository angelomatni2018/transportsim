#ifndef VIRTUAL_NETWORK
#define VIRTUAL_NETWORK

#include <unordered_map>
#include <unordered_set>

#include "network/graph.h"

namespace world {

class VirtualWorldElement : public WorldElement {
private:
  const Point* exactPosition;

public:
  VirtualWorldElement(const Point* exactPosition) : WorldElement{{0, 0}}, exactPosition{exactPosition} {};

  const Point* ExactPosition() const { return exactPosition; };
  virtual const std::vector<Location> AllOccupiedLocations() const { return {primaryLocation}; };

  inline const static std::string Type = "VirtualWorldElement";
  virtual std::string GetType() const { return Type; };
  virtual bool IsType(const std::string& Ty) const { return Type == VirtualWorldElement::Type; }
};

class VirtualGraph : public NetworkGraph {
public:
  void Add(const Point* one, const Point* other) { AddUndirected(get(one), get(other), euclidianDistance(*one, *other)); }

private:
  std::unordered_map<const Point*, std::unique_ptr<VirtualWorldElement>> pointToElement;

  VirtualWorldElement* get(const Point* point) {
    if (pointToElement.find(point) != pointToElement.end()) {
      return pointToElement.at(point).get();
    }
    pointToElement[point] = std::make_unique<VirtualWorldElement>(point);
    return pointToElement[point].get();
  }
};

} // namespace world

#endif
#ifndef WORLD_ELEMENTS
#define WORLD_ELEMENTS

#include <iostream>
#include <math.h>
#include <unordered_map>
#include <vector>

#include "base/spatial.h"

namespace world {

// Every tile is subdivided into 16 unique locations (4x4) that vehicles can fall on.
// The alternative would have been to use decimals for vehicle positioning, which is unwarranted complexity
constexpr int STRUCTURE_BASE_SIZE_UNIT = 4;
// This magic number of 4 is NOT TO BE CHANGED. It is fundamental to the pathing math used
static_assert(STRUCTURE_BASE_SIZE_UNIT == 4);

class Network;

class WorldElement {
protected:
  // The primary location is some arbitrary point from which the rest of the occupied coordinates
  // of this world element are represented.
  // Other coords are represented either as offsets or by a shape's dimensions
  // This choice of representation is decided by each derived class and determines the implementation of getAllOccupiedLocations
  Location primaryLocation;

public:
  WorldElement(Location primaryLoc);

  const Location& PrimaryLocation() const;
  virtual const std::vector<Location> AllOccupiedLocations() const = 0;

  virtual std::string GetType() const = 0;
  virtual bool IsType(const std::string& Ty) const { return false; }

  friend class Network;
};

class SquareWorldElement : public WorldElement {
protected:
  // Convention is that the location of the building is the bottom left
  // and the size indicates how many units to the right and above that the building covers
  int sizeX, sizeY;

public:
  SquareWorldElement(std::pair<int, int> size, Location primaryLoc);

  const std::vector<Location> AllOccupiedLocations() const override;
};

class CoordOffsetWorldElement : public WorldElement {
protected:
  std::vector<Location> offsetsFromPrimaryLocation;

public:
  CoordOffsetWorldElement(std::vector<Location> initialOffsets, Location primaryLoc);

  const std::vector<Location> AllOccupiedLocations() const override;
};

class Building : public SquareWorldElement {
private:
public:
  Building(std::pair<int, int> size, Location primaryLoc);

  virtual int CurrentOccupancy();
  virtual int OccupancyCapacity();

  virtual void AddOccupant(){};
  virtual void RemoveOccupant(){};

  inline const static std::string Type = "Building";
  std::string GetType() const override { return Building::Type; }
  bool IsType(const std::string& Ty) const override { return Ty == Building::Type || SquareWorldElement::IsType(Ty); }

  friend class Network;
};

class CommercialBuilding : public Building {
private:
  int numVisitors, currentVisitors;

public:
  CommercialBuilding(int numOccupants, std::pair<int, int> size, Location loc);

  int CurrentOccupancy() override;
  int OccupancyCapacity() override;

  void AddOccupant() override;
  void RemoveOccupant() override;

  inline const static std::string Type = "CommercialBuilding";
  std::string GetType() const override { return CommercialBuilding::Type; }
  bool IsType(const std::string& Ty) const override { return Ty == CommercialBuilding::Type || Building::IsType(Ty); }

  friend class Network;
};

class ResidentialBuilding : public Building {
private:
  int numResidents, currentResidents;

public:
  ResidentialBuilding(int numOccupants, std::pair<int, int> size, Location loc);

  int CurrentOccupancy() override;
  int OccupancyCapacity() override;

  void AddOccupant() override;
  void RemoveOccupant() override;

  inline const static std::string Type = "ResidentialBuilding";
  std::string GetType() const override { return ResidentialBuilding::Type; }
  bool IsType(const std::string& Ty) const override { return Ty == ResidentialBuilding::Type || Building::IsType(Ty); }

  friend class Network;
};

class Roadway : public SquareWorldElement {
protected:
  uint8_t directionsMask = ALL_DIRECTIONS;

public:
  Roadway(std::pair<int, int> size, Location loc);

  inline const static std::string Type = "Roadway";
  std::string GetType() const override { return Roadway::Type; }
  bool IsType(const std::string& Ty) const override { return Ty == Roadway::Type || SquareWorldElement::IsType(Ty); }

  std::vector<Location> LocationsThrough(Location from, Location to) {
    auto locations = offsetsThrough(from, to);
    for (int i = 0; i < locations.size(); ++i)
      locations[i] = primaryLocation + locations[i];
    return locations;
  }

  virtual std::vector<Location> offsetsThrough(Location from, Location to) = 0;

  uint8_t DirectionTowardsLocation(Location targetLocation) const;
  static uint8_t DirectionWhenHeading(Heading heading);
  uint8_t DirectionsMask() const { return directionsMask; }
  void SetDirections(uint8_t mask) { directionsMask = mask; }
  static bool CanDirectionsConnect(uint8_t incomingMask, uint8_t outgoingMask, Heading heading);
  bool CanDirectionsConnect(Roadway* next) const;

  friend class Network;
};

// To be used for a roadway without traffic control
// Future derived classes of Roadway would potentially include:
// TrafficSignalIntersection and RoundaboutIntersection
class RoadSegment : public Roadway {
public:
  RoadSegment(Location loc);

  inline const static std::string Type = "RoadSegment";
  std::string GetType() const override { return RoadSegment::Type; }
  bool IsType(const std::string& Ty) const override { return Ty == RoadSegment::Type || Roadway::IsType(Ty); }

  std::vector<Location> offsetsThrough(Location from, Location to) override;

  friend class Network;
};

class Vehicle : public CoordOffsetWorldElement {
private:
  ResidentialBuilding* home;

public:
  Vehicle(ResidentialBuilding* home, std::vector<Location> initialOffsets, Location primaryLoc);

  inline const static std::string Type = "Vehicle";
  std::string GetType() const override { return Vehicle::Type; }
  bool IsType(const std::string& Ty) const override { return Ty == Vehicle::Type || CoordOffsetWorldElement::IsType(Ty); }
};

} // namespace world

#endif
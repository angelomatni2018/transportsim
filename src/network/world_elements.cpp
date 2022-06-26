#include "network/world_elements.h"
#include "spdlog/spdlog.h"

using namespace world;

WorldElement::WorldElement(Location primaryLoc) : primaryLocation{primaryLoc} {}
const Location& WorldElement::PrimaryLocation() const {
  return this->primaryLocation;
}

SquareWorldElement::SquareWorldElement(std::pair<int, int> size, Location loc) : sizeX{size.first}, sizeY{size.second}, WorldElement{loc} {}

const std::vector<Location> SquareWorldElement::AllOccupiedLocations() const {
  std::vector<Location> locations;
  for (int x = 0; x < sizeX; ++x) {
    for (int y = 0; y < sizeY; ++y) {
      locations.push_back(primaryLocation + std::make_pair(x, y));
    }
  }
  return locations;
}

CoordOffsetWorldElement::CoordOffsetWorldElement(std::vector<Location> initialOffsets, Location primaryLoc)
    : offsetsFromPrimaryLocation{initialOffsets}, WorldElement{primaryLoc} {}

const std::vector<Location> CoordOffsetWorldElement::AllOccupiedLocations() const {
  std::vector<Location> locations = {primaryLocation};
  for (auto offset : offsetsFromPrimaryLocation) {
    locations.push_back(primaryLocation + offset);
  }
  return locations;
}

Building::Building(std::pair<int, int> size, Location loc) : SquareWorldElement{size, loc} {
  if (sizeX % STRUCTURE_BASE_SIZE_UNIT != 0 || sizeY % STRUCTURE_BASE_SIZE_UNIT != 0) {
    spdlog::trace("Structure (of type {}) with size {} is not a multiple of STRUCTURE_BASE_SIZE_UNIT={}", this->GetType(), to_string(size),
                  STRUCTURE_BASE_SIZE_UNIT);
    throw "Building structure misaligned";
  }
}

int Building::CurrentOccupancy() {
  return 0;
}

int Building::OccupancyCapacity() {
  return 0;
}

CommercialBuilding::CommercialBuilding(int numOccupants, std::pair<int, int> size, Location loc)
    : numVisitors{numOccupants}, currentVisitors{0}, Building{size, loc} {}

void CommercialBuilding::AddOccupant() {
  currentVisitors++;
}

void CommercialBuilding::RemoveOccupant() {
  currentVisitors--;
}

int CommercialBuilding::CurrentOccupancy() {
  return currentVisitors;
}

int CommercialBuilding::OccupancyCapacity() {
  return numVisitors;
}

ResidentialBuilding::ResidentialBuilding(int numOccupants, std::pair<int, int> size, Location loc)
    : numResidents{numOccupants}, currentResidents{0}, Building{size, loc} {}

void ResidentialBuilding::AddOccupant() {
  currentResidents++;
}

void ResidentialBuilding::RemoveOccupant() {
  currentResidents--;
};

int ResidentialBuilding::CurrentOccupancy() {
  return currentResidents;
}

int ResidentialBuilding::OccupancyCapacity() {
  return numResidents;
}

Roadway::Roadway(std::pair<int, int> size, Location loc) : SquareWorldElement{size, loc} {}

std::string Roadway::MaskToString(uint8_t mask) {
  std::string s;
  s = s + (mask & 1 ? "1," : "") + (mask & 2 ? "2," : "") + (mask & 4 ? "4," : "") + (mask & 8 ? "8," : "") + (mask & 16 ? "16," : "") +
      (mask & 32 ? "32," : "") + (mask & 64 ? "64," : "") + (mask & 128 ? "128," : "");
  s.erase(s.end() - 1);
  return s;
}

uint8_t Roadway::DirectionTowardsLocation(Location targetLocation) const {
  return DirectionWhenHeading(targetLocation - this->primaryLocation);
}

uint8_t Roadway::DirectionWhenHeading(Heading heading) {
  auto& [x, y] = heading;
  auto normX = x / std::max(1, abs(x));
  auto normY = y / std::max(1, abs(y));
  switch (normX) {
  case -1:
    switch (normY) {
    case -1:
      return Direction::SouthWest;
    case 0:
      return Direction::West;
    case 1:
      return Direction::NorthWest;
    }
  case 0:
    switch (normY) {
    case -1:
      return Direction::South;
    case 1:
      return Direction::North;
    }
  case 1:
    switch (normY) {
    case -1:
      return Direction::SouthEast;
    case 0:
      return Direction::East;
    case 1:
      return Direction::NorthEast;
    }
  }
  return 0;
}

bool Roadway::CanDirectionsConnect(Roadway* next) const {
  return Roadway::CanDirectionsConnect(this->DirectionsMask(), next->DirectionsMask(), next->primaryLocation - this->primaryLocation);
}

bool Roadway::CanDirectionsConnect(uint8_t incomingMask, uint8_t outgoingMask, Heading heading) {
  // Rotating by 45 degrees clockwise from North -> NorthEast -> East -> ... -> West -> NorthWest
  // is done by left shifting one bit. From NorthWest -> North requires the left shifting to be circular bit shifting.
  // To reverse a direction (180 degrees = 45 * 4), we do a circular bit shift of 4 bits.
  auto circularLsh = [](uint8_t mask, int shift) -> uint8_t { return (mask << shift) | (mask >> (8 - shift)); };
  auto reverseDirections = [circularLsh](uint8_t mask) -> uint8_t { return circularLsh(mask, 4); };

  auto revOutMask = reverseDirections(outgoingMask);
  auto headingMask = DirectionWhenHeading(heading);
  // spdlog::trace("Incoming {}    Outgoing {}    RevOut {}    Heading {}", incomingMask, outgoingMask, revOutMask, headingMask);
  // spdlog::trace("Incoming {}    Outgoing {}    RevOut {}    Heading {}", MaskToString(incomingMask), MaskToString(outgoingMask),
  // MaskToString(revOutMask), MaskToString(headingMask));
  if (headingMask == 0) {
    spdlog::debug("Roadway::CanDirectionsConnect invalid heading provided: {}", to_string(heading));
    return false;
  }

  // spdlog::debug("heading mask: {} incomingMask: {} revOutMask: {}", headingMask, incomingMask, revOutMask);

  // We reverse the outgoing mask because the direction masks express which way(s) you can EXIT a segment.
  // If you can exit the outgoing segment (travel SouthWest from the center of it),
  // then heading NorthWest is a valid way to ENTER the outgoing segment.
  if ((incomingMask & headingMask) != 0 && (revOutMask & headingMask) != 0) {
    return true;
  }

  // There is one caveat; if you exit in a diagonal direction,
  // you can enter an adjacent segment from the corner shared by the segment you just exited
  // Imagine a zig zag from the center of a segment to a corner and then from that corner to the 2nd segment's center.
  auto isCardinal = [](uint8_t mask) { return mask & (Direction::North | Direction::South | Direction::West | Direction::East); };
  auto isZigZagOrZagZig = [circularLsh](uint8_t mask1, uint8_t mask2, uint8_t headingDirection) {
    auto headingZig = circularLsh(headingDirection, 1);
    auto headingZag = circularLsh(headingDirection, 7);
    // spdlog::debug("heading zig: {} heading zag: {}", headingZig, headingZag);
    return ((mask1 & headingZig) && (mask2 & headingZag)) || ((mask1 & headingZag) && (mask2 & headingZig));
  };
  return isCardinal(headingMask) && isZigZagOrZagZig(incomingMask, revOutMask, headingMask);
}

Vehicle::Vehicle(ResidentialBuilding* home, std::vector<Location> initialOffsets, Location primaryLoc)
    : home{home}, CoordOffsetWorldElement{initialOffsets, primaryLoc} {}

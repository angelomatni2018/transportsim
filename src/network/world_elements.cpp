#include "network/world_elements.h"
#include "spdlog/spdlog.h"

using namespace world;

WorldElement::WorldElement(Location primaryLoc) : primaryLocation{primaryLoc} {}
const Location &WorldElement::PrimaryLocation() const { return this->primaryLocation; }

SquareWorldElement::SquareWorldElement(std::pair<int, int> size, Location loc)
    : sizeX{size.first}, sizeY{size.second}, WorldElement{loc} {}

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
        spdlog::error("Structure (of type {}) with size {} is not a multiple of STRUCTURE_BASE_SIZE_UNIT={}",
            this->GetType(), to_string(size), STRUCTURE_BASE_SIZE_UNIT);
        abort();
    }
}

int Building::CurrentOccupancy() { return 0; }

int Building::OccupancyCapacity() { return 0; }

CommercialBuilding::CommercialBuilding(int numOccupants, std::pair<int, int> size, Location loc)
    : numVisitors{numOccupants}, currentVisitors{0}, Building{size, loc} {}

void CommercialBuilding::AddOccupant() { currentVisitors++; }

void CommercialBuilding::RemoveOccupant() { currentVisitors--; }

int CommercialBuilding::CurrentOccupancy() { return currentVisitors; }

int CommercialBuilding::OccupancyCapacity() { return numVisitors; }

ResidentialBuilding::ResidentialBuilding(int numOccupants, std::pair<int, int> size, Location loc)
    : numResidents{numOccupants}, currentResidents{0}, Building{size, loc} {}

void ResidentialBuilding::AddOccupant() { currentResidents++; }

void ResidentialBuilding::RemoveOccupant() { currentResidents--; };

int ResidentialBuilding::CurrentOccupancy() { return currentResidents; }

int ResidentialBuilding::OccupancyCapacity() { return numResidents; }

Roadway::Roadway(std::pair<int, int> size, Location loc) : SquareWorldElement{size, loc} {}

Vehicle::Vehicle(ResidentialBuilding *home, std::vector<Location> initialOffsets, Location primaryLoc)
    : home{home}, CoordOffsetWorldElement{initialOffsets, primaryLoc} {}

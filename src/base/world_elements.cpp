#include "base/world_elements.h"

using namespace world;

WorldElement::WorldElement(Location loc) : location{loc} {};
const Location &WorldElement::getLocation() const { return this->location; }
std::string WorldElement::getType() const { return WorldElement::Type; }

Building::Building(std::pair<int, int> size, Location loc)
    : sizeX{size.first}, sizeY{size.second}, WorldElement{loc} {};
int Building::getCurrentOccupancy() { return 0; }
int Building::getOccupancyCapacity() { return 0; }
std::string Building::getType() const { return Building::Type; }

CommercialBuilding::CommercialBuilding(int numOccupants, std::pair<int, int> size, Location loc)
    : numVisitors{numOccupants}, currentVisitors{0}, Building{size, loc} {}
void CommercialBuilding::addOccupant() { currentVisitors++; }
void CommercialBuilding::removeOccupant() { currentVisitors--; }
int CommercialBuilding::getCurrentOccupancy() { return currentVisitors; }
int CommercialBuilding::getOccupancyCapacity() { return numVisitors; }
std::string CommercialBuilding::getType() const { return CommercialBuilding::Type; }

ResidentialBuilding::ResidentialBuilding(int numOccupants, std::pair<int, int> size, Location loc)
    : numResidents{numOccupants}, currentResidents{numOccupants}, Building{size, loc} {}
void ResidentialBuilding::addOccupant() { currentResidents++; }
void ResidentialBuilding::removeOccupant() { currentResidents--; };
int ResidentialBuilding::getCurrentOccupancy() { return currentResidents; }
int ResidentialBuilding::getOccupancyCapacity() { return numResidents; }
std::string ResidentialBuilding::getType() const { return ResidentialBuilding::Type; }

Roadway::Roadway(Location loc) : WorldElement{loc} {}
std::string Roadway::getType() const { return Roadway::Type; }

bool RoadJunction::isValidDirectionality(BiDirectionality directionality)
{
    return (this->directionalities & directionality);
}

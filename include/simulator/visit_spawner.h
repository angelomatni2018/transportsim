#ifndef SPAWNER
#define SPAWNER

#include "base/network.h"
#include "base/nearest_building_iterator.h"

#include <unordered_map>
#include <vector>
#include <queue>

using namespace world;

class VisitSpawner {
private:
    std::unordered_map<CommercialBuilding *, std::vector<ResidentialBuilding *>> buildingToAssigneesMap;
    std::queue<CommercialBuilding *> spawnQueue;
    double spawnInterval;
    double rollingTimeElapsed;

public:
    VisitSpawner(double spawnInterval);

    // TODO: Have the visit spawner choose which residential building to assign to a commercial building by the distance
    // along roads between the two buildings, not a manhattan distance between the two building's coordinates
    std::vector<std::pair<CommercialBuilding *, ResidentialBuilding *>> spawn(const Network &network, double timeElapsed);

};

#endif

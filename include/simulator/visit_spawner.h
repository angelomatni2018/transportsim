#ifndef SPAWNER
#define SPAWNER

#include "simulator/imports.h"

#include <unordered_map>
#include <vector>
#include <queue>

using namespace world;

class VisitSpawner {
private:

    // Include variable spawn rates for different commercial buildings
    struct CommercialSpawnInfo {
        int spawnBacklogCount = 0;
    };

    struct ResidentialSpawnInfo {
        int insufficientSupplyOccurenceCount = 0;
    };

    std::unordered_map<CommercialBuilding *, CommercialSpawnInfo> commSpawnInfo;
    std::unordered_map<ResidentialBuilding *, ResidentialSpawnInfo> resSpawnInfo;
    std::queue<CommercialBuilding *> spawnQueue;
    double spawnInterval;
    double rollingTimeElapsed;

public:
    VisitSpawner(double spawnInterval);

    // TODO: Have the visit spawner choose which residential building to assign to a commercial building by the distance
    // along roads between the two buildings, not a manhattan distance between the two building's coordinates
    std::vector<std::pair<CommercialBuilding *, ResidentialBuilding *>> Spawn(const Network &network, double timeElapsed);

    std::vector<std::pair<const CommercialBuilding *, int>> CommercialBuildingsWithBacklog() const;

    std::vector<const ResidentialBuilding *> MostDemandedResidentialBuildings() const ;

};

#endif

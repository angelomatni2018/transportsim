#include "simulator/visit_spawner.h"

VisitSpawner::VisitSpawner(double spawnInterval) : spawnInterval{spawnInterval}, rollingTimeElapsed{0} {}

std::vector<std::pair<CommercialBuilding *, ResidentialBuilding *>> VisitSpawner::Spawn(const Network &network, double timeElapsed) {
    // Ensure any newly created commercial buildings are tracked
    for (auto building : network.Buildings()) {
        if (auto comm = dynamic_cast<CommercialBuilding *>(building); comm != nullptr) {
            if (this->buildingToAssigneesMap.find(comm) == this->buildingToAssigneesMap.end()) {
                this->buildingToAssigneesMap.insert(std::pair(comm, std::vector<ResidentialBuilding *>()));
                this->spawnQueue.push(comm);
            }
        }
    }

    std::vector<std::pair<CommercialBuilding *, ResidentialBuilding *>> spawnedVisits;
    rollingTimeElapsed += timeElapsed;
    // TODO: Enforce a spawn every spawnInterval?
    while (rollingTimeElapsed >= spawnInterval && !this->spawnQueue.empty()) {
        rollingTimeElapsed -= spawnInterval;

        auto nextCommInLine = this->spawnQueue.front();

        // TODO: Enforce occupancy capacity? Or leave responsibility to caller?
        nextCommInLine->AddOccupant();
        NearestBuildingIterator iter(network, nextCommInLine);
        const Building *building;
        while (building = iter.Next(), building != nullptr) {
            if (auto res = dynamic_cast<ResidentialBuilding *>(const_cast<Building*>(building)); res != nullptr) {
                // TODO: Enforce occupancy capacity? Or leave responsibility to caller?
                res->AddOccupant();
                spawnedVisits.push_back(std::make_pair(nextCommInLine, res));
                this->buildingToAssigneesMap[nextCommInLine].push_back(res);

                // The spawn has been fulfilled, move to the back of the queue
                this->spawnQueue.pop();
                this->spawnQueue.push(nextCommInLine);
                break;
            }
        }
    }
    return spawnedVisits;
}
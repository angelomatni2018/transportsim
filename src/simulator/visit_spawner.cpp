#include "simulator/visit_spawner.h"
#include <functional>

VisitSpawner::VisitSpawner(double spawnInterval) : spawnInterval{spawnInterval}, rollingTimeElapsed{0} {}

std::vector<std::pair<CommercialBuilding*, ResidentialBuilding*>> VisitSpawner::Spawn(const Network& network, double timeElapsed) {
  for (auto building : network.Buildings()) {
    if (building->IsType(CommercialBuilding::Type)) {
      auto comm = static_cast<CommercialBuilding*>(building);
      if (this->commSpawnInfo.find(comm) == this->commSpawnInfo.end()) {
        this->commSpawnInfo.insert({comm, CommercialSpawnInfo()});
        this->spawnQueue.push(comm);
      }
    } else if (building->IsType(ResidentialBuilding::Type)) {
      auto res = static_cast<ResidentialBuilding*>(building);
      if (this->resSpawnInfo.find(res) == this->resSpawnInfo.end()) {
        this->resSpawnInfo.insert({res, ResidentialSpawnInfo()});
      }
    }
  }

  std::vector<std::pair<CommercialBuilding*, ResidentialBuilding*>> spawnedVisits;
  rollingTimeElapsed += timeElapsed;
  while (rollingTimeElapsed >= spawnInterval && !this->spawnQueue.empty()) {
    rollingTimeElapsed -= spawnInterval;

    for (int i = 0; i < this->commSpawnInfo.size(); ++i) {
      if (this->spawnQueue.empty())
        break;
      auto nextCommInLine = this->spawnQueue.front();

      bool canBeVisited = false;
      if (nextCommInLine->CurrentOccupancy() < nextCommInLine->OccupancyCapacity()) {
        NearestBuildingIterator iter(network, nextCommInLine);
        const Building* building;
        while (building = iter.Next(), building != nullptr) {
          if (!building->IsType(ResidentialBuilding::Type))
            continue;

          auto res = static_cast<ResidentialBuilding*>(const_cast<Building*>(building));
          if (res->CurrentOccupancy() == res->OccupancyCapacity()) {
            this->resSpawnInfo[res].insufficientSupplyOccurenceCount++;
            continue;
          }

          spawnedVisits.push_back(std::make_pair(nextCommInLine, res));
          nextCommInLine->AddOccupant();
          res->AddOccupant();
          canBeVisited = true;
          break;
        }
      }

      if (!canBeVisited) {
        this->commSpawnInfo[nextCommInLine].spawnBacklogCount++;
      }
      this->spawnQueue.pop();
      this->spawnQueue.push(nextCommInLine);
    }
  }
  return spawnedVisits;
}

std::vector<std::pair<const CommercialBuilding*, int>> VisitSpawner::CommercialBuildingsWithBacklog() const {
  std::vector<std::pair<const CommercialBuilding*, int>> backlogs;
  for (auto& [comm, spawnInfo] : this->commSpawnInfo) {
    if (spawnInfo.spawnBacklogCount == 0)
      continue;
    backlogs.push_back({comm, spawnInfo.spawnBacklogCount});
  }
  return backlogs;
}

using ResB = ResidentialBuilding;
std::vector<const ResB*> VisitSpawner::MostDemandedResidentialBuildings() const {
  auto demandCmp = [&](ResB* res1, ResB* res2) -> bool {
    return resSpawnInfo.at(res1).insufficientSupplyOccurenceCount < resSpawnInfo.at(res2).insufficientSupplyOccurenceCount;
  };
  std::priority_queue<ResB*, std::vector<ResB*>, std::function<bool(ResB*, ResB*)>> resQ(demandCmp);

  for (auto& [res, spawnInfo] : this->resSpawnInfo) {
    if (spawnInfo.insufficientSupplyOccurenceCount == 0)
      continue;
    resQ.push(res);
  }

  std::vector<const ResB*> mostDemanded;
  while (!resQ.empty()) {
    mostDemanded.push_back(resQ.top());
    resQ.pop();
  }
  return mostDemanded;
}

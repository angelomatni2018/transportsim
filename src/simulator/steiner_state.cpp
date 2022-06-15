#include "simulator/steiner_state.h"

using namespace world;

StateChange SteinerState::Simulate(const FrameData& frameData, const sf::RenderWindow& window, InputManager& inputManager) {
  StateChange stateChange;
  stateChange.paths = &this->paths;

  auto spawned = this->structureDrawer.TrackMouseToSpawn(frameData, window, inputManager, inputNetwork);
  if (spawned.size() > 0) {
    if (this->connectedNetwork != nullptr) {
      for (auto& [loc, el] : this->connectedNetwork->SpatialMap()) {
        stateChange.removes.push_back(el);
      }
    }
    this->connectedNetwork = SteinerPointSearchByClustering::ConnectStructures(inputNetwork);
    for (auto& [loc, el] : this->connectedNetwork->SpatialMap()) {
      stateChange.adds.push_back(el);
    }
  }

  // auto visits = this->spawner.Spawn(this->connectedNetwork, 1);
  // for (auto visit : visits) {
  //   auto comm = visit.first;
  //   auto res = visit.second;

  //   const int FRAMES_PER_TILE = 4;
  //   auto path = VehiclePathConstructor::Construct(this->connectedNetwork, nextPathId++, visit, frameData.frameNumber, FRAMES_PER_TILE);
  //   if (path != nullptr) {
  //     spdlog::trace("Path spawned: res at {} to comm at {} (path length {})", to_string(res->PrimaryLocation()),
  //     to_string(comm->PrimaryLocation()),
  //                   path->orderedPathEvents.size());
  //     this->paths.emplace(path);
  //   } else {
  //     spdlog::trace("No path spawned: res at {} to comm at {}", to_string(res->PrimaryLocation()), to_string(comm->PrimaryLocation()));
  //   }
  // }
  // if (visits.size() > 0) {
  //   PathReconciler().Reconcile(paths);
  // }

  return stateChange;
}
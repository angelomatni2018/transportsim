#include "simulator/simulation_state.h"

using namespace world;

void FrameData::NextFrame() {
  auto nextLastFrame = std::chrono::system_clock::now();
  this->rollingDuration += nextLastFrame - lastFrame;
  this->lastFrame = nextLastFrame;

  if (rollingDuration > FRAME_DURATION) {
    rollingDuration -= FRAME_DURATION;
    ++this->frameNumber;
  }
}

StateChange SimulationState::Simulate(const FrameData& frameData, const sf::RenderWindow& window, InputManager& inputManager) {
  StateChange stateChange;
  stateChange.paths = &this->paths;

  // if (network.Roads().size() == 0) {
  //   int starterRoadLength = 3;
  //   auto size1 = STRUCTURE_BASE_SIZE_UNIT * Location{1, 1};
  //   auto size32 = STRUCTURE_BASE_SIZE_UNIT * Location{3, 2};
  //   auto leftBuilding = this->network.Add(ResidentialBuilding(INT32_MAX, size1, STRUCTURE_BASE_SIZE_UNIT * Location{-1, 0}));
  //   auto rightBuilding = this->network.Add(CommercialBuilding(INT32_MAX, size32, STRUCTURE_BASE_SIZE_UNIT * Location{starterRoadLength, 0}));
  //   stateChange.adds.push_back(leftBuilding);
  //   stateChange.adds.push_back(rightBuilding);
  //   for (auto i = 0; i < starterRoadLength; ++i) {
  //     auto firstRoad = static_cast<Roadway*>(this->network.Add(RoadSegment(STRUCTURE_BASE_SIZE_UNIT * Location{i, 0})));
  //     firstRoad->SetDirections(Direction::West | Direction::East);
  //     stateChange.adds.push_back(firstRoad);
  //   }
  //   return stateChange;
  // }

  auto visits = this->spawner.Spawn(this->network, 1);
  for (auto visit : visits) {
    auto comm = visit.first;
    auto res = visit.second;

    const int FRAMES_PER_TILE = 4;
    auto path = this->paths.Add(Path());
    if (VehiclePathConstructor::Construct(path, this->network, nextPathId++, visit, frameData.frameNumber, FRAMES_PER_TILE)) {
      // spdlog::trace("Path spawned: res at {} to comm at {} (path length {})", to_string(res->PrimaryLocation()),
      // to_string(comm->PrimaryLocation()),
      //               path->orderedPathEvents->size());
    } else {
      spdlog::trace("No path spawned: res at {} to comm at {}", to_string(res->PrimaryLocation()), to_string(comm->PrimaryLocation()));
      this->paths.Remove(path);
    }
  }
  if (visits.size() > 0) {
    PathReconciler().Reconcile(paths.Get());
  }

  auto spawned = this->structureDrawer.TrackMouseToSpawn(frameData, window, inputManager, network);
  stateChange.adds.insert(stateChange.adds.end(), spawned.begin(), spawned.end());

  return stateChange;
}

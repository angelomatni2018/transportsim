#include "simulator/simulation_state.h"
#include "pathing/vehiclepathconstructor.h"
#include "simulator/simulation_config.h"

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

  if (network.Roads().size() == 0) {
    int starterRoadLength = 3;
    auto size1 = STRUCTURE_BASE_SIZE_UNIT * Location{1, 1};
    auto leftBuilding = new ResidentialBuilding(INT32_MAX, size1, STRUCTURE_BASE_SIZE_UNIT * Location{-1, 0});
    auto rightBuilding = new CommercialBuilding(INT32_MAX, size1, STRUCTURE_BASE_SIZE_UNIT * Location{starterRoadLength, 0});
    this->network.AddBuilding(leftBuilding);
    this->network.AddBuilding(rightBuilding);
    stateChange.elements.push_back(leftBuilding);
    stateChange.elements.push_back(rightBuilding);
    for (auto i = 0; i < starterRoadLength; ++i) {
      auto firstRoad = new RoadSegment(STRUCTURE_BASE_SIZE_UNIT * Location{i, 0});
      this->network.AddRoadway(firstRoad);
      stateChange.elements.push_back(firstRoad);
    }
    return stateChange;
  }

  auto visits = this->spawner.Spawn(this->network, 1);
  for (auto visit : visits) {
    auto comm = visit.first;
    auto res = visit.second;

    const int FRAMES_PER_TILE = 4;
    auto path = VehiclePathConstructor::Construct(this->network, nextPathId++, visit, frameData.frameNumber, FRAMES_PER_TILE);
    if (path != nullptr) {
      spdlog::trace("Path spawned: res at {} to comm at {} (path length {})", to_string(res->PrimaryLocation()), to_string(comm->PrimaryLocation()),
                    path->orderedPathEvents.size());
      this->paths.emplace(path);
    }
  }
  if (visits.size() > 0) {
    PathReconciler().Reconcile(paths);
  }

  auto windowSize = window.getSize();
  int windowWidth = windowSize.x;
  int windowHeight = windowSize.y;
  auto aspectRatio = 1.0 * windowWidth / windowHeight;
  if (inputManager.IsClick(sf::Mouse::Left)) {
    auto mousePos = sf::Mouse::getPosition(window);
    // HACK: We do 1 - y_fraction here so that 0.2 represents 20% from the bottom of the screen instead of 20% from the top
    auto mousePosFraction = sf::Vector2f(1.0 * mousePos.x / windowWidth, 1.0 - (1.0 * mousePos.y / windowHeight));
    auto mousePosCentered =
        sf::Vector2f((-GRID_CENTER + GRID_SIZE * mousePosFraction.x) * aspectRatio, -GRID_CENTER + (GRID_SIZE * mousePosFraction.y));
    auto squareLoc = VectorToLocation(sf::Vector2i(floor(mousePosCentered.x / SQUARE_RESIZE), floor(mousePosCentered.y / SQUARE_RESIZE)));
    squareLoc = STRUCTURE_BASE_SIZE_UNIT * squareLoc;
    spdlog::trace("Mouse: {} Square: {}", to_string(VectorToLocation(mousePos)), to_string(squareLoc));
    spdlog::trace("Window: {},{}", windowWidth, windowHeight);

    if (!this->network.HasStructureAt(squareLoc)) {
      WorldElement* spawnElem = nullptr;
      if (inputManager.IsHold(sf::Keyboard::Num1)) {
        spawnElem = new CommercialBuilding(INT32_MAX, std::make_pair(STRUCTURE_BASE_SIZE_UNIT, STRUCTURE_BASE_SIZE_UNIT), squareLoc);
        this->network.AddBuilding((Building*)spawnElem);
      } else if (inputManager.IsHold(sf::Keyboard::Num2)) {
        spawnElem = new ResidentialBuilding(INT32_MAX, std::make_pair(STRUCTURE_BASE_SIZE_UNIT, STRUCTURE_BASE_SIZE_UNIT), squareLoc);
        this->network.AddBuilding((Building*)spawnElem);
      } else {
        spawnElem = new RoadSegment(squareLoc);
        this->network.AddRoadway((Roadway*)spawnElem);
      }
      stateChange.elements.push_back(spawnElem);
    }
  }

  return stateChange;
}

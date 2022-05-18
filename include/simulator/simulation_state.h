#ifndef SIMULATION_STATE
#define SIMULATION_STATE

#include "simulator/frame_data.h"
#include "simulator/imports.h"
#include "simulator/sfml_helpers.h"
#include "simulator/structure_drawer.h"
#include "simulator/visit_spawner.h"

namespace world {

struct StateChange {
  std::vector<WorldElement*> elements;
  std::unordered_set<Path*>* paths;
};

struct SimulationState {
  Network network;
  VisitSpawner spawner;
  StructureDrawer structureDrawer;
  std::unordered_set<Path*> paths;
  int nextPathId = 0;

  SimulationState(double visitSpawnRate) : spawner{visitSpawnRate} {}
  StateChange Simulate(const FrameData& frameData, const sf::RenderWindow& window, InputManager& inputManager);
};

} // namespace world

#endif

#ifndef SIMULATION_STATE
#define SIMULATION_STATE

#include "pathing/vehiclepathconstructor.h"
#include "simulator/frame_data.h"
#include "simulator/imports.h"
#include "simulator/sfml_helpers.h"
#include "simulator/simulation_config.h"
#include "simulator/structure_drawer.h"
#include "simulator/visit_spawner.h"

namespace world {

struct StateChange {
  std::vector<WorldElement*> adds, removes;
  PtrSet<Path>* paths;
};

struct SimulationState {
  Network network;
  VisitSpawner spawner;
  StructureDrawer structureDrawer;
  PtrSet<Path> paths;
  int nextPathId = 0;

  SimulationState(double visitSpawnRate) : spawner{visitSpawnRate} {}
  StateChange Simulate(const FrameData& frameData, const sf::RenderWindow& window, InputManager& inputManager);
};

} // namespace world

#endif

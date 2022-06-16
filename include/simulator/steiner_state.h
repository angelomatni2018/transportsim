#ifndef STEINER_SOLVER
#define STEINER_SOLVER

#include "base/geometric_median.h"
#include "base/kmeans.h"
#include "network/min_spanning_tree.h"
#include "pathing/vehiclepathconstructor.h"
#include "simulator/frame_data.h"
#include "simulator/imports.h"
#include "simulator/sfml_helpers.h"
#include "simulator/simulation_state.h"
#include "simulator/structure_drawer.h"
#include "simulator/visit_spawner.h"
#include "steiner_tree_solver/road_candidate_graph.h"
#include "steiner_tree_solver/steiner_point_search_by_clustering.h"

namespace world {

struct SteinerState {
  Network inputNetwork;
  std::unique_ptr<Network> connectedNetwork;
  VisitSpawner spawner;
  StructureDrawer structureDrawer;
  PtrSet<Path> paths;
  int nextPathId = 0;

  SteinerState(double visitSpawnRate) : spawner{visitSpawnRate} {}
  StateChange Simulate(const FrameData& frameData, const sf::RenderWindow& window, InputManager& inputManager);
};

} // namespace world

#endif

#ifndef SIMULATION_STATE
#define SIMULATION_STATE

#include "simulator/imports.h"
#include "simulator/sfml_helpers.h"
#include "simulator/visit_spawner.h"

struct FrameData {
  const std::chrono::duration<int64_t, std::milli> FRAME_DURATION = std::chrono::milliseconds(50);
  std::chrono::_V2::system_clock::time_point lastFrame = std::chrono::system_clock::now();
  std::chrono::nanoseconds rollingDuration = lastFrame - lastFrame;
  int64_t frameNumber = 0;

  void NextFrame();
};

struct StateChange {
  std::vector<WorldElement*> elements;
  std::unordered_set<Path*>* paths;
};

struct SimulationState {
  Network network;
  VisitSpawner spawner;
  std::unordered_set<Path*> paths;
  int nextPathId = 0;

  SimulationState(double visitSpawnRate) : spawner{visitSpawnRate} {}
  StateChange Simulate(const FrameData& frameData, const sf::RenderWindow& window, InputManager& inputManager);
};

#endif

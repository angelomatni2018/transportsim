#ifndef RENDER_STATE
#define RENDER_STATE

#include "simulator/imports.h"
#include "simulator/sfml_helpers.h"
#include "simulator/simulation_state.h"
#include "simulator/visit_spawner.h"

using namespace world;

struct RenderState {
  sf::Texture squareTexture;
  std::unordered_map<WorldElement*, sf::Sprite*> elementSprites;

  RenderState();
  void Render(sf::RenderWindow& window, const FrameData& frameData, const StateChange& stateChange);
};

#endif
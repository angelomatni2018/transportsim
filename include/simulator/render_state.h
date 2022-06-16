#ifndef RENDER_STATE
#define RENDER_STATE

#include "simulator/imports.h"
#include "simulator/sfml_helpers.h"
#include "simulator/simulation_state.h"
#include "simulator/visit_spawner.h"

namespace world {

class RenderState {
private:
  sf::Texture squareTexture;
  PtrVec<sf::Sprite> sPool;
  std::unordered_map<WorldElement*, sf::Sprite*> elementSprites;

  void drawRoadSegment(sf::RenderWindow& window, RoadSegment* segment);

public:
  RenderState();
  void Render(sf::RenderWindow& window, const FrameData& frameData, const StateChange& stateChange);
};

} // namespace world

#endif
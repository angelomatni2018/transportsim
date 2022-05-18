#ifndef STRUCTURE_DRAWER
#define STRUCTURE_DRAWER

#include <iostream>
#include <math.h>
#include <unordered_map>
#include <vector>

#include "simulator/frame_data.h"
#include "simulator/imports.h"
#include "simulator/sfml_helpers.h"
#include "simulator/simulation_config.h"

namespace world {

class StructureDrawer {
private:
  static constexpr Location NO_LOCATION = Location{INT32_MAX, INT32_MAX};
  Location prevLocation = NO_LOCATION;
  int lastFrameClicked = 0;

public:
  std::vector<WorldElement*> TrackMouseToSpawn(const FrameData& frameData, const sf::RenderWindow& window, InputManager& inputManager,
                                               Network& network);
};

} // namespace world

#endif
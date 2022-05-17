#include "simulator/render_state.h"
#include "simulator/simulation_config.h"
#include "spdlog/spdlog.h"

RenderState::RenderState() {
  // Load THE ONLY sprite we have :(
  if (!squareTexture.loadFromFile("assets/square.png"))
    abort();
}

void draw(sf::RenderWindow& window, sf::Sprite* sprite) {
  auto originalPos = sprite->getPosition();
  // TODO: Figure out how to properly apply global translations and reflections here
  // sprite->setPosition(sf::Vector2f(originalPos.x, GRID_SIZE - originalPos.y));
  // sprite->setRotation(90);
  // spdlog::trace("drawing: {}", to_string(VectorToLocation(sprite->getPosition())));
  window.draw(*sprite);
  sprite->setPosition(originalPos);
}

void RenderState::Render(sf::RenderWindow& window, const FrameData& frameData, const StateChange& stateChange) {
  for (auto element : stateChange.elements) {
    sf::Sprite* sprite = new sf::Sprite(this->squareTexture);
    auto [x, y] = (1.0 / STRUCTURE_BASE_SIZE_UNIT) * element->PrimaryLocation();
    sprite->setScale(sf::Vector2f(1.0 * SQUARE_RESIZE / SQUARE_PIXEL_DIM, 1.0 * SQUARE_RESIZE / SQUARE_PIXEL_DIM));
    sprite->setPosition(sf::Vector2f(1.0 * SCREEN_CENTER + x * SQUARE_RESIZE, 1.0 * SCREEN_CENTER + y * SQUARE_RESIZE));
    if (element->IsType(Roadway::Type)) {
      sprite->setColor(sf::Color(150, 150, 150));
    } else if (element->IsType(CommercialBuilding::Type)) {
      sprite->setColor(sf::Color(255, 0, 0));
    } else if (element->IsType(ResidentialBuilding::Type)) {
      sprite->setColor(sf::Color(150, 0, 0));
    } else {
      sprite->setColor(sf::Color(rand() % 100 + 100, 0, 0));
    }
    elementSprites[element] = sprite;
  }

  for (auto& [element, sprite] : elementSprites) {
    // spdlog::trace("element: {}", to_string(element->PrimaryLocation()));
    draw(window, sprite);

    sf::Sprite spriteDot = sf::Sprite(this->squareTexture);
    spriteDot.setColor(sf::Color(255, 255, 255));
    spriteDot.setScale(sf::Vector2f(.025, .025));
    if (element->IsType(CommercialBuilding::Type)) {
      spriteDot.setColor(sf::Color(200, 0, 0));
      spriteDot.setScale(sf::Vector2f(.1, .1));
    }
    if (element->IsType(ResidentialBuilding::Type)) {
      spriteDot.setColor(sf::Color(100, 0, 0));
      spriteDot.setScale(sf::Vector2f(.1, .1));
    }
    double offsetToCenterRescaledSquare = SQUARE_PIXEL_DIM * spriteDot.getScale().x / 2;
    double centerOfElementSquare = SQUARE_RESIZE / 2 - offsetToCenterRescaledSquare;
    spriteDot.setPosition(sprite->getPosition() + sf::Vector2f(centerOfElementSquare, centerOfElementSquare));
    // spdlog::trace("element dot");
    draw(window, &spriteDot);
  }

  auto drawCar = [&](const PathEvent* pathEvent) {
    sf::Sprite* sprite = new sf::Sprite(this->squareTexture);
    auto [x, y] = pathEvent->locations[0];
    // HACK: Avoid floating point error associated with "unique path middle of nowhere coordinate"
    if (x < -9999999)
      return;
    auto centerAndAlign = [](int v) { return 1.0 * SCREEN_CENTER + v * SQUARE_RESIZE / 4; };
    sprite->setPosition(sf::Vector2f(centerAndAlign(x), centerAndAlign(y)));
    auto randomColorScale = (int64_t(pathEvent->path) & 0xFF);
    sprite->setColor(sf::Color(0, randomColorScale, 0));
    auto numPixels = 0.25 * SQUARE_RESIZE / SQUARE_PIXEL_DIM;
    sprite->setScale(numPixels, numPixels);
    draw(window, sprite);
  };

  std::unordered_set<Path*> toErase;
  for (auto path : *stateChange.paths) {
    if (path->orderedPathEvents.size() <= 1) {
      toErase.emplace(path);
      continue;
    }
    auto nextEvent = path->orderedPathEvents[1];
    if (nextEvent->timeAtPoint > frameData.frameNumber) {
      drawCar(path->orderedPathEvents[0]);
    } else {
      drawCar(nextEvent);
      path->Remove(0);
    }
  }
  for (auto path : toErase) {
    stateChange.paths->erase(path);
    delete path;
  }
}
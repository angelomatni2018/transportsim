#include "simulator/render_state.h"
#include "simulator/simulation_config.h"
#include "spdlog/spdlog.h"

using namespace world;

RenderState::RenderState() {
  // Load THE ONLY sprite we have :(
  if (!squareTexture.loadFromFile("assets/square.png"))
    abort();
}

void draw(sf::RenderWindow& window, sf::Sprite* sprite) {
  auto originalPos = sprite->getPosition();
  /*
    What is this monstrosity of a transformation you may ask?
    It reflects about the x-axis, bringing the origin from being at the top left to being at the bottom left
    A simple (y_new -> MAX - y_old) transformation isn't enough BECAUSE sprite pixels are drawn from the top left.
    We need to additionally offset y_old by the y-dimensional size of the scaled sprite.
  */
  auto scaledYDimSizeOfSprite = sprite->getScale().y * sprite->getTexture()->getSize().y;
  sprite->setPosition(sf::Vector2f(originalPos.x, GRID_SIZE - (originalPos.y + scaledYDimSizeOfSprite)));
  // spdlog::trace("drawing: {}", to_string(VectorToLocation(sprite->getPosition())));
  window.draw(*sprite);
  sprite->setPosition(originalPos);
}

void RenderState::drawRoadSegment(sf::RenderWindow& window, RoadSegment* segment) {
  auto sprite = elementSprites.at(segment);
  draw(window, sprite);

  const std::unordered_map<Location, uint8_t, pair_hash> offsetToValidDirections = {
      {Location{0, 0}, Direction::SouthWest},
      {Location{0, 1}, Direction::SouthWest | Direction::West},
      {Location{0, 2}, Direction::NorthWest | Direction::West},
      {Location{0, 3}, Direction::NorthWest},
      {Location{1, 0}, Direction::South | Direction::SouthWest},
      {Location{1, 1}, ALL_DIRECTIONS},
      {Location{1, 2}, ALL_DIRECTIONS},
      {Location{1, 3}, Direction::North | Direction::NorthWest},
      {Location{2, 0}, Direction::South | Direction::SouthEast},
      {Location{2, 1}, ALL_DIRECTIONS},
      {Location{2, 2}, ALL_DIRECTIONS},
      {Location{2, 3}, Direction::North | Direction::NorthEast},
      {Location{3, 0}, Direction::SouthEast},
      {Location{3, 1}, Direction::SouthEast | Direction::East},
      {Location{3, 2}, Direction::NorthEast | Direction::East},
      {Location{3, 3}, Direction::NorthEast},

      {Location{-1, 0}, Direction::SouthWest},
      {Location{0, -1}, Direction::SouthWest},
      {Location{-1, 3}, Direction::NorthWest},
      {Location{0, 4}, Direction::NorthWest},
      {Location{3, -1}, Direction::SouthEast},
      {Location{4, 0}, Direction::SouthEast},
      {Location{3, 4}, Direction::NorthEast},
      {Location{4, 3}, Direction::NorthEast},
  };

  float subSquareSize = (1.0 / STRUCTURE_BASE_SIZE_UNIT);
  float subSquarePixels = subSquareSize * SQUARE_RESIZE / SQUARE_PIXEL_DIM;
  sf::Sprite subSquare(this->squareTexture);
  subSquare.setScale(sf::Vector2f(subSquarePixels, subSquarePixels));
  subSquare.setColor(sf::Color(200, 200, 200));
  auto gridPos = [subSquareSize](int segmentCoord, int offsetCoord) -> float {
    return SCREEN_CENTER + (segmentCoord * SQUARE_RESIZE) + (subSquareSize * SQUARE_RESIZE * offsetCoord);
  };
  auto [x, y] = subSquareSize * segment->PrimaryLocation();
  auto drawAtOffset = [&, x = x, y = y](int i, int j) {
    if (segment->DirectionsMask() & offsetToValidDirections.at(Location{i, j})) {
      subSquare.setPosition(sf::Vector2f(gridPos(x, i), gridPos(y, j)));
      draw(window, &subSquare);
    }
  };

  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      drawAtOffset(i, j);
    }
  }

  // spill-over to smoothen road connections
  // bottom left
  drawAtOffset(-1, 0);
  drawAtOffset(0, -1);
  // top left
  drawAtOffset(-1, 3);
  drawAtOffset(0, 4);
  // bottom right
  drawAtOffset(3, -1);
  drawAtOffset(4, 0);
  // top right
  drawAtOffset(3, 4);
  drawAtOffset(4, 3);
}

void RenderState::Render(sf::RenderWindow& window, const FrameData& frameData, const StateChange& stateChange) {
  for (auto element : stateChange.elements) {
    sf::Sprite* sprite = new sf::Sprite(this->squareTexture);
    auto [x, y] = (1.0 / STRUCTURE_BASE_SIZE_UNIT) * element->PrimaryLocation();
    sprite->setScale(sf::Vector2f(1.0 * SQUARE_RESIZE / SQUARE_PIXEL_DIM, 1.0 * SQUARE_RESIZE / SQUARE_PIXEL_DIM));
    sprite->setPosition(sf::Vector2f(1.0 * SCREEN_CENTER + x * SQUARE_RESIZE, 1.0 * SCREEN_CENTER + y * SQUARE_RESIZE));
    if (element->IsType(Roadway::Type)) {
      sprite->setColor(sf::Color(100, 100, 100));
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
    if (element->IsType(RoadSegment::Type)) {
      drawRoadSegment(window, static_cast<RoadSegment*>(element));
    } else {
      draw(window, sprite);
    }

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
    // HACK: Avoid floating point error associated with "unique path middle of nowhere coordinate"
    if (pathEvent->locations[0].first < -9999999)
      return;

    for (auto& [x, y] : pathEvent->locations) {
      auto centerAndAlign = [](int v) { return 1.0 * SCREEN_CENTER + v * SQUARE_RESIZE / 4; };
      sprite->setPosition(sf::Vector2f(centerAndAlign(x), centerAndAlign(y)));
      auto randomColorScale = (int64_t(pathEvent->path) & 0xFF);
      sprite->setColor(sf::Color(0, randomColorScale, 0));
      auto numPixels = 0.25 * SQUARE_RESIZE / SQUARE_PIXEL_DIM;
      sprite->setScale(numPixels, numPixels);
      draw(window, sprite);
    }
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
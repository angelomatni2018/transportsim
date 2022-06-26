#include "simulator/structure_drawer.h"

using namespace world;

std::vector<WorldElement*> StructureDrawer::TrackMouseToSpawn(const FrameData& frameData, const sf::RenderWindow& window, InputManager& inputManager,
                                                              Network& network) {

  auto windowSize = window.getSize();
  int windowWidth = windowSize.x;
  int windowHeight = windowSize.y;
  auto aspectRatio = 1.0 * windowWidth / windowHeight;

  if (!inputManager.IsClick(sf::Mouse::Left)) {
    // Check if too long has passed; then forget the prevLocation clicked
    constexpr auto THRESHOLD = std::chrono::milliseconds(3000);
    if (prevLocation != NO_LOCATION && (frameData.FRAME_DURATION * (frameData.frameNumber - lastFrameClicked) > THRESHOLD)) {
      prevLocation = NO_LOCATION;
    }
    return {};
  }
  lastFrameClicked = frameData.frameNumber;

  auto mousePos = sf::Mouse::getPosition(window);
  // HACK: We do 1 - y_fraction here so that 0.2 represents 20% from the bottom of the screen instead of 20% from the top
  auto mousePosFraction = sf::Vector2f(1.0 * mousePos.x / windowWidth, 1.0 - (1.0 * mousePos.y / windowHeight));
  auto mousePosCentered =
      sf::Vector2f((-GRID_CENTER + GRID_SIZE * mousePosFraction.x) * aspectRatio, -GRID_CENTER + (GRID_SIZE * mousePosFraction.y));
  auto currentLocation =
      STRUCTURE_BASE_SIZE_UNIT * Location{floor(mousePosCentered.x / SQUARE_NUM_PIXELS), floor(mousePosCentered.y / SQUARE_NUM_PIXELS)};
  // spdlog::trace("Mouse: {} Square: {}", to_string(VectorToPair<int, int>(mousePos)), to_string(currentLocation));
  // spdlog::trace("Window: {},{}", windowWidth, windowHeight);

  auto isComm = inputManager.IsHold(sf::Keyboard::Num1);
  auto isRes = inputManager.IsHold(sf::Keyboard::Num2);
  if (isComm || isRes) {
    auto buildingDims = STRUCTURE_BASE_SIZE_UNIT * (isComm ? Location{3, 2} : Location{1, 1});
    for (auto loc : Building(buildingDims, currentLocation).AllOccupiedLocations()) {
      if (network.HasStructureAt(loc)) {
        return {};
      }
    }

    Building* spawnElem;
    if (isComm) {
      spawnElem = network.Add(CommercialBuilding(INT32_MAX, buildingDims, currentLocation));
    } else {
      spawnElem = network.Add(ResidentialBuilding(INT32_MAX, buildingDims, currentLocation));
    }
    return {spawnElem};
  }

  if (prevLocation == NO_LOCATION) {
    prevLocation = currentLocation;
    return {};
  }

  auto heading = currentLocation - prevLocation;
  auto [headingX, headingY] = heading;
  // The two locations must be adjacent
  if (abs(headingX) > STRUCTURE_BASE_SIZE_UNIT || abs(headingY) > STRUCTURE_BASE_SIZE_UNIT || (headingX == 0 && headingY == 0)) {
    prevLocation = currentLocation;
    return {};
  }

  std::vector<WorldElement*> newRoadsInPair;
  auto findOrCreate = [&](Location atLoc) -> std::pair<WorldElement*, bool> {
    if (network.HasStructureAt(atLoc)) {
      return {const_cast<WorldElement*>(network.StructureAt(atLoc)), false};
    }
    auto road = static_cast<Roadway*>(network.Add(RoadSegment(atLoc)));
    road->SetDirections(0);
    return {road, true};
  };

  auto [prevEl, isNewPrev] = findOrCreate(prevLocation);
  auto [currentEl, isNewCurrent] = findOrCreate(currentLocation);
  if (isNewPrev)
    newRoadsInPair.push_back(prevEl);
  if (isNewCurrent)
    newRoadsInPair.push_back(currentEl);

  if (prevEl->IsType(RoadSegment::Type)) {
    auto seg = static_cast<RoadSegment*>(prevEl);
    seg->SetDirections(seg->DirectionsMask() | Roadway::DirectionWhenHeading(heading));
  }
  if (currentEl->IsType(RoadSegment::Type)) {
    auto seg = static_cast<RoadSegment*>(currentEl);
    seg->SetDirections(seg->DirectionsMask() | Roadway::DirectionWhenHeading(-1 * heading));
  }

  prevLocation = currentLocation;
  return newRoadsInPair;
}

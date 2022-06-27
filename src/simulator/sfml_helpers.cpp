#include "simulator/sfml_helpers.h"

using namespace world;

void InputManager::NextFrame() {
  keyTogglePrev = keyToggleNext;
  keyToggleNext.clear();
  mouseTogglePrev = mouseToggleNext;
  mouseToggleNext.clear();
}

bool InputManager::IsHold(sf::Keyboard::Key key) {
  return sf::Keyboard::isKeyPressed(key);
}

bool InputManager::IsPress(sf::Keyboard::Key key) {
  keyToggleNext[key] = sf::Keyboard::isKeyPressed(key);
  // spdlog::error("Key {} : {} then {}", key, keyTogglePrev[key], keyToggleNext[key]);
  if (!keyTogglePrev[key] && keyToggleNext[key]) {
    return true;
  }
  return false;
}

bool InputManager::IsHold(sf::Mouse::Button button) {
  return sf::Mouse::isButtonPressed(button);
}

bool InputManager::IsClick(sf::Mouse::Button button) {
  mouseToggleNext[button] = sf::Mouse::isButtonPressed(button);
  if (!mouseTogglePrev[button] && mouseToggleNext[button]) {
    return true;
  }
  return false;
}
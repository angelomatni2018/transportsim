#include "simulator/sfml_helpers.h"

bool InputManager::IsHold(sf::Keyboard::Key key) {
  return sf::Keyboard::isKeyPressed(key);
}

bool InputManager::IsPress(sf::Keyboard::Key key) {
  if (sf::Keyboard::isKeyPressed(key)) {
    if (!keyToggle[key]) {
      keyToggle[key] = true;
      return true;
    }
  } else {
    keyToggle[key] = false;
  }
  return false;
}

bool InputManager::IsClick(sf::Mouse::Button button) {
  if (sf::Mouse::isButtonPressed(button)) {
    if (!mouseToggle[button]) {
      mouseToggle[button] = true;
      return true;
    }
  } else {
    mouseToggle[button] = false;
  }
  return false;
}
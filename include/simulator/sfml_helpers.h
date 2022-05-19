#ifndef SFML_HELPERS
#define SFML_HELPERS

#include "simulator/imports.h"

namespace world {

class InputManager {
public:
  std::unordered_map<sf::Keyboard::Key, bool> keyToggle;
  std::unordered_map<sf::Mouse::Button, bool> mouseToggle;

  bool IsPress(sf::Keyboard::Key key);
  bool IsHold(sf::Keyboard::Key key);

  bool IsClick(sf::Mouse::Button button);
  bool IsHold(sf::Mouse::Button button);
};

template <class T>
Location VectorToLocation(sf::Vector2<T> loc) {
  return std::make_pair(round(loc.x), round(loc.y));
}

template <class T>
sf::Vector2<T> Vector2(T val) {
  return sf::Vector2<T>(val, val);
}

} // namespace world

#endif
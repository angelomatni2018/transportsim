#ifndef SFML_HELPERS
#define SFML_HELPERS

#include <SFML/Graphics.hpp>

#include "base/spatial.h"

using namespace world;

class InputManager {
public:
    std::unordered_map<sf::Keyboard::Key, bool> keyToggle;
    std::unordered_map<sf::Mouse::Button, bool> mouseToggle;

    bool IsPress(sf::Keyboard::Key key) ;
    bool IsHold(sf::Keyboard::Key key) ;

    bool IsClick(sf::Mouse::Button button) ;
};

template <class T>
Location VectorToLocation(sf::Vector2<T> loc) {
    return std::make_pair(round(loc.x), round(loc.y));
}

#endif
#ifndef SFML_HELPERS
#define SFML_HELPERS

#include <SFML/Graphics.hpp>

#include "base/spatial.h"

using namespace world;

bool isKeyHold(std::unordered_map<sf::Keyboard::Key, bool> &keyToggle, sf::Keyboard::Key key) ;
bool isKeyPress(std::unordered_map<sf::Keyboard::Key, bool> &keyToggle, sf::Keyboard::Key key) ;

template <class T>
Location vectorToLocation(sf::Vector2<T> loc) {
    return std::make_pair(round(loc.x), round(loc.y));
}

#endif
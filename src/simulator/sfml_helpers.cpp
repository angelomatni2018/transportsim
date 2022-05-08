#include "simulator/sfml_helpers.h"

bool isKeyHold(std::unordered_map<sf::Keyboard::Key, bool> &keyToggle, sf::Keyboard::Key key) {
    if (sf::Keyboard::isKeyPressed(key)) {
        if (!keyToggle[key]) {
            keyToggle[key] = true;
        }
        return true;
    } else {
        keyToggle[key] = false;
    }
    return false;
}

bool isKeyPress(std::unordered_map<sf::Keyboard::Key, bool> &keyToggle, sf::Keyboard::Key key) {
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
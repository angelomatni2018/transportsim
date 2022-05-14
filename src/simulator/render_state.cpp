#include "simulator/render_state.h"
#include "simulator/simulation_config.h"

RenderState::RenderState () {
    // Load THE ONLY sprite we have :(
    if (!squareTexture.loadFromFile("assets/square.png")) abort();
}

void RenderState::Render(sf::RenderWindow &window, const FrameData &frameData, const StateChange &stateChange) {
    for (auto element : stateChange.elements) {
        sf::Sprite * sprite = new sf::Sprite(this->squareTexture);
        auto [x, y] = (1.0 / STRUCTURE_BASE_SIZE_UNIT) * element->PrimaryLocation();
        sprite->setScale(sf::Vector2f(1.0 * SQUARE_RESIZE / SQUARE_PIXEL_DIM, 1.0 * SQUARE_RESIZE / SQUARE_PIXEL_DIM));
        sprite->setPosition(sf::Vector2f(1.0 * SCREEN_CENTER + x * SQUARE_RESIZE, 1.0 * SCREEN_CENTER + y * SQUARE_RESIZE));
        if (Roadway* r = dynamic_cast<Roadway*>(element); r != nullptr) {
            sprite->setColor(sf::Color(150, 150, 150));
        } else if (CommercialBuilding *r = dynamic_cast<CommercialBuilding*>(element); r != nullptr) {
            sprite->setColor(sf::Color(255, 0, 0));
        } else if (ResidentialBuilding *r = dynamic_cast<ResidentialBuilding*>(element); r != nullptr) {
            sprite->setColor(sf::Color(150, 0, 0));
        } else {
            sprite->setColor(sf::Color(rand() % 100 + 100, 0, 0));
        }
        elementSprites[element] = sprite;
    }

    for (auto &[element, sprite] : elementSprites) {
        window.draw(*sprite);

        sf::Sprite spriteDot = sf::Sprite(this->squareTexture);
        spriteDot.setColor(sf::Color(255, 255, 255));
        spriteDot.setScale(sf::Vector2f(.025, .025));
        if (CommercialBuilding *r = dynamic_cast<CommercialBuilding*>(element); r != nullptr) {
            spriteDot.setColor(sf::Color(200, 0, 0));
            spriteDot.setScale(sf::Vector2f(.1, .1));
        }
        if (ResidentialBuilding *r = dynamic_cast<ResidentialBuilding*>(element); r != nullptr) {
            spriteDot.setColor(sf::Color(100, 0, 0));
            spriteDot.setScale(sf::Vector2f(.1, .1));
        }
        double offsetToCenterRescaledSquare = SQUARE_PIXEL_DIM * spriteDot.getScale().x / 2;
        spriteDot.setPosition(sprite->getPosition() + sf::Vector2f(SQUARE_RESIZE / 2 - offsetToCenterRescaledSquare,
            SQUARE_RESIZE / 2 - offsetToCenterRescaledSquare));
        window.draw(spriteDot);

    }

    auto drawCar = [&](const PathEvent *pathEvent) {
        sf::Sprite * sprite = new sf::Sprite(this->squareTexture);
        auto [x, y] = pathEvent->locations[0];
        sprite->setPosition(sf::Vector2f(1.0 * SCREEN_CENTER + x * SQUARE_RESIZE/4, 1.0 * SCREEN_CENTER + y * SQUARE_RESIZE/4));
        auto randomColorScale = (int64_t(pathEvent->path) & 0xFF);
        sprite->setColor(sf::Color(0, randomColorScale, 0));
        sprite->setScale(0.25 * SQUARE_RESIZE / SQUARE_PIXEL_DIM, 0.25 * SQUARE_RESIZE / SQUARE_PIXEL_DIM);
        window.draw(*sprite);
    };

    std::unordered_set<Path *> toErase;
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
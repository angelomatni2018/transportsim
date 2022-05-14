#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "spdlog/spdlog.h"

#include <iostream>
#include <unistd.h>
#include <chrono>
#include <thread>

#include "simulator/game_simulator.h"
#include "pathing/pathfinder.h"
#include "pathing/pathreconciler.h"
#include "pathing/vehiclepathconstructor.h"

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

using namespace world;

const int GRID_CENTER = 500;
const int GRID_SIZE = 1000;
const int SQUARE_PIXEL_DIM = 100;
const int SQUARE_RESIZE = 64;
const int SCREEN_CENTER = GRID_CENTER;

void drawGrid(sf::RenderWindow& win){
    int rows = 10 * 2;
    int cols = 10 * 2;
    // initialize values
    int numLines = rows+cols-2;
    sf::VertexArray grid(sf::Lines, 2*(numLines));
    auto size = sf::Vector2f(GRID_SIZE, GRID_SIZE);
    float rowH = size.y/rows;
    float colW = size.x/cols;
    // row separators
    for(int i=0; i < rows-1; i++){
        int r = i+1;
        float rowY = rowH*r;
        grid[i*2].position = {0, rowY};
        grid[i*2+1].position = {size.x, rowY};
        grid[i*2].color = sf::Color(255, 255, 255);
        grid[i*2+1].color = sf::Color(255, 255, 255);
    }
    // column separators
    for(int i=rows-1; i < numLines; i++){
        int c = i-rows+2;
        float colX = colW*c;
        grid[i*2].position = {colX, 0};
        grid[i*2+1].position = {colX, size.y};
    }
    // draw it
    win.draw(grid);
}

int main() {
    signal(SIGSEGV, handler);
    spdlog::set_level(spdlog::level::trace);

    // Create the main window
    auto desktopMode = sf::VideoMode::getDesktopMode();
    int screenWidth = desktopMode.width/2;
    int screenHeight = desktopMode.height/2;
    spdlog::debug("Screen: {}x{}", screenWidth, screenHeight);
    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "SFML window");
    auto aspectRatio = 1.0 * desktopMode.width / desktopMode.height;
    sf::View view(sf::Vector2f(GRID_CENTER, GRID_CENTER), sf::Vector2f(GRID_SIZE * aspectRatio, GRID_SIZE));
    window.setView(view);

    InputManager inputManager;
    FrameData frameData;
    int numFramesPerSpawn = 100;
    SimulationState simState(numFramesPerSpawn);
    RenderState renderState;
    while (window.isOpen())
    {
        if (inputManager.IsPress(sf::Keyboard::Escape)) {
            window.close();
        }

        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed) window.close();
            else if (event.type == sf::Event::Resized) {
                screenWidth = event.size.width;
                screenHeight = event.size.height;
                aspectRatio = 1.0 * event.size.width / event.size.height;
                spdlog::debug("Resize: {}x{} (aspect ratio {:.3f})", screenWidth, screenHeight, aspectRatio);
                sf::View view(sf::Vector2f(GRID_CENTER, GRID_CENTER), sf::Vector2f(GRID_SIZE * aspectRatio, GRID_SIZE));
                window.setView(view);
            }
        }

        if (inputManager.IsPress(sf::Keyboard::Up)) {
            view.setSize(view.getSize() + sf::Vector2f(100 * aspectRatio, 100));
            window.setView(view);
        }
        if (inputManager.IsPress(sf::Keyboard::Down)) {
            view.setSize(view.getSize() - sf::Vector2f(100 * aspectRatio, 100));
            window.setView(view);
        }

        auto stateChange = simState.Simulate(frameData, window, inputManager);

        window.clear();
        // drawGrid(window);
        renderState.Render(window, frameData, stateChange);
        window.display();

        // TODO: Adapt the sleep duration so that an iteration of this while loop is as close to the target length of a frame as possible 
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        frameData.NextFrame();
    }
    return 0;
}

void FrameData::NextFrame() {
    auto nextLastFrame = std::chrono::system_clock::now();
    this->rollingDuration += nextLastFrame - lastFrame;
    this->lastFrame = nextLastFrame;

    if (rollingDuration > FRAME_DURATION) {
        rollingDuration -= FRAME_DURATION;
        ++this->frameNumber;
    }
}

StateChange SimulationState::Simulate(const FrameData &frameData, const sf::RenderWindow &window, InputManager &inputManager) {
    StateChange stateChange;
    stateChange.paths = &this->paths;

    if (network.Roads().size() == 0) {
        int starterRoadLength = 3;
        auto size1 = STRUCTURE_BASE_SIZE_UNIT * Location{1, 1};
        auto leftBuilding = new ResidentialBuilding(1, size1, STRUCTURE_BASE_SIZE_UNIT * Location{-1, 0});
        auto rightBuilding = new CommercialBuilding(1, size1, STRUCTURE_BASE_SIZE_UNIT * Location{starterRoadLength, 0});
        this->network.AddBuilding(leftBuilding);
        this->network.AddBuilding(rightBuilding);
        stateChange.elements.push_back(leftBuilding);
        stateChange.elements.push_back(rightBuilding);
        for (auto i = 0; i < starterRoadLength; ++i) {
            auto firstRoad = new RoadSegment(STRUCTURE_BASE_SIZE_UNIT * Location{i, 0});
            this->network.AddRoadway(firstRoad);
            stateChange.elements.push_back(firstRoad);
        }
        return stateChange;
    }

    auto visits = this->spawner.Spawn(this->network, 1);
    for (auto visit : visits) {
        auto comm = visit.first; auto res = visit.second;

        const int FRAMES_PER_TILE = 4;
        VehiclePathConstructor pathConstructor(this->network, visit, frameData.frameNumber, FRAMES_PER_TILE);
        auto path = pathConstructor.path;
        spdlog::trace("Path spawned: res at {} to comm at {} (path length {})",
            to_string(res->PrimaryLocation()), to_string(comm->PrimaryLocation()), path->orderedPathEvents.size());
        this->paths.emplace(path);
        ++pathUniqueId;
        auto uniqueLocInMiddleOfNowhere = std::make_pair(INT32_MIN + pathUniqueId, 0);
        path->Append({uniqueLocInMiddleOfNowhere}, path->orderedPathEvents.back()->timeAtPoint + FRAMES_PER_TILE);
    }
    if (visits.size() > 0) {
        PathReconciler().Reconcile(paths);
    }

    auto windowSize = window.getSize();
    int windowWidth = windowSize.x;
    int windowHeight = windowSize.y;
    auto aspectRatio = 1.0 * windowWidth / windowHeight;
    if (inputManager.IsClick(sf::Mouse::Left)) {
        auto mousePos = sf::Mouse::getPosition(window);
        auto mousePosFraction = sf::Vector2f(1.0 * mousePos.x / windowWidth, 1.0 * mousePos.y / windowHeight);
        auto mousePosCentered = sf::Vector2f(
            (-GRID_CENTER + GRID_SIZE * mousePosFraction.x) * aspectRatio,
            -GRID_CENTER + (GRID_SIZE * mousePosFraction.y));
        auto squareLoc = VectorToLocation(sf::Vector2i(floor(mousePosCentered.x / SQUARE_RESIZE), floor(mousePosCentered.y / SQUARE_RESIZE)));
        squareLoc = STRUCTURE_BASE_SIZE_UNIT * squareLoc;
        spdlog::trace("Mouse: {} Square: {}", to_string(VectorToLocation(mousePos)), to_string(squareLoc));
        spdlog::trace("Window: {},{}", windowWidth, windowHeight);

        if (!this->network.HasStructureAt(squareLoc)) {
            WorldElement *spawnElem = nullptr;
            if (inputManager.IsHold(sf::Keyboard::Num1)) {
                spawnElem = new CommercialBuilding(1, std::make_pair(STRUCTURE_BASE_SIZE_UNIT, STRUCTURE_BASE_SIZE_UNIT), squareLoc);
                this->network.AddBuilding((Building *)spawnElem);
            } else if (inputManager.IsHold(sf::Keyboard::Num2)) {
                spawnElem = new ResidentialBuilding(1, std::make_pair(STRUCTURE_BASE_SIZE_UNIT, STRUCTURE_BASE_SIZE_UNIT), squareLoc);
                this->network.AddBuilding((Building *)spawnElem);
            } else {
                spawnElem = new RoadSegment(squareLoc);
                this->network.AddRoadway((Roadway *)spawnElem);
            }
            stateChange.elements.push_back(spawnElem);
        }
    }

    return stateChange;
}

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
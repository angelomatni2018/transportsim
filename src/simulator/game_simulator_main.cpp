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

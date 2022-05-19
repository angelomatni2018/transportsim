#ifndef SIMULATION_CONFIG
#define SIMULATION_CONFIG

#include "simulator/imports.h"

// The length of each unit square of a structure adhering to the base number of sub-squares a structure must have
constexpr float SUBSQUARE_SIZE = 1.0 / world::STRUCTURE_BASE_SIZE_UNIT;

// TODO: Make this configurable
constexpr int GRID_CENTER = 500;
constexpr int GRID_SIZE = 1000;
constexpr int SQUARE_PIXEL_DIM = 100;
constexpr int SQUARE_NUM_PIXELS = 64;
constexpr int SUBSQUARE_NUM_PIXELS = SUBSQUARE_SIZE * SQUARE_NUM_PIXELS;
constexpr int SCREEN_CENTER = GRID_CENTER;

// The loaded square texture has length SQUARE_PIXEL_DIM; this scale factor changes the length to SQUARE_RESIZE
constexpr float SQUARE_SCALE_FACTOR = 1.0 * SQUARE_NUM_PIXELS / SQUARE_PIXEL_DIM;

constexpr float SUBSQUARE_SCALE_FACTOR = SUBSQUARE_SIZE * SQUARE_SCALE_FACTOR;

#endif

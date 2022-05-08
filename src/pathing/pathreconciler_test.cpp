#include "pathing/pathreconciler.h"
#include <assert.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "external/catch.hpp"

using namespace world;
using namespace std;

unsigned int PathReconciled() {
    auto path = new Path();
    const int FRAMES_PER_TILE = 1;
    for (int i = 0; i < 10; ++i) {
        auto pathEvent = new PathEvent(path);
        pathEvent->location = std::make_pair(i, 0);
        pathEvent->index = i;
        pathEvent->timeAtPoint = FRAMES_PER_TILE * i;
        pathEvent->direction = Direction::NO_DIR;
        path->orderedPathEvents.push_back(pathEvent);
    }
    return PathReconciler::reconcile({path});
}

TEST_CASE( "Simple path is reconciled", "[]" ) {
    REQUIRE( PathReconciled() == true );
}

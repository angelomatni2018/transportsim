#include "pathing/pathreconciler.h"
#include <assert.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "external/catch.hpp"

#include "spdlog/spdlog.h"

using namespace world;
using namespace std;

unsigned int TrailingPathReconciled() {
    const int FRAMES_PER_TILE = 3;

    Path slowerPath;
    for (int i = 1; i < 10; ++i) {
        slowerPath.Append({std::make_pair(i, 0)}, 2 * FRAMES_PER_TILE * (i - 1));
    }

    Path fasterPath;
    for (int i = 0; i < 9; ++i) {
        fasterPath.Append({std::make_pair(i, 0)}, FRAMES_PER_TILE * i);
    }

    if (!PathReconciler::Reconcile({&slowerPath, &fasterPath})) {
        spdlog::error("TrailingPathReconciled: could not reconcile");
        return false;
    }

    for (int i = 1; i < 9; ++i) {
        if (fasterPath.orderedPathEvents[i]->timeAtPoint != 2 * FRAMES_PER_TILE * i) {
            spdlog::error("TrailingPathReconciled: did not correctly reconcile event {}", to_string(*fasterPath.orderedPathEvents[i]));
            return false;
        }
    }
    return true;
}

TEST_CASE( "Simple path is reconciled", "[]" ) {
    // spdlog::set_level(spdlog::level::trace);
    REQUIRE( TrailingPathReconciled() == true );
}

#include "evaluator/traffic_evaluator.h"

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "external/catch.hpp"

#include "spdlog/spdlog.h"

using namespace world;

bool StraightLineOverwhelmed() {
    auto scaled = [](Location loc) { return STRUCTURE_BASE_SIZE_UNIT * loc; };
    Network network;
    CommercialBuilding comm(10, scaled({1,1}), scaled({5,0}));
    network.AddBuilding(&comm);
    ResidentialBuilding res(5, scaled({1,1}), scaled({0,0}));
    network.AddBuilding(&res);
    RoadSegment road1(scaled({1,0})); RoadSegment road2(scaled({2, 0})); RoadSegment road3(scaled({3, 0})); RoadSegment road4(scaled({4,0}));
    for (auto road : std::initializer_list<RoadSegment *>{&road1, &road2, &road3, &road4})
        network.AddRoadway(road);

    auto check = [](TrafficEvaluator &evaluator, double expected=100) -> bool {
        auto actual = evaluator.TimeBeforeOverload(100);
        spdlog::trace("StraightLineOverwhelmed: time to overwhelm: expected {} actual {}", expected, actual);
        if (!equalish(actual, expected)) {
            return false;
        }
        return true;
    };

    double vehicleTilesPerSecond = 2.15;
    double secondsBetweenVisits = 0.999;
    TrafficEvaluator evaluatorTooFast(network, secondsBetweenVisits, vehicleTilesPerSecond);

    vehicleTilesPerSecond = 2.15;
    secondsBetweenVisits = 1.0;
    TrafficEvaluator evaluatorJustRight(network, secondsBetweenVisits, vehicleTilesPerSecond);

    return check(evaluatorJustRight) && check(evaluatorTooFast, 0.999 * 5);
}

TEST_CASE( "Straight line road with spawns faster than travel time is overwhelmed", "[]" ) {
    AddSegfaultHandler();
    spdlog::set_level(spdlog::level::trace);
    REQUIRE( StraightLineOverwhelmed() == true );
}

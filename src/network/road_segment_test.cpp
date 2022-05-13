#include "network/world_elements.h"
#include <assert.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "external/catch.hpp"

#include "spdlog/spdlog.h"

using namespace world;

bool isSmoothAndConnected(std::vector<Location> path) {
    if (path.size() < 2) return false;
    for (auto i = 1; i < path.size(); ++i) {
        auto difference = path[i] - path[i-1];
        if (difference.first == 0 && difference.second == 0) return false;
        if (difference.first > 1 || difference.second > 1) return false;
    }
    return true;
}

bool SmoothConnectivity() {

    std::vector<Location> directions;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) continue;
            directions.push_back({i, j});
        }
    }

    RoadSegment seg1({0, 0});
    int numGoodPaths = 0;
    for (auto firstDir : directions) {
        for (auto secondDir : directions) {
            if (firstDir == secondDir) continue;
            for (auto thirdDir : directions) {
                if (secondDir == thirdDir) continue;
                RoadSegment seg2(seg1.PrimaryLocation() + STRUCTURE_BASE_SIZE_UNIT * firstDir);
                RoadSegment seg3(seg2.PrimaryLocation() + STRUCTURE_BASE_SIZE_UNIT * secondDir);
                RoadSegment seg4(seg3.PrimaryLocation() + STRUCTURE_BASE_SIZE_UNIT * thirdDir);

                std::vector<Location> path23;
                std::vector<Location> offsets23;
                auto path123 = seg2.LocationsThrough(seg1.PrimaryLocation(), seg3.PrimaryLocation());
                auto path234 = seg3.LocationsThrough(seg2.PrimaryLocation(), seg4.PrimaryLocation());
                auto offs123 = seg2.offsetsThrough(seg1.PrimaryLocation(), seg3.PrimaryLocation());
                auto offs234 = seg3.offsetsThrough(seg2.PrimaryLocation(), seg4.PrimaryLocation());
                path23.insert(path23.end(), path123.begin(), path123.end());
                path23.insert(path23.end(), path234.begin(), path234.end());
                offsets23.insert(offsets23.end(), offs123.begin(), offs123.end());
                offsets23.insert(offsets23.end(), offs234.begin(), offs234.end());
                if (!isSmoothAndConnected(path23)) {
                    spdlog::error("RoadSegmentTest: bad path ({}) -> ({}) -> ({}) -> ({})",
                        to_string(seg1.PrimaryLocation()), to_string(seg2.PrimaryLocation()),
                        to_string(seg3.PrimaryLocation()), to_string(seg4.PrimaryLocation())
                    );
                    for (int i = 0; i < path23.size(); ++i) {
                        spdlog::error("{}\t{}", to_string(path23[i]), to_string(offsets23[i]));
                    }
                    return false;
                } else {
                    spdlog::trace("RoadSegmentTest: good path ({}) -> ({}) -> ({}) -> ({})",
                        to_string(seg1.PrimaryLocation()), to_string(seg2.PrimaryLocation()),
                        to_string(seg3.PrimaryLocation()), to_string(seg4.PrimaryLocation())
                    );
                }
                ++numGoodPaths;
            }
        }
    }

    spdlog::info("RoadSegmentTest: {} good paths confirmed", numGoodPaths);
    return true;
}

TEST_CASE( "Path through road segments is smooth and connected", "[]" ) {
    // spdlog::set_level(spdlog::level::trace);
    REQUIRE( SmoothConnectivity() == true );
}

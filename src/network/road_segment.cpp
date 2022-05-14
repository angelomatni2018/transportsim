#include "network/world_elements.h"
#include "spdlog/spdlog.h"

using namespace world;

RoadSegment::RoadSegment(Location loc)
    : Roadway{{STRUCTURE_BASE_SIZE_UNIT, STRUCTURE_BASE_SIZE_UNIT}, loc} {
}

// STRUCTURE_BASE_SIZE_UNIT is statically asserted to be 4
// offsets are defined from the primary location which by convention is the lower left corner
// Spill-over into the corner of adjacent structures en route to the "to" structure is the
// responsibility of this structure
std::vector<Location> RoadSegment::offsetsThrough(Location from, Location to) {
    auto &[fromX, fromY] = from;
    auto &[selfX, selfY] = this->primaryLocation;
    auto &[toX, toY] = to;
    auto formatErr = [&](std::string errorMsg) {
        spdlog::error("RoadSegment::offsetsThrough {}  ({}) -> ({}) -> ({})",
            errorMsg, to_string(from), to_string(this->primaryLocation), to_string(to));
    };

    // if (!(fromX < selfX ^ toX < selfX)) {
    //     formatErr("from and to must surround");
    //     abort();
    // }
    // if (!(fromY < selfY ^ toY < selfY)) {
    //     formatErr("from and to must surround");
    //     abort();
    // }

    auto strictInc = [](int x, int y, int z) { return x < y && y < z; };
    auto strictDec = [](int x, int y, int z) { return x > y && y > z; };
    auto incThenEq = [](int x, int y, int z) { return x < y && y == z; };
    auto decThenEq = [](int x, int y, int z) { return x > y && y == z; };
    auto eqThenInc = [](int x, int y, int z) { return x == y && y < z; };
    auto eqThenDec = [](int x, int y, int z) { return x == y && y > z; };
    auto incThenDec = [](int x, int y, int z) { return x < y && y > z; };
    auto decThenInc = [](int x, int y, int z) { return x > y && y < z; };
    auto allEq = [](int x, int y, int z) { return x == y && y == z; };

    auto axisX = [fromX=fromX, selfX=selfX, toX=toX](bool (*f)(int,int,int))
        { return f(fromX, selfX, toX); };
    auto axisY = [fromY=fromY, selfY=selfY, toY=toY](bool (*f)(int,int,int))
        { return f(fromY, selfY, toY); };

    if (axisX(allEq) && axisY(allEq)) {
        formatErr("from -> self -> to should not stay still");
        abort();
    }

    // straight right
    if (axisX(strictInc) && axisY(allEq)) return {{0,1},{1,1},{2,1},{3,1}};
    // straight left
    if (axisX(strictDec) && axisY(allEq)) return {{3,2},{2,2},{1,2},{0,2}};
    // straight up
    if (axisX(allEq) && axisY(strictInc)) return {{2,0},{2,1},{2,2},{2,3}};
    // straight down
    if (axisX(allEq) && axisY(strictDec)) return {{1,3},{1,2},{1,1},{1,0}};

    // right then left
    if (axisX(incThenDec) && axisY(allEq)) return {{0,1},{0,2}};
    // left then right
    if (axisX(decThenInc) && axisY(allEq)) return {{3,2},{3,1}};
    // up then down
    if (axisX(allEq) && axisY(incThenDec)) return {{2,0},{1,0}};
    // down then up
    if (axisX(allEq) && axisY(decThenInc)) return {{1,3},{2,3}};

    // right + up
    if (axisX(strictInc) && axisY(strictInc)) return {{1,0},{2,1},{3,2},{4,3}};
    // right + down
    if (axisX(strictInc) && axisY(strictDec)) return {{0,2},{1,1},{2,0},{3,-1}};
    // left + up
    if (axisX(strictDec) && axisY(strictInc)) return {{3,1},{2,2},{1,3},{0,4}};
    // left + down
    if (axisX(strictDec) && axisY(strictDec)) return {{2,3},{1,2},{0,1},{-1,0}};

    // right + up then left + down
    if (axisX(incThenDec) && axisY(incThenDec)) return {{1,0},{0,1},{-1,0}};
    // right + down then left + up
    if (axisX(incThenDec) && axisY(decThenInc)) return {{0,2},{1,3},{0,4}};
    // left + up then right + down
    if (axisX(decThenInc) && axisY(incThenDec)) return {{3,1},{2,0},{3,-1}};
    // left + down then right + up
    if (axisX(decThenInc) && axisY(decThenInc)) return {{2,3},{3,2},{4,3}};

    // right then down
    if (axisX(incThenEq) && axisY(eqThenDec)) return {{0,1},{1,0}};
    // right then up
    if (axisX(incThenEq) && axisY(eqThenInc)) return {{0,1},{1,1},{2,2},{2,3}};
    // left then down
    if (axisX(decThenEq) && axisY(eqThenDec)) return {{3,2},{2,2},{1,1},{1,0}};
    // left then up
    if (axisX(decThenEq) && axisY(eqThenInc)) return {{3,2},{2,3}};
    // up then right
    if (axisX(eqThenInc) && axisY(incThenEq)) return {{2,0},{3,1}};
    // up then left
    if (axisX(eqThenDec) && axisY(incThenEq)) return {{2,0},{2,1},{1,2},{0,2}};
    // down then right
    if (axisX(eqThenInc) && axisY(decThenEq)) return {{1,3},{1,2},{2,1},{3,1}};
    // down then left
    if (axisX(eqThenDec) && axisY(decThenEq)) return {{1,3},{0,2}};

    // right then right + down
    if (axisX(strictInc) && axisY(eqThenDec)) return {{0,1},{1,1},{2,0},{3,-1}};
    // left + up then left
    if (axisX(strictDec) && axisY(incThenEq)) return {{3,1},{2,2},{1,2},{0,2}};
    // right then right + up
    if (axisX(strictInc) && axisY(eqThenInc)) return {{0,1},{1,1},{2,1},{3,2},{4,3}};
    // left + down then left
    if (axisX(strictDec) && axisY(decThenEq)) return {{2,3},{1,2},{0,2}};

    // right then left + down
    if (axisX(incThenDec) && axisY(eqThenDec)) return {{0,1},{-1,0}};
    // right + up then left
    if (axisX(incThenDec) && axisY(incThenEq)) return {{1,0},{2,1},{1,2},{0,2}};
    // right then left + up
    if (axisX(incThenDec) && axisY(eqThenInc)) return {{0,1},{1,1},{2,2},{1,3},{0,4}};
    // right + down then left 
    if (axisX(incThenDec) && axisY(decThenEq)) return {{0,2}};

    // left then left + down
    if (axisX(strictDec) && axisY(eqThenDec)) return {{3,2},{2,2},{1,2},{0,1},{-1,0}};
    // right + up then right
    if (axisX(strictInc) && axisY(incThenEq)) return {{1,0},{2,1},{3,1}};
    // left then left + up
    if (axisX(strictDec) && axisY(eqThenInc)) return {{3,2},{2,2},{1,3},{0,4}};
    // right + down then right
    if (axisX(strictInc) && axisY(decThenEq)) return {{0,2},{1,1},{2,1},{3,1}};

    // left then right + down
    if (axisX(decThenInc) && axisY(eqThenDec)) return {{3,2},{2,2},{1,1},{2,0},{3,-1}};
    // left + up then right
    if (axisX(decThenInc) && axisY(incThenEq)) return {{3,1}};
    // left then right + up
    if (axisX(decThenInc) && axisY(eqThenInc)) return {{3,2},{4,3}};
    // left + down then right
    if (axisX(decThenInc) && axisY(decThenEq)) return {{2,3},{1,2},{2,1},{3,1}};

    // up then up + right
    if (axisX(eqThenInc) && axisY(strictInc)) return {{2,0},{2,1},{3,2},{4,3}};
    // down + left then down
    if (axisX(decThenEq) && axisY(strictDec)) return {{2,3},{1,2},{1,1},{1,0}};
    // up then up + left
    if (axisX(eqThenDec) && axisY(strictInc)) return {{2,0},{2,1},{2,2},{1,3},{0,4}};
    // down + right then down
    if (axisX(incThenEq) && axisY(strictDec)) return {{0,2},{1,1},{1,0}};

    // up then down + right
    if (axisX(eqThenInc) && axisY(incThenDec)) return {{2,0},{3,-1}};
    // up + left then down
    if (axisX(decThenEq) && axisY(incThenDec)) return {{3,1},{2,2},{1,1},{1,0}};
    // up then down + left
    if (axisX(eqThenDec) && axisY(incThenDec)) return {{2,0},{2,1},{1,2},{0,1},{-1,0}};
    // up + right then down
    if (axisX(incThenEq) && axisY(incThenDec)) return {{1,0}};

    // down then down + right
    if (axisX(eqThenInc) && axisY(strictDec)) return {{1,3},{1,2},{1,1},{2,0},{3,-1}};
    // up + left then up
    if (axisX(decThenEq) && axisY(strictInc)) return {{3,1},{2,2},{2,3}};
    // down then down + left
    if (axisX(eqThenDec) && axisY(strictDec)) return {{1,3},{1,2},{0,1},{-1,0}};
    // up + right then up
    if (axisX(incThenEq) && axisY(strictInc)) return {{1,0},{2,1},{2,2},{2,3}};

    // down then up + right
    if (axisX(eqThenInc) && axisY(decThenInc)) return {{1,3},{1,2},{2,1},{3,2},{4,3}};
    // down + left then up
    if (axisX(decThenEq) && axisY(decThenInc)) return {{2,3}};
    // down then up + left
    if (axisX(eqThenDec) && axisY(decThenInc)) return {{1,3},{0,4}};
    // down + right then up
    if (axisX(incThenEq) && axisY(decThenInc)) return {{0,2},{1,1},{2,2},{2,3}};

    // down + left then up + left
    if (axisX(strictDec) && axisY(decThenInc)) return {{2,3},{1,3},{0,4}};
    // down + right then up + right
    if (axisX(strictInc) && axisY(decThenInc)) return {{0,2},{1,1},{2,1},{3,2},{4,3}};
    // up + left then down + left
    if (axisX(strictDec) && axisY(incThenDec)) return {{3,1},{2,2},{2,1},{1,0},{0,-1}};
    // up + right then down + right
    if (axisX(strictInc) && axisY(incThenDec)) return {{1,0},{2,0},{3,-1}};

    // down + right then down + left
    if (axisX(incThenDec) && axisY(strictDec)) return {{0,2},{0,1},{-1,0}};
    // up + right then up + left 
    if (axisX(incThenDec) && axisY(strictInc)) return {{1,0},{2,1},{2,2},{1,3},{0,4}};
    // down + left then down + right
    if (axisX(decThenInc) && axisY(strictDec)) return {{2,3},{1,2},{1,1},{2,0},{3,-1}};
    // up + left then up + right 
    if (axisX(decThenInc) && axisY(strictInc)) return {{3,1},{3,2},{4,3}};

    formatErr("unreachable");
    abort();
}

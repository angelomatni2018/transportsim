#ifndef PATHRECONCILER
#define PATHRECONCILER

#include <iostream>
#include <math.h>
#include <unordered_map>
#include <unordered_set>
// #include <ctime>
#include <queue>
#include <algorithm>
#include <chrono>
#include <iostream>

#include "base/network.h"
#include "base/spatial.h"

namespace world
{

    // TODO: Figure out a better way of communicating when a path's element should disappear upon arrival to the last location
    // Right now, that is done using a nullptr PathEvent, resulting in (pathEvent == nullptr) checks everywhere which is a mess

    class Path;

    struct PathEvent
    {
        Path *path;
        int index;
        Location location;
        Direction direction;
        double timeAtPoint;

        PathEvent(Path *path) : path{path}, index{-1}, location{INT32_MAX, INT32_MAX}, direction{Direction::NO_DIR}, timeAtPoint{-1} {};

        LocDir locDir() { return std::make_pair(location, direction); };
        void delay(double amount) ;
    };

    inline std::ostream& operator<<(std::ostream& os, const PathEvent& pathEvent) {
        os << pathEvent.index << ":" << pathEvent.location << "," << pathEvent.direction << "(" << pathEvent.timeAtPoint << ")";
        return os;
    }

    class Path
    {
    public:
        std::vector<PathEvent *> orderedPathEvents;

        void remove(int idx) {
            for (int i = idx + 1; i < orderedPathEvents.size(); ++i) {
                if (orderedPathEvents[i] == nullptr) continue;
                --(orderedPathEvents[i]->index);
            }
            orderedPathEvents.erase(orderedPathEvents.begin() + idx);
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const Path& path) {
        for (auto pathEvent : path.orderedPathEvents) {
            if (pathEvent != nullptr) os << *pathEvent << "; ";
        }
        return os;
    }

    class PathReconciler
    {
    public:
        static bool reconcile(std::unordered_set<Path *> paths);
    };

}

#endif
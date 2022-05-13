#ifndef PATHRECONCILER
#define PATHRECONCILER

#include <iostream>
#include <math.h>
#include <unordered_set>
// #include <ctime>
#include <queue>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>

#include "base/spatial.h"
#include "network/network.h"

namespace world
{

    class Path;

    struct PathEvent
    {
        Path *path;
        int index;
        std::vector<Location> locations;
        double timeAtPoint;

        PathEvent(Path *path, int idx, std::vector<Location> locs, double timeAtPoint)
            : path{path}, index{idx}, locations{locs}, timeAtPoint{timeAtPoint} {};

        void Delay(double amount) ;
    };

    inline std::ostream& operator<<(std::ostream& os, const PathEvent& pathEvent) {
        os << pathEvent.index << ": ";
        for (auto loc : pathEvent.locations) {
            os << loc << " ";
        }
        return os << "T" << pathEvent.timeAtPoint;
    }

    inline std::string to_string(const PathEvent& v) {
        std::stringstream strm; strm << v; return strm.str();
    }

    class Path
    {
    public:
        std::vector<PathEvent *> orderedPathEvents;

        PathEvent *Append(std::vector<Location> locs, double timeAtPoint) {
            auto event = new PathEvent(this, orderedPathEvents.size(), locs, timeAtPoint);
            orderedPathEvents.push_back(event);
            return event;
        }

        void Remove(int idx) {
            for (int i = idx + 1; i < orderedPathEvents.size(); ++i) {
                --(orderedPathEvents[i]->index);
            }
            orderedPathEvents.erase(orderedPathEvents.begin() + idx);
        }

        ~Path() {
            for (auto event : orderedPathEvents) delete event;
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const Path& path) {
        for (auto pathEvent : path.orderedPathEvents) {
            os << *pathEvent << "; ";
        }
        return os;
    }

    inline std::string to_string(const Path& v) {
        std::stringstream strm; strm << v; return strm.str();
    }

    class PathReconciler
    {
    public:
        static bool Reconcile(const std::unordered_set<Path *> paths);
    };

}

#endif
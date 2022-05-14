#ifndef VEHICLEPATHCONSTRUCTOR
#define VEHICLEPATHCONSTRUCTOR

#include "base/spatial.h"
#include "network/network.h"
#include "pathing/pathreconciler.h"
#include "pathing/pathfinder.h"
#include "spdlog/spdlog.h"

namespace world {

    class VehiclePathConstructor {
    public:
        Path *path;
        double currentTime;
        double timeInterval;

        VehiclePathConstructor(
            const Network &network,
            std::pair<CommercialBuilding *, ResidentialBuilding *> visit,
            double startTime,
            double timeInterval
        );
        ~VehiclePathConstructor();

    private:
        void appendEventsThrough(Location structureFrom, Location structureAt, Location structureTo);
        void appendEvent(std::vector<Location> locations);
    };

}

#endif

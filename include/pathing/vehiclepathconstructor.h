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
        static Path *Construct(
            const Network &network,
            int id,
            std::pair<CommercialBuilding *, ResidentialBuilding *> visit,
            double startTime,
            double timeInterval
        );
    };

}

#endif

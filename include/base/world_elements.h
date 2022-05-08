#ifndef WORLD_ELEMENTS
#define WORLD_ELEMENTS

#include <iostream>
#include <math.h>
#include <vector>
#include <unordered_map>

#include "base/spatial.h"

namespace world {

    class Network;

    class WorldElement
    {
    private:
        Location location;

    public:
        WorldElement(Location loc);
        const Location &getLocation() const;

        inline const static std::string Type = "WorldElement";
        virtual std::string getType() const;

        friend class Network;
    };

    class Building : public WorldElement
    {
    private:
        // Convention is that the location of the building is the bottom left
        // and the size indicates how many units to the right and above that the building covers
        int sizeX, sizeY;

    public:
        Building(std::pair<int, int> size, Location loc);

        virtual int getCurrentOccupancy();
        virtual int getOccupancyCapacity();

        virtual void addOccupant(){};
        virtual void removeOccupant(){};

        inline const static std::string Type = "Building";
        std::string getType() const override;

        friend class Network;
    };

    class CommercialBuilding : public Building
    {
    private:
        int numVisitors, currentVisitors;

    public:
        CommercialBuilding(int numOccupants, std::pair<int, int> size, Location loc);

        int getCurrentOccupancy() override;
        int getOccupancyCapacity() override;

        void addOccupant() override;
        void removeOccupant() override;

        inline const static std::string Type = "CommercialBuilding";
        std::string getType() const override;

        friend class Network;
    };

    class ResidentialBuilding : public Building
    {
    private:
        int numResidents, currentResidents;

    public:
        ResidentialBuilding(int numOccupants, std::pair<int, int> size, Location loc);

        int getCurrentOccupancy() override;
        int getOccupancyCapacity() override;

        void addOccupant() override;
        void removeOccupant() override;

        inline const static std::string Type = "ResidentialBuilding";
        std::string getType() const override;

        friend class Network;
    };

    class Roadway : public WorldElement
    {
    public:
        Roadway(Location loc);

        inline const static std::string Type = "Roadway";
        std::string getType() const override;

        friend class Network;
    };

    class RoadSegment : public Roadway
    {
    private:
        u_int8_t singleDirectionality = BiDirectionality::NO_BIDIR;

        inline const static std::string Type = "RoadSegment";
        std::string getType() const override;


    public:
        friend class Network;
    };

    class RoadJunction : public Roadway
    {
    private:
        u_int8_t directionalities = BiDirectionality::NO_BIDIR;

        inline const static std::string Type = "RoadJunction";
        std::string getType() const override;


    public:
        bool isValidDirectionality(BiDirectionality directionality);

        friend class Network;
    };
}

#endif
#pragma once

#include <map>
#include <memory>
#include <vector>

#include "../help_structs/entities.h"

class HeuristicModel {
public:
    explicit HeuristicModel(const std::shared_ptr<std::vector<Aircraft>>& aircrafts);

    void SetAircraftToFlight(int aircraft, const Flight& flight);
    void RemoveAircraftFromFlight(int aircraft, const Flight& flight);

    double GetTotalFine() const;

private:
    class FlightsAtTimes {
    public:
        FlightsAtTimes() = default;

        void SetAircraftToFlight(int aircraft, const Flight& flight);
        void RemoveAircraftFromFlight(int aircraft, const Flight& flight);

        int GetTotalFine() const;

    private:
        std::vector<std::vector<int>> flights_number_;
        int fine_{0};
    };

    class AircraftEndPoints {  // describes 1 aircraft
    public:
        AircraftEndPoints() = default;

        void SetFlight(const Flight& flight);
        void RemoveFlight(const Flight& flight);

        int GetTotalFine() const;

    private:
        enum class FlightStage {
            kArrival = 0,
            kDeparture = 1,
        };

        struct EndPoint {
            int time_point;
            FlightStage stage;

            bool operator<(const EndPoint& other) const;
        };

        struct AirportAndFine {
            int airport;
            int fine;
        };

    private:
        bool HasConflict(std::multimap<HeuristicModel::AircraftEndPoints::EndPoint,
                                       HeuristicModel::AircraftEndPoints::AirportAndFine>::iterator left,
                         std::multimap<HeuristicModel::AircraftEndPoints::EndPoint,
                                       HeuristicModel::AircraftEndPoints::AirportAndFine>::iterator right);

        std::multimap<EndPoint, AirportAndFine>::iterator GetPrev(
            std::multimap<EndPoint, AirportAndFine>::iterator it);
        std::multimap<EndPoint, AirportAndFine>::iterator GetNext(
            std::multimap<EndPoint, AirportAndFine>::iterator point);

        void InsertTimePoint(const EndPoint& point, int airport);
        void RemoveTimePoint(std::multimap<EndPoint, AirportAndFine>::iterator point);

    private:
        std::multimap<EndPoint, AirportAndFine> end_points_;
        int mismatches_number_{0};
    };

private:
    static double AIRPORTS_MISMATCH_FINE;
    static double FLIGHTS_INTERSECTION_FINE;

private:
    std::shared_ptr<std::vector<Aircraft>> aircrafts_;
    double seats_fine_{0};
    FlightsAtTimes flights_at_times_;
    std::vector<AircraftEndPoints> end_points_;
};

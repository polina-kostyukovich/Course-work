#pragma once

#include <map>
#include <memory>
#include <vector>

#include "../help_structs/entities.h"

class HeuristicModel {
public:
    struct ModelState {
        double fine;
        SolutionCorrectnessInfo correctness_info;
    };

public:
    HeuristicModel() = default;
    HeuristicModel(const std::shared_ptr<std::vector<Aircraft>>& aircrafts,
                   const std::shared_ptr<std::vector<Airport>>& airports,
                   int hours_in_cycle,
                   const std::vector<int>& time_points);

    void SetAircraftToFlight(int aircraft, const Flight& flight);
    void RemoveAircraftFromFlight(int aircraft, const Flight& flight);

    ModelState GetTotalFine() const;

private:
    class FlightsAtTimes {
    public:
        FlightsAtTimes() = default;
        FlightsAtTimes(int aircrafts_number, int time_points_number);

        void SetAircraftToFlight(int aircraft, const Flight& flight);
        void RemoveAircraftFromFlight(int aircraft, const Flight& flight);

        void Print() const;

        int GetTotalFine() const;

    private:
        std::vector<std::vector<int>> flights_number_;
        int fine_{0};
    };

    class AircraftEndPoints {  // describes 1 aircraft
    public:
         AircraftEndPoints(const std::shared_ptr<std::vector<Airport>>& airports,
                           int hours_in_cycle,
                           const std::vector<int>& time_points)
            : airports_(airports), hours_in_cycle_(hours_in_cycle), time_points_(time_points) {}

        void SetFlight(const Flight& flight);
        void RemoveFlight(const Flight& flight);

        void Print() const;

        int GetTotalFine() const;
        int GetStayCost() const;

    private:
        enum class FlightStage {
            kArrival = 0,
            kDeparture = 1,
        };

        struct EndPoint {
            int time_point;
            int airport;
            FlightStage stage;

            bool operator<(const EndPoint& other) const;

            std::string ToStr() const;
        };

    private:
        bool HasStay(std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator left,
                     std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator right);
        bool HasConflict(std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator left,
                         std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator right);

        std::multimap<EndPoint, int>::iterator GetPrev(
            std::multimap<EndPoint, int>::iterator it);
        std::multimap<EndPoint, int>::iterator GetNext(
            std::multimap<EndPoint, int>::iterator point);

        void InsertTimePoint(const EndPoint& point);
        void RemoveTimePoint(std::multimap<EndPoint, int>::iterator point);

        int GetStayTime(std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator left,
                        std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator right);

    private:
        std::multimap<EndPoint, int> end_points_;
        std::shared_ptr<std::vector<Airport>> airports_;
        std::vector<int> time_points_;
        int mismatches_number_{0};
        int stay_cost_{0};
        int hours_in_cycle_;
    };

    class FlightsCost {
    public:
        FlightsCost() = default;
        explicit FlightsCost(const std::shared_ptr<std::vector<Aircraft>>& aircrafts);

        void SetAircraftToFlight(int aircraft, const Flight& flight);
        void RemoveAircraftFromFlight(int aircraft, const Flight& flight);

        int GetTotalFine() const;

    private:
        int fine_{0};
        std::shared_ptr<std::vector<Aircraft>> aircrafts_;
    };

private:
    std::shared_ptr<std::vector<Aircraft>> aircrafts_;
    double seats_fine_{0};
    FlightsAtTimes flights_at_times_;
    std::vector<AircraftEndPoints> end_points_;
    FlightsCost flights_cost_;
};

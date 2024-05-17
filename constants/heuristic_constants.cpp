#include "heuristic_constants.h"

namespace constants {

const int LAUNCHES_NUMBER = 5;

const int ITERATIONS_NUMBER = 10'000'000;

const double EPSILON = 1e-7;  // a little smaller than FLIGHTS_COST and STAY_COST

const double NO_AIRCRAFT_FINE = 1e9;

const double FLIGHTS_INTERSECTION_FINE = 1e6;
const double AIRPORTS_MISMATCH_FINE = 1e6;
const double SEATS_FINE = 1;
const double FLIGHTS_COST = 1e-6;
const double STAY_COST = 1e-6;

}

% params declaration
int: start_time;
int: finish_time;
int: hour_size;

int: flights_number;
int: aircrafts_number;
int: airports_number;

array[1..flights_number] of var int: flights_departure_points;
array[1..flights_number] of var int: flights_arrival_points;
array[1..flights_number] of var int: flights_departure_times;
array[1..flights_number] of var int: flights_arrival_times;
array[1..flights_number] of var int: flights_distances;
array[1..flights_number] of var int: flights_min_passangers;

array[1..aircrafts_number] of var int: aircrafts_seats;
array[1..aircrafts_number] of var int: aircrafts_costs;

array[1..airports_number] of var int: airports_costs;


% variables
array[1..flights_number] of var 1..aircrafts_number: aircrafts;
array[1..flights_number] of var 0..flights_number: next_flights;
array[1..aircrafts_number] of var 0..flights_number: first_flights;
array[1..aircrafts_number] of var 0..flights_number: last_flights;
% 0 - no such object


% help functions
function var 0..flights_number: next_flight(var 1..flights_number: index) =
    let {
        var set of 1..flights_number: searchables;
%         constraint forall(value in searchables)(
%             (value > index) /\ (aircrafts[value] = aircrafts[index])
%         );
        constraint forall(i in (index+1)..flights_number)(
            i in searchables xor aircrafts[i] != aircrafts[index]
        );
        var 0..flights_number: answer;
        constraint if searchables = {} then answer = 0 else answer = min(searchables) endif;
    } in answer;

function var 0..flights_number: first_flight(var 1..aircrafts_number: aircraft) =
    let {
        var set of 1..flights_number: aircraft_flights;
        constraint forall(flight in 1..flights_number)(
            flight in aircraft_flights xor aircrafts[flight] != aircraft
        );
        var 0..flights_number: answer;
        constraint if aircraft_flights = {} then answer = 0 else answer = min(aircraft_flights) endif;
    } in answer;

function var 0..flights_number: last_flight(var 1..aircrafts_number: aircraft) =
    let {
        var set of 1..flights_number: aircraft_flights;
        constraint forall(flight in 1..flights_number)(
            flight in aircraft_flights xor aircrafts[flight] != aircraft
        );
        var 0..flights_number: answer;
        constraint if aircraft_flights = {} then answer = 0 else answer = max(aircraft_flights) endif;
    } in answer;


% constraints
constraint forall(i in 1..flights_number)(
    next_flights[i] = next_flight(i)
);

constraint forall(j in 1..aircrafts_number)(
    first_flights[j] = first_flight(j)
);

constraint forall(j in 1..aircrafts_number)(
    last_flights[j] = last_flight(j)
);


constraint forall(i in 1..flights_number)(
    flights_min_passangers[i] <= aircrafts_seats[aircrafts[i]]
);

constraint forall(i in 1..flights_number)(
    next_flights[i] > 0 -> flights_arrival_times[i] <= flights_departure_times[next_flights[i]]
);

constraint forall(i in 1..flights_number)(
    next_flights[i] > 0 -> flights_arrival_points[i] = flights_departure_points[next_flights[i]]
);

constraint forall(j in 1..aircrafts_number)(
    flights_arrival_points[last_flights[j]] = flights_departure_points[first_flights[j]]
);


% help functions for target function
function var int: second_summand(var 1..flights_number: i, var 1..aircrafts_number: j) =
    if next_flights[i] = 0 then 0 else bool2int(aircrafts[i] = j) * (flights_departure_times[next_flights[i]] - flights_arrival_times[i] + 2 * hour_size) * airports_costs[flights_arrival_points[i]] endif;

function var int: third_summand(var 1..aircrafts_number: j) =
    if last_flights[j] = 0 then 0 else (finish_time - flights_arrival_times[last_flights[j]] + flights_departure_times[first_flights[j]] - start_time + 2 * hour_size) * airports_costs[flights_departure_points[first_flights[j]]] endif;


% % target function
% solve minimize sum(i in 1..flights_number)(
%     flights_distances[i] * aircrafts_costs[aircrafts[i]]
% ) + sum(j in 1..aircrafts_number)(
%     sum(i in 1..(last_flights[j]-1))(
%         second_summand(i, j)
%     )
% ) + sum(j in 1..aircrafts_number)(
%     third_summand(j)
% );

solve :: seq_search([
    int_search(aircrafts, dom_w_deg, indomain_random),
%     int_search(next_flights, input_order, indomain_median),
%     int_search(first_flights, input_order, indomain_median),
%     int_search(last_flights, input_order, indomain_median),
])
% :: restart_linear(10 + flights_number)
% minimize sum(i in 1..flights_number)(
%     flights_distances[i] * aircrafts_costs[aircrafts[i]]
% ) + sum(j in 1..aircrafts_number)(
%     sum(i in 1..(last_flights[j]-1))(
%         second_summand(i, j)
%     )
% ) + sum(j in 1..aircrafts_number)(
%     third_summand(j)
% );
satisfy;

% solve satisfy;

% output
output [show(aircrafts), show(next_flights), show(first_flights), show(last_flights)];

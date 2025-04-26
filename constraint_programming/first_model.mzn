include "partition_set.mzn";

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
array[1..aircrafts_number] of var set of 1..flights_number: flights;


% help functions
function var 0..flights_number: set_size(var set of 1..flights_number: values) = sum(i in values)(1);

function var 1..flights_number: set_value_by_index(var set of 1..flights_number: values, var 1..flights_number: index) = 
  let { var 1..flights_number: value;
        constraint value in values;
        constraint sum(item in values where item < value)(1) = index-1; } in value;


% constraints
%partition
constraint partition_set(flights, 1..flights_number);

% help but not necessary constraint
constraint forall(j in 1..aircrafts_number)(
    set_size(flights[j]) != 1
);

constraint forall(j in 1..aircrafts_number)(
    forall(flight in flights[j])(
        flights_min_passangers[flight] <= aircrafts_seats[j]
    )
);

constraint forall(j in 1..aircrafts_number)(
    forall(flight1, flight2 in flights[j] where flight1 < flight2)(
        flights_arrival_times[flight1] <= flights_departure_times[flight2]
    )
);

constraint forall(j in 1..aircrafts_number)(
    forall(k in 1..(set_size(flights[j])-1))(
        flights_arrival_points[set_value_by_index(flights[j], k)] = flights_departure_points[set_value_by_index(flights[j], k+1)]
    )
);

constraint forall(j in 1..aircrafts_number)(
    flights_arrival_points[set_value_by_index(flights[j], set_size(flights[j]))] = flights_departure_points[set_value_by_index(flights[j], 1)]
);


% % target function
% array[1..aircrafts_number] of var int: stay_costs;

% constraint forall(j in 1..aircrafts_number)(
%     stay_costs[j] = sum(k in 1..(set_size(flights[j])-1))(
%         (flights_departure_times[set_value_by_index(flights[j], k+1)] - flights_arrival_times[set_value_by_index(flights[j], k)] + 2 * hour_size) * airports_costs[flights_arrival_points[set_value_by_index(flights[j], k)]]
%     ) + 
%     (finish_time - flights_arrival_times[set_value_by_index(flights[j], set_size(flights[j]))] + flights_departure_times[set_value_by_index(flights[j], 1)] - start_time + 2 * hour_size) * airports_costs[flights_departure_points[set_value_by_index(flights[j], 1)]]
% );

% solve minimize sum(j in 1..aircrafts_number)(
%     sum(i in flights[j])(
%         flights_distances[i] * aircrafts_costs[j]
%     )
% ) + sum(j in 1..aircrafts_number)(stay_costs[j]);


solve satisfy;

% output
output [show(flights)]

include "partition_set.mzn";
include "globals.mzn";
include "all_different_except_0.mzn";

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
array[1..aircrafts_number,1..flights_number] of var 0..flights_number: flights;
array[1..aircrafts_number] of var 1..flights_number + 1: start;
array[1..aircrafts_number] of var 0..flights_number: set_size;


% help functions
% function var 0..flights_number: set_size(array[1..flights_number] of 0..flights_number: values) = flights_number - count(values, 0);


% constraints
constraint forall(j in 1..aircrafts_number)( set_size[j] = flights_number - sum(i in 1..flights_number)(bool2int(flights[j, i] != 0)) );

constraint forall(j in 1..aircrafts_number)(
    (set_size[j] = flights_number /\ start[j] = 1) \/ 
    (set_size[j] = 0 /\ start[j] = flights_number + 1) \/ 
    (flights[j, start[j] - 1] = 0 /\ flights[j, start[j]] != 0)
);

%partition
constraint all_different_except_0(flights);

constraint count(flights, 0, (aircrafts_number - 1) * flights_number);

constraint forall(j in 1..aircrafts_number)( forall(i in 1..(flights_number - 1))( flights[j, i] <= flights[j, i + 1] ) );

% help but not necessary constraint
constraint redundant_constraint( forall(j in 1..aircrafts_number)( set_size[j] != 1 ) );

constraint forall(j in 1..aircrafts_number)(
    forall(i in start[j]..flights_number)(
        flights_min_passangers[flights[j, i]] <= aircrafts_seats[j]
    )
);

constraint forall(j in 1..aircrafts_number)(
    forall(i in start[j]..(flights_number - 1))(
        flights_arrival_times[flights[j, i]] <= flights_departure_times[flights[j, i + 1]]
    )
);
    
constraint forall(j in 1..aircrafts_number)(
    forall(i in start[j]..(flights_number - 1))(
        flights_arrival_points[flights[j, i]] = flights_departure_points[flights[j, i + 1]]
    )
);

constraint forall(j in 1..aircrafts_number)(
    (start[j] != flights_number + 1) -> flights_arrival_points[flights[j, flights_number]] = flights_departure_points[flights[j, start[j]]]
);


% % target function variables
% array[1..aircrafts_number] of var int: stay_costs;

% % target function
% constraint forall(j in 1..aircrafts_number)(
%   set_size[j] > 0 ->
%     stay_costs[j] = sum(i in start[j]..(flights_number - 1))(
%         (flights_departure_times[flights[j, i + 1]] - flights_arrival_times[flights[j, i]] + 2 * hour_size) * airports_costs[flights_arrival_points[flights[j, i]]]
%     ) + 
%     (finish_time - flights_arrival_times[flights[j, flights_number]] + flights_departure_times[flights[j, start[j]]] - start_time + 2 * hour_size) * airports_costs[flights_departure_points[flights[j, start[j]]]]
% );


% % target function
% solve minimize sum(j in 1..aircrafts_number)(
%     sum(i in start[j]..flights_number)(
%         flights_distances[flights[j, i]] * aircrafts_costs[j]
%     )
% ) + sum(j in 1..aircrafts_number)(stay_costs[j]);

solve satisfy;

% output
output [show(flights)]

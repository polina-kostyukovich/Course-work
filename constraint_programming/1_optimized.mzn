include "partition_set.mzn";
include "globals.mzn";

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

% target function variables
array[1..aircrafts_number] of var int: stay_costs;


% help functions
function var 0..flights_number: set_size(var set of 1..flights_number: values) = sum(i in values)(1);

% function var 1..flights_number: set_value_by_index(var set of 1..flights_number: values, var 1..flights_number: index) = 
%   let { var 1..flights_number: value;
%         constraint value in values;
%         constraint sum(item in values where item < value)(1) = index-1; } in value;


% constraints
%partition
constraint partition_set(flights, 1..flights_number);  % 5, 6

% help but not necessary constraint
constraint redundant_constraint( forall(j in 1..aircrafts_number)( set_size(flights[j]) != 1 ) );

constraint forall(j in 1..aircrafts_number)(
    forall(flight in flights[j])(
        flights_min_passangers[flight] <= aircrafts_seats[j]
    )
);

constraint forall(j in 1..aircrafts_number)( % optimize?
    forall(flight1, flight2 in flights[j] where flight1 < flight2)(
        flights_arrival_times[flight1] <= flights_departure_times[flight2]
    )
);


predicate constraints_with_sorted_set(var set of 1..flights_number: values, var int: target_coef) =
  let {
    var 0..flights_number: values_size = set_size(values);
    array[1..flights_number] of var 0..flights_number: values_array;
    
    constraint forall(value in values)( value in values_array );
    
    constraint count(values_array, 0, flights_number - values_size);
    
    constraint increasing(values_array);
    
    var 1..flights_number + 1: start;
    
    constraint (values_size = flights_number /\ start = 1) \/ (values_size = 0 /\ start = flights_number + 1) \/ (values_array[start - 1] = 0 /\ values_array[start] != 0);
    
    var bool: constraint1 = forall(k in start..(flights_number - 1))(
        flights_arrival_points[values_array[k]] = flights_departure_points[values_array[k + 1]]
    );
    
    var bool: constraint2 = flights_arrival_points[values_array[flights_number]] = flights_departure_points[values_array[start]];
    
%     % target function
%     var bool: target_constraint = (target_coef = sum(k in start..(flights_number - 1))(
%         (flights_departure_times[values_array[k + 1]] - flights_arrival_times[values_array[k]] + 2 * hour_size) * airports_costs[flights_arrival_points[values_array[k]]]
%       ) + 
%         (finish_time - flights_arrival_times[values_array[flights_number]] + flights_departure_times[values_array[start]] - start_time + 2 * hour_size) * airports_costs[flights_departure_points[values_array[start]]]
%     );
%   } in constraint1 /\ constraint2 /\ target_constraint;
  } in constraint1 /\ constraint2;


constraint forall(j in 1..aircrafts_number)(
    constraints_with_sorted_set(flights[j], stay_costs[j])
);


% % target function
% solve minimize sum(j in 1..aircrafts_number)(
%     sum(i in flights[j])(
%         flights_distances[i] * aircrafts_costs[j]
%     )
% ) + sum(j in 1..aircrafts_number)(stay_costs[j]);

solve satisfy;


% output
output [show(flights)]

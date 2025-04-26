cmake_minimum_required(VERSION 3.24)
project(aircrafts)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")

set(CMAKE_PREFIX_PATH /opt/homebrew/Cellar/poco/1.14.1/lib/cmake/Poco/)

find_package(Poco REQUIRED JSON)
#find_package(HIGHS REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED)

include_directories(/opt/homebrew/Cellar/poco/1.14.1/lib/cmake/Poco/include/)

add_executable(vertices_degrees_generation
        data_generation/vertices_degrees_generation.cpp
        help_structs/entities.cpp
)

add_executable(graph_generation
        data_generation/graph_generation.cpp
)

add_executable(data_generation
        data_generation/data_generation.cpp
        help_structs/entities.cpp
)

add_executable(generate_all_data
        generate_data.cpp
        help_structs/entities.cpp
)

add_executable(bad_data_generation
        data_generation/bad_data_generation.cpp
        help_structs/entities.cpp
)

add_executable(generate_all_bad_data
        generate_bad_data.cpp
        help_structs/entities.cpp
)

#add_executable(ilp_solution
#        ilp_solution.cpp
#        help_structs/row_wise_matrix.cpp
#        help_structs/entities.cpp
#        help_structs/indices_converter.cpp
#        help_structs/data_reader.cpp
#        help_structs/data_processor.cpp
#)

#add_executable(local_search_solution
#        local_search_solution.cpp
#        heuristic_model/heuristic_model.cpp
#        heuristic_model/local_search_solver.cpp
#        constants/heuristic_constants.cpp
#        help_structs/data_reader.cpp
#        help_structs/data_processor.cpp
#        help_structs/entities.cpp
#)

#add_executable(simulated_annealing_solution
#        simulated_annealing_solution.cpp
#        heuristic_model/heuristic_model.cpp
#        heuristic_model/simulated_annealing_solver.cpp
#        heuristic_model/simulated_annealing_solver.cpp
#        constants/heuristic_constants.cpp
#        help_structs/data_reader.cpp
#        help_structs/data_processor.cpp
#        help_structs/entities.cpp
#        help_structs/temperature_functions.cpp
#)

#add_executable(all_methods_analysis
#        analysis/all_methods.cpp
#        constants/heuristic_constants.cpp
#)

#add_executable(temperature_functions_comparison
#        analysis/temperature_functions_comparison.cpp
#        constants/heuristic_constants.cpp
#)

#add_executable(choosing_constants
#        analysis/choosing_constants.cpp
#        constants/heuristic_constants.cpp
#)

#add_executable(tf_sim_annealing
#        analysis/tf_sim_annealing.cpp
#        constants/heuristic_constants.cpp
#)

#add_executable(tf_local_search
#        analysis/tf_local_search.cpp
#        constants/heuristic_constants.cpp
#)

add_executable(convert_data_for_cp
        constraint_programming/convert_data.cpp
        help_structs/data_reader.cpp
        help_structs/entities.cpp)

add_executable(convert_multiple_data_for_cp
        constraint_programming/convert_multiple_files.cpp
)

#add_executable(cp_testing
#        constraint_programming/testing.cpp
#)

#add_executable(testing
#        testing.cpp
#)

target_link_libraries(
        vertices_degrees_generation
        PRIVATE Poco::JSON
)

target_link_libraries(data_generation
        PRIVATE Poco::JSON
)

target_link_libraries(generate_all_data
        PRIVATE Poco::JSON
)

target_link_libraries(bad_data_generation
        PRIVATE Poco::JSON
)

target_link_libraries(generate_all_bad_data
        PRIVATE Poco::JSON
)

#target_link_libraries(ilp_solution
#        PRIVATE Poco::JSON
#        highs::highs
#)

#target_link_libraries(testing
#        PRIVATE Poco::JSON
#)

#target_link_libraries(local_search_solution
#        PRIVATE Poco::JSON
#)
#
#target_link_libraries(simulated_annealing_solution
#        PRIVATE Poco::JSON
#)

target_link_libraries(convert_data_for_cp
        PRIVATE Poco::JSON
)

target_link_libraries(convert_multiple_data_for_cp
        PRIVATE Poco::JSON
)

#target_link_libraries(cp_testing
#        PRIVATE Poco::JSON
#)

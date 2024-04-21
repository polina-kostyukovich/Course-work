cmake_minimum_required(VERSION 3.26)
project(aircrafts)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,leak,undefined")

set(CMAKE_PREFIX_PATH /home/<user>/libs/poco/lib/cmake/Poco)

find_package(Poco REQUIRED JSON)
find_package(HIGHS REQUIRED)
find_package(Threads REQUIRED)

add_executable(vertices_degrees_generation data_generation/vertices_degrees_generation.cpp)
add_executable(graph_generation data_generation/graph_generation.cpp)
add_executable(data_generation data_generation/data_generation.cpp help_structs/entities.cpp)
add_executable(solution solution.cpp help_structs/row_wise_matrix.cpp
        help_structs/entities.cpp help_structs/indices_converter.cpp)
add_executable(heuristic_solution heuristic_solution.cpp heuristic_model/heuristic_model.cpp
        heuristic_model/local_search_solver.cpp heuristic_model/simulated_annealing_solver.cpp
        constants/heuristic_constants.cpp)
add_executable(testing testing.cpp)

target_link_libraries(
        vertices_degrees_generation
        PRIVATE Poco::JSON
)
target_link_libraries(
        data_generation
        PRIVATE Poco::JSON
)
target_link_libraries(solution
        PRIVATE Poco::JSON
        highs::highs
)
target_link_libraries(testing
        PRIVATE Poco::JSON
)
target_link_libraries(heuristic_solution
        PRIVATE Poco::JSON
        highs::highs
)

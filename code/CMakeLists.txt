cmake_minimum_required(VERSION 3.26)
project(aircrafts)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,leak,undefined")

set(CMAKE_PREFIX_PATH /home/<user>/libs/poco/lib/cmake/Poco)

find_package(Poco REQUIRED JSON)

add_executable(vertices_degrees_generation data_generation/vertices_degrees_generation.cpp)
add_executable(graph_generation data_generation/graph_generation.cpp)
add_executable(data_generation data_generation/data_generation.cpp)

target_link_libraries(
        vertices_degrees_generation
        PRIVATE Poco::JSON
)
target_link_libraries(
        data_generation
        PRIVATE Poco::JSON
)

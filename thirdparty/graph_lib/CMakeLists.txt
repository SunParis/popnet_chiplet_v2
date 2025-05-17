# The min required cmake version is 3.5 before 
cmake_minimum_required(VERSION 3.10)
project(graph_lib)
aux_source_directory(. DIR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -shared -fPIC")

find_package(Boost REQUIRED COMPONENTS graph iostreams regex serialization system filesystem)

add_library(mygraph STATIC ${DIR})

target_link_libraries(mygraph PRIVATE ${Boost_LIBRARIES})

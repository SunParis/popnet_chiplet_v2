cmake_minimum_required(VERSION 3.10)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.30")
    cmake_policy(SET CMP0167 NEW)
endif()

project(popnet)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE RELEASE)

set(POWER_RELEASE "thirdparty/orion_power_beta_mar2003")
set(GRAPH_LIB "thirdparty/graph_lib")
set(FMT_LIB "thirdparty/fmt")
set(JSON_LIB "thirdparty/nlohmann_json")

add_subdirectory(${POWER_RELEASE}/power)
add_subdirectory(${GRAPH_LIB})
add_subdirectory(${FMT_LIB})
add_subdirectory(${JSON_LIB})

set(CMAKE_CXX_FLAGS "-Wno-deprecated -g -DPOWER_TEST")# -O3")
include_directories(includes)
include_directories(${POWER_RELEASE}/power ${POWER_RELEASE}/library)
include_directories(${GRAPH_LIB})
include_directories(${FMT_LIB}/include)
include_directories(${JSON_LIB}/include)

file(GLOB_RECURSE C_SOURCES "srcs/*.c")
file(GLOB_RECURSE CXX_SOURCES "srcs/*.cpp")
file(GLOB_RECURSE CC_SOURCES "srcs/*.cc")
set(SIM_SRCS ${C_SOURCES} ${CXX_SOURCES} ${CC_SOURCES})

link_directories(${POWER_RELEASE}/power)

find_package(Boost REQUIRED COMPONENTS graph)

add_executable(popnet ${SIM_SRCS})

target_link_libraries(popnet orion_power mygraph)
target_link_libraries(popnet ${Boost_LIBRARIES})
target_link_libraries(popnet fmt)
target_link_libraries(popnet nlohmann_json::nlohmann_json)
cmake_minimum_required(VERSION 3.20)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30")
    cmake_policy(SET CMP0167 NEW)
endif()

project(PopnetChiplet VERSION 2.0.0 LANGUAGES C CXX)

option(POPNET_BUILD_TESTS "Build the Popnet test suite" ON)
option(POPNET_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(thirdparty/orion_power_beta_mar2003/power)
add_subdirectory(thirdparty/graph_lib)
add_subdirectory(thirdparty/fmt)
add_subdirectory(thirdparty/nlohmann_json)

set(POPNET_CORE_SOURCES
    src/alg/shortest_path_routing.cpp
    src/global_defines/RGen.cpp
    src/global_defines/SStd.cpp
    src/global_defines/global.cpp
    src/global_defines/input_trace.cpp
    src/global_defines/message_define.cpp
    src/global_defines/packet_defines.cpp
    src/global_defines/proto_engine.cpp
    src/io/delay_writer.cpp
    src/preprocess/config.cpp
    src/router/base_router.cpp
    src/router/modules.cpp
    src/router/router.cpp
    src/router/topo_router.cpp
    src/sim/Sim.cpp
    src/sim/simulation_state.cpp
)

add_library(popnet_core STATIC ${POPNET_CORE_SOURCES})
add_library(Popnet::Core ALIAS popnet_core)

target_include_directories(popnet_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/popnet>
)

target_include_directories(popnet_core SYSTEM
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/orion_power_beta_mar2003/power
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/orion_power_beta_mar2003/library
)

target_compile_definitions(popnet_core
    PRIVATE POWER_TEST
)

target_compile_options(popnet_core PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>
)

target_link_libraries(popnet_core
    PUBLIC
        fmt::fmt
        mygraph
        nlohmann_json::nlohmann_json
    PRIVATE
        orion_power
)

if(POPNET_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(popnet_core PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(popnet_core PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

add_executable(popnet app/main.cpp)
target_link_libraries(popnet PRIVATE Popnet::Core)

if(POPNET_BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
endif()

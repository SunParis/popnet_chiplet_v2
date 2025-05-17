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

# Find Boost graph component (already included)
find_package(Boost REQUIRED COMPONENTS graph)

# Initialize flag to control whether to use Boost.Filesystem
set(USE_BOOST_FILESYSTEM OFF)

# Detect GCC compiler and check its version to decide filesystem implementation
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "GCC detected, version: ${CMAKE_CXX_COMPILER_VERSION}")
    string(REGEX MATCHALL "[0-9]+" GCC_VER_COMPONENTS ${CMAKE_CXX_COMPILER_VERSION})
    list(GET GCC_VER_COMPONENTS 0 GCC_MAJOR)
    list(GET GCC_VER_COMPONENTS 1 GCC_MINOR)

    # Use Boost.Filesystem for GCC versions less than 9
    if(GCC_MAJOR LESS 9)
        message(STATUS "GCC version < 9 detected, will use Boost.Filesystem")
        set(USE_BOOST_FILESYSTEM ON)
    endif()
endif()

# If Boost.Filesystem is required, find Boost filesystem and system components
if(USE_BOOST_FILESYSTEM)
    message(STATUS "Finding Boost Filesystem and System components")
    find_package(Boost REQUIRED COMPONENTS filesystem system)
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Boost not found but required for filesystem")
    endif()
endif()

add_executable(popnet ${SIM_SRCS})

# Use keyword form (PRIVATE) for all target_link_libraries calls
target_link_libraries(popnet PRIVATE orion_power mygraph)
target_link_libraries(popnet PRIVATE ${Boost_LIBRARIES})
target_link_libraries(popnet PRIVATE fmt)
target_link_libraries(popnet PRIVATE nlohmann_json::nlohmann_json)

if(USE_BOOST_FILESYSTEM)
    target_link_libraries(popnet PRIVATE ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
    target_compile_definitions(popnet PRIVATE USE_BOOST_FS)
else()
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles("
        #include <filesystem>
    " HAVE_STD_FILESYSTEM)

    if(HAVE_STD_FILESYSTEM)
        target_link_libraries(popnet PRIVATE stdc++fs)
    endif()

    target_compile_definitions(popnet PRIVATE USE_STD_FS)
endif()


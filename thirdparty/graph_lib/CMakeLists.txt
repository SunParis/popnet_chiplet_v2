find_package(Boost 1.74 REQUIRED COMPONENTS graph iostreams regex serialization filesystem)

add_library(mygraph STATIC graph.cpp)

target_compile_features(mygraph PUBLIC cxx_std_20)
target_include_directories(mygraph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mygraph
    PUBLIC
        Boost::headers
        Boost::graph
        Boost::iostreams
        Boost::regex
        Boost::serialization
        Boost::filesystem
)

target_compile_options(mygraph PRIVATE -Wall -Wextra -Wpedantic)

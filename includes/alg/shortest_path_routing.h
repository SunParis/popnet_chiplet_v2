# pragma once

/**
 * @file shortest_path_routing.h
 * @brief The header file for shortest path routing.
 */

# ifndef _SHORTEST_PATH_ROUTING_H_
# define _SHORTEST_PATH_ROUTING_H_ 1


# include <boost/multi_array.hpp>
# include <cmath>
# include <limits>


# include "graph.h"
# include "global.h"


# define PIPELINE_STAGE_DELAY_0 PIPE_DELAY_
# define PIPELINE_STAGE_NUMBER 5
# define ROUTING_DELAY_0 (PIPELINE_STAGE_DELAY_0 * PIPELINE_STAGE_NUMBER)
# define LINK_DELAY_0 WIRE_DELAY_



namespace ShortestPath {

using TMatrix = boost::multi_array<double, 2>;
using TDelayMatrix = TMatrix;
using TIntMatrix = boost::multi_array<int, 2>;
using TNextHopTable = TIntMatrix;

/**
 * @brief Reset the matrix with integer elements.
 * @param table The matrix to be reset.
 * @param n The size of the matrix.
 * @param value The value to be filled in the matrix.
 */
void resetIntMatrix(TIntMatrix& table, size_t n, int value);


/**
 * @brief Reset the matrix with double elements.
 * @param table The matrix to be reset.
 * @param n The size of the matrix.
 * @param value The value to be filled in the matrix.
 */
void resetMatrix(TMatrix& matrix, size_t n, double value);


/**
 * @brief calculate the shortest path tables.
 * @param g The graph.
 * @param delayTable The delay table.
 * @param energyTable The energy table.
 * @param nextHop The next hop table.
 */
void calculateShortestPathTables(GraphLib::graph_t& g, TMatrix& delayTable, TMatrix& energyTable, TNextHopTable& nextHop);

};

# endif

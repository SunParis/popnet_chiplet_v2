# include "alg/shortest_path_routing.h"


/**
 * @brief Calculate the routing delay of a router.
 * @param propertyMap The property map of the graph.
 * @param v The vertex.
 * @return The routing delay.
 * @tparam PM The type of the property map.
 * @tparam V The type of the vertex.
 */
template<typename PM, typename V>
inline double routing_delay(PM& propertyMap, V& v) {
    return propertyMap[v] * PIPELINE_STAGE_NUMBER;
}


/**
 * @brief Calculate the link delay of an edge.
 * @param propertyMap The property map of the graph.
 * @param e The edge.
 * @return The link delay.
 * @tparam PM The type of the property map.
 * @tparam E The type of the edge.
 */
template<typename PM, typename E>
inline double link_delay(PM& propertyMap, E& e) {
    return propertyMap[e];
}


/**
 * @brief Set the value of the next hop table.
 * @param table The next hop table.
 * @param x The x coordinate.
 * @param y The y coordinate.
 * @param x_next The next hop of x.
 * @param y_next The next hop of y.
 */
inline void setDoubleHop(ShortestPath::TNextHopTable& table, std::size_t x, std::size_t y, int x_next, int y_next) {
    table[x][y] = x_next;
    table[y][x] = y_next;
}


/**
 * @brief Set the value of the matrix.
 * @param delayTable The delay table.
 * @param x The x coordinate.
 * @param y The y coordinate.
 * @param delay The delay value.
 */
inline void setDoubleValue(ShortestPath::TMatrix& delayTable, std::size_t x, std::size_t y, double delay) {
    delayTable[x][y] = delay;
    delayTable[y][x] = delay;
}


void ShortestPath::resetIntMatrix(ShortestPath::TIntMatrix& table, std::size_t n, int value) {
    table.resize(boost::extents[n][n]);
    
    for (std::size_t i = 0; i < n; ++i)
        std::fill(table[i].begin(), table[i].end(), value);
}


void ShortestPath::resetMatrix(ShortestPath::TMatrix& matrix, std::size_t n, double value) {
    matrix.resize(boost::extents[n][n]);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            matrix[i][j] = (i == j) ? 0: value;
        }
    }
}


void ShortestPath::calculateShortestPathTables(GraphLib::graph_t& g,
    ShortestPath::TMatrix& delayTable, ShortestPath::TMatrix& energyTable,
    ShortestPath::TNextHopTable& nextHop)
{
    
    const double INF_WEIGHT = std::numeric_limits<double>::infinity();
    
    std::size_t n = boost::num_vertices(g);
    
    resetIntMatrix(nextHop, n, -1);
    
    for (std::size_t i = 0; i < n; i++)
        nextHop[i][i] = i;
    
    resetMatrix(delayTable, n, INF_WEIGHT);
    resetMatrix(energyTable, n, INF_WEIGHT);
    
    // Initialize the delay and energy of the routers
    auto index = boost::get(&GraphLib::vertex_info::index, g);
    auto vertexList = boost::vertices(g);
    auto forwardEnergyMap = boost::get(&GraphLib::vertex_info::energyPerForwarding, g);
    auto pipelineStageDelay = boost::get(&GraphLib::vertex_info::pipelineStageDelay, g);
    std::vector<double> forwardEnergy(n);
    std::vector<double> routerDelay(n);
    std::for_each(vertexList.first, vertexList.second, [&](const GraphLib::vertex_t& v){
        std::size_t idx = index[v];
        forwardEnergy[idx] = forwardEnergyMap[v];
        routerDelay[idx] = routing_delay(pipelineStageDelay, v);
    });
    
    // Initialize the delay and energy table
    auto edgeList = boost::edges(g);
    auto delay = boost::get(boost::edge_weight, g);
    auto hopEnergyMap = boost::get(boost::edge_weight2, g);
    std::for_each(edgeList.first, edgeList.second, [&](const GraphLib::edge_t& e){
        int src_index = index[boost::source(e, g)];
        int tar_index = index[boost::target(e, g)];
        
        // Assume that the router forwarding takes time (including the routers of the sender and receiver)
        setDoubleValue(delayTable, src_index, tar_index, link_delay(delay, e) + routerDelay[src_index] + routerDelay[tar_index]);
        setDoubleHop(nextHop, src_index, tar_index, tar_index, src_index);
        setDoubleValue(energyTable, src_index, tar_index, forwardEnergy[src_index] + hopEnergyMap[e] + forwardEnergy[tar_index]);
    });

    // Floyd-Warshall
    for (std::size_t k = 0; k < n; ++k){
        for (std::size_t i = 0; i < n; ++i){
            for (std::size_t j = 0; j < n; ++j){
                double t = delayTable[i][k] + delayTable[k][j];
                // Forwarding delay of router k is calculated twice, so it should be subtracted once
                if (i != j && j != k && i != k)
                    t -= routerDelay[k];
                
                if (t < delayTable[i][j]) {
                    delayTable[i][j] = t;
                    // The forwarding energy of router k needs to be subtracted
                    energyTable[i][j] = energyTable[i][k] + energyTable[k][j] - forwardEnergy[k];
                    setDoubleHop(nextHop, i, j, nextHop[i][k], nextHop[j][k]);
                }
            }
        }
    }
}

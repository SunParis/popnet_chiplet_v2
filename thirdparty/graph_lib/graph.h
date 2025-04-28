# pragma once

# ifndef _GRAPH_H_
# define _GRAPH_H_ 1

# include <boost/config.hpp>
# include <boost/graph/adjacency_list.hpp>
# include <boost/graph/breadth_first_search.hpp>
# include <boost/graph/graph_utility.hpp>
# include <boost/graph/graphviz.hpp>
# include <boost/graph/graphml.hpp>

# include <iostream>
# include <string>
# include <set>
# include <list>
# include <forward_list>
# include <algorithm>

# ifdef _WIN64
    # ifdef _DEBUG
        # pragma comment(lib, "libboost_graph-vc142-mt-gd-x64-1_74.lib")
        # pragma comment(lib, "libboost_regex-vc142-mt-gd-x64-1_74.lib")
    # else
        # pragma comment(lib, "libboost_graph-vc142-mt-x64-1_74.lib")
        # pragma comment(lib, "libboost_regex-vc142-mt-x64-1_74.lib")
    # endif
# endif

namespace GraphLib {

/**
 * @brief Vertex information
 * @property bfso: breadth first search order
 * @property color: default color type
 * @property index: index
 * @property name: vertex name
 * @property energyPerForwarding: energy per forwarding
 * @property inTraffic: in traffic
 * @property outTraffic: out traffic
 * @property pipelineStageDelay: pipeline stage delay
 * @note all members are initialized
 */
struct vertex_info {
    
    std::size_t bfso;
    boost::default_color_type color;
    std::size_t index;
    std::string name;
    double energyPerForwarding;
    int inTraffic;
    int outTraffic;
    double pipelineStageDelay;

	vertex_info()
	:   bfso(0),
        color(),
        index(0),
        name(),
        energyPerForwarding(0),
        inTraffic(0),
        outTraffic(0),
        pipelineStageDelay(0)
    {}

};

/**
 * @brief Edge information
 * @param delay: delay
 * @param energyPerHop: energy per hop
 */
struct SEdgeInfo {
    std::string name;
    double delay;
    double energyPerHop;
};


typedef boost::undirectedS edge_type;

typedef vertex_info vertex_p;

/**
 * @note the first 'double' is delay, the second 'double' is energy per forwarding
 */
typedef boost::property<boost::edge_weight_t, double,
    boost::property<boost::edge_name_t, std::string,
    boost::property<boost::edge_weight2_t, double
>>> edge_p;

typedef boost::property<boost::graph_name_t, std::string> graph_p;

typedef boost::listS vertexContainer_t;

typedef boost::listS edgeContainer_t;

typedef boost::adjacency_list<edgeContainer_t, vertexContainer_t,
    edge_type, vertex_p, edge_p, graph_p> graph_t;

typedef boost::graph_traits<graph_t>::vertices_size_type gsize_t;

typedef boost::graph_traits<graph_t>::vertex_descriptor vertexDes_t;

typedef boost::graph_traits<graph_t>::edge_descriptor edgeDes_t;

typedef std::set<std::string> vertexList_t;

typedef std::set<vertexList_t> vertexCutsetList_t;

typedef std::list<edgeDes_t> edgeList_t;

typedef std::forward_list<edgeList_t> edgeCutsetList_t;

typedef vertexDes_t vertex_t;

typedef edgeDes_t edge_t;

/**
 * @brief Get the Vertex Name from the graph
 * @param g graph
 * @param v vertex descriptor
 * @param name vertex name
 */
void getVertexName(const graph_t& g, vertexDes_t v, std::string& name);

/**
 * @brief Print the graph by vertex names (my version)
 * @param g graph
 */
void myPrintGraphByVertexName(const graph_t& g);

/**
 * @brief Print the graph by vertex names (boost version)
 * @param g graph
 */
void boostPrintGraphByVertexName(const graph_t& g);

/**
 * @brief Print the graph
 * @param g graph
 * @note just calling boostPrintGraphByVertexName(g)
 */
void printGraph(const graph_t& g);

/**
 * @brief Remove a vertex and its edges from the graph
 * @param g graph
 * @param v vertex descriptor
 */
void removeVertexAndItsEdges(graph_t& g, vertexDes_t v);

/**
 * @brief Print the graph index
 * @param g graph
 */
void printGraphIndex(const graph_t& g);

/**
 * @brief Print the graph color
 * @param g graph
 */
void printGraphColor(const graph_t& g);

/**
 * @brief Reset the index of the graph
 * @param g graph
 */
void resetIndex(graph_t& g);

/**
 * @brief Get the undirected degree of a vertex
 * @param g graph
 * @param v vertex descriptor
 * @return the undirected degree of the vertex
 */
gsize_t undirectedDegree(const graph_t& g, vertex_t v);

/**
 * @brief Reset the color of the graph
 * @param g graph
 */
void resetColor(graph_t& g);

/**
 * @brief Set the dynamic properties of the graph
 * @param dp dynamic properties
 * @param graph graph
 * @param graphName graph name
 */
void setDP(boost::dynamic_properties& dp, graph_t& graph, boost::ref_property_map<graph_t*, std::string>& graphName);

/**
 * @brief Read a graph from a .gv file
 * @param gvFile .gv file
 * @param graph graph
 * @return true if the graph is read successfully, false otherwise
 * @note read from a dot language source code file
 */
bool readGvGraph(const std::string& gvFile, graph_t& graph);

/**
 * @brief Read a graph from a .graphml file
 * @param graphmlFile .graphml file
 * @param graph graph
 * @return true if the graph is read successfully, false otherwise
 * @note read from a graph markup language file
 */
bool readGraphmlGraph(const std::string& graphmlFile, graph_t& graph);

/**
 * @brief Read a graph from a file
 * @param graphFile file
 * @param graph graph
 * @return true if the graph is read successfully, false otherwise
 * @note just calling readGraphmlGraph(graphFile, graph)
 */
bool readGraph(const std::string& graphFile, graph_t& graph);

/**
 * @brief Write a graph to a .gv file
 * @param gvFile .gv file
 * @param graph graph
 * @return true if the graph is written successfully, false otherwise
 * @note write to a dot language source code file
 */
bool writeGvGraph(const std::string& gvFile, graph_t& graph);

/**
 * @brief Write a graph to a .graphml file
 * @param graphmlFile .graphml file
 * @param graph graph
 * @return true if the graph is written successfully, false otherwise
 * @note write to a graph markup language file
 */
bool writeGraphmlGraph(const std::string& graphmlFile, graph_t& graph);

/**
 * @brief Write a graph to a .graphml file
 * @param graphmlFile .graphml file
 * @param graph graph
 * @return true if the graph is written successfully, false otherwise
 * @note just calling writeGraphmlGraph(graphFile, graph)
 */
bool writeGraph(const std::string& graphmlFile, graph_t& graph);

};

#endif

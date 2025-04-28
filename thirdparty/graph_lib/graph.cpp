# include "graph.h"


void GraphLib::getVertexName(const GraphLib::graph_t& g, GraphLib::vertexDes_t v, std::string& name) {
    auto vnm = boost::get(&vertex_info::name, g);
    name = vnm[v];
}

void GraphLib::myPrintGraphByVertexName(const GraphLib::graph_t& g) {
    auto vnm = boost::get(&GraphLib::vertex_info::name, g);
    boost::graph_traits<GraphLib::graph_t>::edge_iterator ei, ei_end;
    std::cout << "edges: ";
    for (boost::tuples::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei) {
        std::cout << '(' << vnm[boost::source(*ei, g)] << ", " << vnm[boost::target(*ei, g)] << ") ";
    }
    std::cout << std::endl;
}

void GraphLib::boostPrintGraphByVertexName(const GraphLib::graph_t& g) {
    boost::print_graph(g, boost::get(&GraphLib::vertex_info::name, g));
}

void GraphLib::printGraph(const GraphLib::graph_t& g) {
    GraphLib::boostPrintGraphByVertexName(g);
}

void GraphLib::removeVertexAndItsEdges(GraphLib::graph_t& g, GraphLib::vertexDes_t v) {
    boost::clear_vertex(v, g);
    boost::remove_vertex(v, g);
}

void GraphLib::printGraphIndex(const GraphLib::graph_t& g) {
    auto vertexList = boost::vertices(g);
    std::cout << "Index:\n";
    std::for_each(vertexList.first, vertexList.second, [&](GraphLib::vertex_t v) {
        std::cout << "(" << g[v].name << ", " << g[v].index << ")\n";
    });
}

void GraphLib::printGraphColor(const GraphLib::graph_t& g) {
    auto vertexList = boost::vertices(g);
    std::cout << "Color:\n";
    std::for_each(vertexList.first, vertexList.second, [&](GraphLib::vertex_t v) {
        std::cout << "(" << g[v].name << ", " << g[v].color << ")\n";
    });
}

void GraphLib::resetIndex(GraphLib::graph_t &g) {
	gsize_t index = 0;
	BGL_FORALL_VERTICES(v, g, GraphLib::graph_t) {
		boost::put(&GraphLib::vertex_info::index, g, v, index++);
	}
}

GraphLib::gsize_t GraphLib::undirectedDegree(const GraphLib::graph_t& g, GraphLib::vertex_t v) {
    return boost::degree(v, g);
}

void GraphLib::resetColor(GraphLib::graph_t& g) {
	BGL_FORALL_VERTICES(v, g, GraphLib::graph_t) {
		boost::put(&GraphLib::vertex_info::color, g, v, boost::default_color_type());
	}
}

void GraphLib::setDP(boost::dynamic_properties& dp, GraphLib::graph_t& graph, boost::ref_property_map<GraphLib::graph_t*, std::string>& graphName) {
    dp.property("name", graphName);
    dp.property("node_id", boost::get(&GraphLib::vertex_info::name, graph));
    dp.property("energy_per_forwarding", boost::get(&GraphLib::vertex_info::energyPerForwarding, graph));
    dp.property("in_traffic", boost::get(&GraphLib::vertex_info::inTraffic,graph));
    dp.property("out_traffic", boost::get(&GraphLib::vertex_info::outTraffic,graph));
    dp.property("pipeline_stage_delay", boost::get(&GraphLib::vertex_info::pipelineStageDelay, graph));
    dp.property("id", boost::get(boost::edge_name_t::edge_name, graph));
    dp.property("weight", boost::get(boost::edge_weight_t::edge_weight, graph));
    dp.property("energy_per_hop", boost::get(boost::edge_weight2_t::edge_weight2, graph));
}

bool GraphLib::readGvGraph(const std::string& gvFile, GraphLib::graph_t& graph) {
    boost::dynamic_properties dp;
    boost::ref_property_map<GraphLib::graph_t*, std::string> gname(boost::get_property(graph, boost::graph_name_t::graph_name));
    GraphLib::setDP(dp, graph, gname);
    std::ifstream ifs(gvFile);
    bool status = boost::read_graphviz(ifs, graph, dp);
    ifs.close();
    GraphLib::resetIndex(graph);
    return status;
}

bool GraphLib::readGraphmlGraph(const std::string& graphmlFile, GraphLib::graph_t& graph) {
    boost::dynamic_properties dp;
    boost::ref_property_map<GraphLib::graph_t*, std::string> gname(boost::get_property(graph, boost::graph_name_t::graph_name));
    GraphLib::setDP(dp, graph, gname);
    std::ifstream ifs(graphmlFile);
    boost::read_graphml(ifs, graph, dp);
    ifs.close();
    GraphLib::resetIndex(graph);
    return true;
}

bool GraphLib::readGraph(const std::string& graphFile, GraphLib::graph_t& graph) {
    return GraphLib::readGraphmlGraph(graphFile, graph);
}

bool GraphLib::writeGraphmlGraph(const std::string& graphmlFile, GraphLib::graph_t& graph) {
    std::ofstream ofs(graphmlFile);
    boost::dynamic_properties dp;
    boost::ref_property_map<GraphLib::graph_t*, std::string> gname(boost::get_property(graph, boost::graph_name_t::graph_name));
    GraphLib::setDP(dp, graph, gname);
    boost::write_graphml(ofs, graph, boost::get(&GraphLib::vertex_info::name, graph), dp);
    ofs.close();
    return true;
}

bool GraphLib::writeGraph(const std::string& graphmlFile, GraphLib::graph_t& graph) {
    return GraphLib::writeGraphmlGraph(graphmlFile, graph);
}

bool GraphLib::writeGvGraph(const std::string& gvFile, GraphLib::graph_t& graph) {
    std::ofstream ofs(gvFile);
    boost::dynamic_properties dp;
    boost::ref_property_map<GraphLib::graph_t*, std::string> gname(boost::get_property(graph, boost::graph_name_t::graph_name));
    GraphLib::setDP(dp, graph, gname);
    boost::write_graphviz_dp(ofs, graph, dp);
    return true;
}

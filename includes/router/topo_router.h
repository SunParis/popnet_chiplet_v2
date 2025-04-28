# pragma once

# ifndef _CONFIG_ROUTER_H_
# define _CONFIG_ROUTER_H_ 1

# include "global.h"
# include "router/base_router.h"
# include "alg/shortest_path_routing.h"

# include "graph.h"

class TopoInfo {

public:

    struct SLinkInfo {
		
        TAddressNumber neighbourPort;
		
        TimeType linkDelay;
		
        TAddressNumber neighbour;
	
    };

	GraphLib::graph_t topo0;

    ShortestPath::TMatrix delayTable;

    ShortestPath::TMatrix energyTable;

    ShortestPath::TIntMatrix routingTable;

    TAddressNumber vertexCnt;

    std::unique_ptr<std::vector<SLinkInfo>[]> portMap;

    std::unique_ptr<std::unordered_map<TAddressNumber, TAddressNumber>[]> nextHop_port_map;

    std::vector<std::size_t> vPortCount;

    std::unique_ptr<GraphLib::vertex_t[]> topoVertices;

    void readTopo(const std::string& topo_file_path);

    void setPipelineDelay();

    void setPortCnt();

    void otherInit();

    void setTopoVertices();

    void calRoutingTable();

    TopoInfo(const std::string& topo_file_path);

};



class CGraphTopo: public BaseRouter {

protected:

	TopoInfo& topo_info;
    
    /**
     * @brief the routing algorithm
     * @param dst The destination address
     * @param src The source address
     * @param s_ph The physical port
     * @param s_vc The virtual channel
     */
    void routingAlg(const AddrType& dst, const AddrType& src, long s_ph, long s_vc);
	
    /**
     * @brief get the wire delay
     * @param port The port
     * @return The wire delay
     */
    TimeType getWireDelay(long port);
	
    /**
     * @brief get the next address
     * @param next_addr The next address
     * @param port The port
     * @note The result will be stored in `next_addr`
     */
    void getNextAddress(AddrType& next_addr, long port);
	
    /**
     * @brief get the wire pc
     * @param port The port
     * @return The wire pc
     */
    long getWirePc(long port);
	
    /**
     * @brief get the from router
     * @param from The from address
     * @param port The port
     * @note The result will be stored in `from`
     */
    void getFromRouter(AddrType& from, long port);
	
    /**
     * @brief get the from port
     * @param port The port
     * @return The from port
     */
    long getFromPort(long port);

    /**
     * @brief get the pipeline stage delay
     */
    TimeType pipelineStageDelay();

public:

	/**
     * @brief Construct a new CGraphTopo object
     * @param config The config of the simulation
     * @param addr The address of the router
     * @param router_list_ The list of routers
     * @param topo_info The graph topology information
     */
    CGraphTopo(const Config& config, AddrType addr, std::vector<BaseRouter *>& router_list_, TopoInfo& topo_info);

};

class ReconfigTopoInfo {

public:

    struct SFlow {
        int src, dst;
        double delay;
    };

    struct SLinkInfo {
		
        TAddressNumber neighbourPort;
		
        TimeType linkDelay;
		
        TAddressNumber neighbour;
	
    };
    
    double reconfigPeriod;
    
    std::int64_t periodCnt;
    
    std::int64_t reconfigRouterCnt;
    
    std::vector<TAddressNumber> reconfigRouters;
    
    std::vector<std::vector<SFlow>> flows;

    GraphLib::graph_t curTopo;

    GraphLib::graph_t topo0;
    
    std::int64_t curReconfigPeriodNumber;
    
    TimeType lastReconfigurationTime;
    
    TimeType nextReconfigurationTime;

    ShortestPath::TIntMatrix routingTable;

    ShortestPath::TIntMatrix old_routingTable;

    std::unique_ptr<std::vector<SLinkInfo>[]> portMap;

    std::unique_ptr<std::vector<SLinkInfo>[]> old_portMap;
	
    std::unique_ptr<std::unordered_map<TAddressNumber, TAddressNumber>[]> nextHop_port_map;
    
    std::unique_ptr<std::unordered_map<TAddressNumber, TAddressNumber>[]> old_nextHop_port_map;
    
    ShortestPath::TMatrix delayTable;

    ShortestPath::TMatrix energyTable;    

    TAddressNumber vertexCnt;

    std::vector<std::size_t> vPortCount;

    std::unique_ptr<GraphLib::vertex_t[]> topoVertices;

    void readTopo(const std::string& topo_file_path);

    void setPipelineDelay(GraphLib::graph_t& topo);

    void setPortCnt();

    void otherInit();

    void setTopoVertices(GraphLib::graph_t& topo);

    void calRoutingTable(GraphLib::graph_t& topo);
    
    void readReconfigurationFile(const std::string& reconfigurationFilePath);
    
    void setPortCnt_rtr(GraphLib::graph_t& topo);
    
    void reconfigurateTopology(std::int64_t periodNumber);
    
    void resetTopology();
    
    void swapRoutingInfo();

    double getReconfigurationPeriod();
    
    std::int64_t getCurrentReconfigurationPeriod();

    void reconfigurate(TimeType reconfigurationTime, TimeType next_reconfigurationTime);

    ReconfigTopoInfo(const std::string& topo_file_path, const std::string& reconfig_file_path);

};

class CReconfigTopoRouter: public BaseRouter {

protected:

	ReconfigTopoInfo& topo_info;
    
    /**
     * @brief the routing algorithm
     * @param dst The destination address
     * @param src The source address
     * @param s_ph The physical port
     * @param s_vc The virtual channel
     */
    void routingAlg(const AddrType& dst, const AddrType& src, long s_ph, long s_vc);
	
    /**
     * @brief get the wire delay
     * @param port The port
     * @return The wire delay
     */
    TimeType getWireDelay(long port);
	
    /**
     * @brief get the next address
     * @param next_addr The next address
     * @param port The port
     * @note The result will be stored in `next_addr`
     */
    void getNextAddress(AddrType& next_addr, long port);
	
    /**
     * @brief get the wire pc
     * @param port The port
     * @return The wire pc
     */
    long getWirePc(long port);
	
    /**
     * @brief get the from router
     * @param from The from address
     * @param port The port
     * @note The result will be stored in `from`
     */
    void getFromRouter(AddrType& from, long port);
	
    /**
     * @brief get the from port
     * @param port The port
     * @return The from port
     */
    long getFromPort(long port);

    /**
     * @brief get the pipeline stage delay
     */
    TimeType pipelineStageDelay();

    bool isEventAfterReconfiguration(TimeType delay);
    
    bool isNearReconfiguration();

public:

    CReconfigTopoRouter(const Config& config, AddrType addr, std::vector<BaseRouter *>& router_list_, ReconfigTopoInfo& topo_info);

};

# endif
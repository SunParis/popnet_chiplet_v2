# pragma once


# ifndef _BASE_ROUTER_H_
# define _BASE_ROUTER_H_ 1


# include <functional>
# include <vector>
# include <unordered_map>

# include "global.h"
# include "router/modules.h"
# include "preprocess/config.h"

class BaseRouter {

protected:

    /**
     * @brief the router list
     */
    std::vector<BaseRouter *>& router_list_;

    /**
     * @brief the address of the router
     */
    AddrType address_;
	
    /**
     * @brief the input module of the router
     */
	InputModules input_module_;
	
    /**
     * @brief the output module of the router
     */
	OutputModules output_module_;
	
    /**
     * @brief the power module of the router
     */
	PowerModules power_module_;
	
    /**
     * @brief the input buffer of the router, which will be random
     */
	DataType init_data_;

    /**
     * @brief the config of the simulation
     */
    const Config& config_;
	
    /**
     * @brief the size of the network
     */
    long ary_size_;
	
    /**
     * @brief the size of the flit
     */
    long flit_size_;
	
    /**
     * @brief the number of physical ports
     */
    long physic_ports_;
	
    /**
     * @brief the number of virtual channels
     */
    long vc_number_;
	
    /**
     * @brief the size of the buffer
     */
    long inbuffer_size_;
	
    /**
     * @brief the size of the output buffer
     */
    long outbuffer_size_;
	
    /**
     * @brief the total delay of the flits
     */
	TimeType total_delay_;
	
    /**
     * @brief the routing algorithm used
     */
	RoutingType routing_alg_;
	
    /**
     * @brief the current routing algorithm
     */
    std::function<void(const AddrType&, const AddrType&, long, long)> curr_algorithm;
    
    /**
     * @brief the current previous router function
     */
    std::function<void(AddrType&, long)> curr_prevRouterFunc;
    
    /**
     * @brief the current previous port function
     */
    std::function<long(long)> curr_prevPortFunc;
    
    /**
     * @brief the current wire delay function
     */
    std::function<TimeType(long)> curr_wireDelayFunc;
    
    /**
     * @brief the current next address function
     */
    std::function<void(AddrType&, long)> curr_nextAddFunc;
    
    /**
     * @brief the current wire pc function
     */
    std::function<long(long)> curr_wirePcFunc;

	/**
     * @brief local time
     */
	TimeType local_time_;
	
    /**
     * @brief packet counter
     */
	long packet_counter_;

    /**
     * @brief the routing time
     */
    TimeType routing_time_ = 0;

	/**
     * @brief the wire delay time
     * @param port The port
     * @return The wire delay time
     */
    virtual TimeType getWireDelay(long port);
	
    /**
     * @brief the wire delay time for the mesh
     * @param port The port
     * @return The wire delay time
     */
    TimeType getWireDelay_mesh(long port);
	
    /**
     * @brief the wire delay time for the chiplet mesh
     * @param port The port
     * @return The wire delay time
     */
    TimeType getWireDelay_chipletMesh(long port);
	
    /**
     * @brief the wire delay time for the chiplet star
     * @param port The port
     * @return The wire delay time
     */
    TimeType getWireDelay_chipletStar(long port);
    
    /**
     * @brief get the next address
     * @param nextAddress The next address
     * @param port The port
     * @note The result will be stored in `nextAddress`
     */
    virtual void getNextAddress(AddrType& nextAddress, long port);

    /**
     * @brief get the next address for the chiplet mesh
     * @param nextAddress The next address
     * @param port The port
     * @note The result will be stored in `nextAddress`
     */
    void getNextAddress_chipletMesh(AddrType& nextAddress, long port);

    /**
     * @brief get the next address for the chiplet star
     * @param nextAddress The next address
     * @param port The port
     * @note The result will be stored in `nextAddress`
     */
    void getNextAddress_chipletStar(AddrType& nextAddress, long port);

    /**
     * @brief get the next address for the mesh
     * @param nextAddress The next address
     * @param port The port
     * @note The result will be stored in `nextAddress`
     */
    void getNextAddress_mesh(AddrType& nextAddress, long port);

    /**
     * @brief get the wire pc
     * @param port The port
     * @return The wire pc
     */
    virtual long getWirePc(long port);

    /**
     * @brief get the wire pc for the mesh
     * @param port The port
     * @return The wire pc
     */
    long getWirePc_mesh(long port);

    /**
     * @brief get the wire pc for the chiplet star
     * @param port The port
     * @return The wire pc
     */
    long getWirePc_chipletStar(long port);

    /**
     * @brief get the wire pc for the chiplet mesh
     * @param port The port
     * @return The wire pc
     * @note The result will be stored in `from`
     */
    virtual void getFromRouter(AddrType& from, long port);

    /**
     * @brief get the wire pc for the mesh
     * @param from The from address
     * @param port The port
     * @note The result will be stored in `from`
     */
    void getFromRouter_mesh(AddrType& from, long port);

    /**
     * @brief get the wire pc for the chiplet star
     * @param from The from address
     * @param port The port
     * @note The result will be stored in `from`
     */
    void getFromRouter_chipletStar(AddrType& from, long port);

    /**
     * @brief get the from port
     * @param port The port
     * @return The from port
     */
    virtual long getFromPort(long port);

    /**
     * @brief get the from port for the mesh
     * @param port The port
     * @return The from port
     */
    long getFromPort_mesh(long port);

    /**
     * @brief get the from port for the chiplet star
     * @param port The port
     * @return The from port
     */
    long getFromPort_chipletStar(long port);

    /**
     * @brief Get the destination address according to the routing algorithm
     * @param destination The destination address
     * @param source The source address
     * @param s_ph The physical port
     * @param s_vc The virtual channel
     * @note The result will be stored in `destination`
     */
    virtual void routingAlg(const AddrType& destination, const AddrType& source, long s_ph, long s_vc);

    /**
     * @brief Set the routing type
     */
    void setRoutingType();

    /**
     * @brief Get the pipe stage delay time
     * @param return The pipe stage delay time
     */
    virtual TimeType getPipeStageDelay() const;

    /**
     * @brief Get a router according to the address
     * @param addr The address of the router
     * @return The router
     */
    BaseRouter& getRouter(const AddrType& addr);

    void updateTrans(TimeType new_time, const Flit& flit);

    void updateTransACK(TimeType new_time, ProtoStateMachine& trans);

    void updateTransNormal(TimeType new_time, ProtoStateMachine& trans);

    void injctPacket(long flit_id, const AddrType& src_addr, const AddrType& des_addr,
        TimeType start_time, long packet_size, TId packet_id);
    
    /**
     * @brief Update the delay
     * @param new_delay The new delay
     */
    void updateDelay(TimeType new_delay);

    void decideRouting();    
	
    VCType selectVC(long phy_idx, long vc_idx);
	
    void arbitrationVC();

	void arbitrationSW();

	void toOutBuffer();

	void traverseLink();
	
    void traverseLink(long port);    
	
    void XY_algorithm(const AddrType& des_t, const AddrType& sor_t, long s_ph, long s_vc);
	
    void TXY_algorithm(const AddrType& des_t, const AddrType& sor_t, long s_ph, long s_vc);
        
	void chiplet_routing_alg(const AddrType& des_t,const AddrType& sor_t, long s_ph, long s_vc);
    
	void chiplet_star_topo_routing_alg(const AddrType& des_t,const AddrType& src_t, long s_ph, long s_vc);

public:

    TimeType getLocalTime() const;

    void setLocalTime(TimeType new_local_time);

    void acceptFlit(TimeType accept_time, const Flit& target_flt);
	
	/**
     * @brief Get the power of buffer
     * @return the power of buffer
     */
    double getBufferPower();
    
    /**
     * @brief Get the power of link
     * @return the power of link
     */
    double getLinkPower();
    
    /**
     * @brief Get the power of crossbar
     * @return the power of crossbar
     */
    double getCrossbarPower();
    
    /**
     * @brief Get the power of arbiter
     * @return the power of arbiter
     */
    double getArbiterPower();
    			
    /**
     * @brief Get the total delay
     * @return The total delay
     */
    TimeType getTotalDelay() const;

	void recvCredit(long phy_idx, long vc_idx);

	void recvPacket();
	
    void recvFlit(long phy_idx, long vc_idx, Flit& flit);	

    void routingPipeStage(TimeType routing_period);

    /**
     * @brief Construct a new BaseRouter object
     * @param config The config of the simulation
     * @param addr The address of the router
     * @param router_list The list of routers
     */
    BaseRouter(const Config& config, AddrType addr, std::vector<BaseRouter *>& router_list);

    virtual ~BaseRouter() = default;

};

/**
 * @brief Add the remainder
 * @param dividend The dividend
 * @param divisor The divisor
 */
static void remainderAdd(long& dividend, long divisor) {
	dividend++;
	if (dividend == divisor)
        dividend = 0;
}

/**
 * @brief Reduce the remainder
 * @param dividend The dividend
 * @param divisor The divisor
 */
static void remainderReduce(long& dividend, long divisor) {
	if (dividend == 0)
        dividend = divisor-1;
	else dividend--;
}

/**
 * @brief Check if the address is central
 * @param address The address to check
 * @param centerXY The center of the network
 */
static bool isCentral(const AddrType& address, long centerXY) {
	return address[0] == centerXY && address[1] == centerXY
        && address[2] == 0 && address[3] == 0;
}

# endif
# pragma once

# ifndef _ROUTER_H_
# define _ROUTER_H_ 1

# include "global.h"
# include "router/base_router.h"

class CXYRouter: public BaseRouter {

protected:
	
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

public:
	
    /**
     * @brief Construct a new CXYRouter object
     * @param config The config of the simulation
     * @param addr The address of the router
     * @param router_list_ The list of routers
     */
    CXYRouter(const Config& config, AddrType addr, std::vector<BaseRouter *>& router_list_);

};

class CTXYRouter: public BaseRouter {

protected:
	
    /**
     * @brief the routing algorithm
     * @param dst The destination address
     * @param src The source address
     * @param s_ph The physical port
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

public:
	
    /**
     * @brief Construct a new CTXYRouter object
     * @param config The config of the simulation
     * @param addr The address of the router
     * @param router_list_ The list of routers
     */
    CTXYRouter(const Config& config, AddrType addr, std::vector<BaseRouter *>& router_list_);

};


class CChipletMeshRouter: public BaseRouter {

protected:

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

public:

	/**
     * @brief Construct a new CChipletMeshRouter object
     * @param config The config of the simulation
     * @param addr The address of the router
     * @param router_list_ The list of routers
     */
    CChipletMeshRouter(const Config& config, AddrType addr, std::vector<BaseRouter *>& router_list_);

};


class CChipletStarRouter: public BaseRouter {

protected:

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

public:

	/**
     * @brief Construct a new CChipletStarRouter object
     * @param config The config of the simulation
     * @param addr The address of the router
     * @param router_list_ The list of routers
     */
    CChipletStarRouter(const Config& config, AddrType addr, std::vector<BaseRouter *>& router_list_);

};


# endif
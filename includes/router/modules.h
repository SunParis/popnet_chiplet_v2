# pragma once

# ifndef _MODULES_H_
# define _MODULES_H_ 1

# include <vector>
# include <list>

# include "global.h"
extern "C" {
    # include "SIM_power.h"
    # include "SIM_router_power.h"
    # include "SIM_power_router.h"
}

# ifndef FUNC
// Define FUNC as a no-operation macro to suppress specific linting errors in VSCode.
// This definition does not affect the compiled executable.
// clang-format off
# define FUNC(...) // NOLINT
// clang-format on
# endif

class InputModules {

private:
	
    std::vector<std::vector<std::list<Flit>>> input_;
    
    std::vector<std::vector<VCStateType>> states_;
	
    // the candidate routing vcs
	std::vector<std::vector<std::vector<VCType>>> routing_;
	
    // the chosen routing vc
	std::vector<std::vector<VCType>> crouting_;
	
    // this is a flag to show that the buffer of injection is full
	bool ibuff_full_;

public:
   
    /**
     * @brief Construct a new Input Modules object
     * @param phy_port_num the number of physical ports
     * @param vc_num the number of virtual channels
     */
    InputModules(long phy_port_num, long vc_num);

    ~InputModules() = default;

    /**
     * @brief If the input buffer is full
     */
    bool isIBuffFull() const;

    /**
     * @brief set the input buffer is full or not
     */
    void setIBuffFull(bool is_buffer_full);

    /**
     * @brief Get the buffer size
     * @param pc the physical port
     * @param vc the virtual channel
     * @return the buffer size
     */
    std::size_t getBufferSize(long pc, long vc) const;

    /**
     * @brief set the state
     * @param pc the physical port
     * @param vc the virtual channel
     * @param state the new state
     */
    void setState(long pc, long vc, VCStateType state);

    /**
     * @brief get the state
     * @param pc the physical port
     * @param vc the virtual channel
     * @return the state
     */
    VCStateType getState(long pc, long vc) const;

    /**
     * @brief add a flit
     * @param pc the physical port
     * @param vc the virtual channel
     * @param flit the flit
     */
    void addFlit(long pc, long vc, const Flit& flit);

    /**
     * @brief Get the flit
     * @param pc the physical port
     * @param vc the virtual channel
     * @return the flit
     */
    Flit& getFlit(long pc, long vc);

    /**
     * @brief Remove the flit
     * @param pc the physical port
     * @param vc the virtual channel
     * @param flit the flit
     */
    void removeFlit(long pc, long vc);

    /**
     * @brief Get the routing
     * @param pc the physical port
     * @param vc the virtual channel
     * @return the routing
     */
    std::vector<VCType>& getRouting(long pc, long vc);

    /**
     * @brief Add the routing
     * @param pc the physical port
     * @param vc the virtual channel
     * @param vc_type the virtual channel type
     */
    void addRouting(long pc, long vc, VCType vc_type);
    
    /**
     * @brief Clear the routing
     * @param pc the physical port
     * @param vc the virtual channel
     */
    void clearRouting(long pc, long vc);

    /**
     * @brief Get the CRouting
     * @param pc the physical port
     * @param vc the virtual channel
     */
    VCType getCRouting(long pc, long vc);

    /**
     * @brief Set the CRouting
     * @param pc the physical port
     * @param vc the virtual channel
     * @param vc_type the virtual channel type
     */
    void setCRouting(long pc, long vc, VCType vc_type);

};

class OutputModules {
private:
	
    // used for next input
	long buffer_size_;
	
    std::vector<std::vector<long>> counter_;
	
    std::vector<std::vector<VCStateType>> flit_state_;
	
    // assigned for the input 
	std::vector<std::vector<VCType>> assign_;
	
    // USED_ FREE_
	std::vector<std::vector<VCUsageType>> usage_;
	
    // local output buffers
	std::vector<std::list<Flit>> outbuffers_;
	
    // output address
	std::vector<std::list<VCType>> outadd_;
	std::vector<long> localcounter_;

public:

    /**
     * @brief Construct a new Output Modules object
     * @param phy_port_num the number of physical ports
     * @param vc_num the number of virtual channels
     * @param input_buffer_size the size of input buffer
     * @param output_buffer_size the size of output buffer
     */
    OutputModules(long phy_port_num, long vc_num, long input_buffer_size, long output_buffer_size);

    ~OutputModules() = default;

    /**
     * @brief increment the counter
     * @param phy_port the physical port
     * @param vc the virtual channel
     */
    void incCounter(long phy_port, long vc);

    /**
     * @brief decrement the counter
     * @param phy_port the physical port
     * @param vc the virtual channel
     */
    void decCounter(long phy_port, long vc);

    /**
     * @brief get the counter
     * @param phy_port the physical port
     * @param vc the virtual channel
     * @return the counter
     */
    long getCounter(long phy_port, long vc);

    /**
     * @brief Get the local counter
     * @param phy_port the physical port
     * @return the local counter
     */
    long getLocalCounter(long phy_port);

    /**
     * @brief Get the usage
     * @param phy_port the physical port
     * @param vc the virtual channel
     * @return the usage
     */
    VCUsageType getUsage(long phy_port, long vc);

    /**
     * @brief Acquire the channel
     * @param phy_port the physical port
     * @param vc the virtual channel
     * @param vc_type the vc_type
     */
    void acquireChannel(long phy_port, long vc, VCType vc_type);

    /**
     * @brief Release a channel
     * @param phy_port the physical port
     * @param vc the virtual channel
     */
    void releaseChannel(long phy_port, long vc);

    /**
     * @brief Get the out buffer size
     * @param port the physical port
     * @return the out buffer size
     */
    std::size_t getOutBufferSize(long port);
    
    /**
     * @brief Add a flit
     * @param port the physical port
     * @param flit the flit
     */
    void addFlit(long port, const Flit& flit);

    /**
     * @brief Get the flit
     * @param port the port
     * @return the flit
     */
    Flit& getFlit(long port);

    /**
     * @brief Remove the flit
     * @param port the port
     */
    void removeFlit(long port);

    /**
     * @brief Add a address
     * @param port the port
     * @param addr the address
     */
    void addAddr(long port, VCType& addr);

    /**
     * @brief Get the address
     * @param port the port
     * @return the address
     */
    VCType getAddr(long port);

    /**
     * @brief Remove the address
     * @param port the port
     */
    void removeAddr(long port);

};


class PowerModules {

private:
    
    long flit_size_;
	SIM_power_router_info_t router_info_;
	SIM_power_router_t router_power_;
	SIM_power_arbiter_t arbiter_vc_power_;
	SIM_power_bus_t link_power_;
	std::vector<DataType> buffer_write_;
	std::vector<DataType> buffer_read_;
	std::vector<DataType> crossbar_read_;
	std::vector<DataType> crossbar_write_;
	std::vector<DataType> link_traversal_;
	std::vector<long> crossbar_input_;
	std::vector<std::vector<AtomType>> arbiter_vc_req_;
	std::vector<std::vector<unsigned long>> arbiter_vc_grant_;

public:

    /**
     * @brief Construct a new Power Modules object
     * @param phy_port_num the number of physical ports
     * @param vc_num the number of virtual ports
     * @param flit_size the size of flit
     * @param link_length the length of link
     */
    PowerModules(long phy_port_num, long vc_num, long flit_size, long link_length);

    ~PowerModules() = default;

    /**
     * @brief Add the power of buffer read
     * @param in_port the input port
     * @param read_d the read data
     */
    void addBufferReadPwr(long in_port, DataType& read_d);
    
    /**
     * @brief Add the power of buffer write
     * @param in_port the input port
     * @param write_d the write data
     */
    void addBufferWritePwr(long in_port, DataType& write_d);
    
    /**
     * @brief Add the power of crossbar traversal
     * @param in_port the input port
     * @param out_port the output port
     * @param trav_d the traversal data
     */
    void addCrossbarTravPwr(long in_port, long out_port, DataType& trav_d);
    
    /**
     * @brief Add the power of VC arbiter
     * @param pc the physical port
     * @param vc the virtual channel
     * @param req the request
     * @param gra the grant
     */
    void addVCArbitPwr(long pc, long vc, AtomType req, unsigned long gra);
    
    /**
     * @brief Add the power of link traversal
     * @param in_port the input port
     * @param read_d the read data
     */
    void addLinkTravPwr(long in_port, DataType& read_d);

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

};

# endif
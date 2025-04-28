# pragma once

# ifndef _CONFIG_H_
# define _CONFIG_H_ 1

# include <string>
# include <unistd.h>
# include <filesystem>
# include <optional>
# include <memory>

# include "global.h"

# include "nlohmann/json.hpp"

class Config {

    /**
     * @brief The number of routers in each dimension of the network, e.g. 9*9 network has this parameter as 9
     */
    long ary_number_;
	
    /**
     * @brief The dimension of the n-cube
     */
    long cube_number_;
	
    /**
     * @brief The number of virtual channels per physical port
     */
    long virtual_channel_number_;

    /**
     * @brief The number of physical ports
     */
    long physical_port_number_;
	
    /**
     * @brief The buffer size of each VC
     */
    long in_buffer_size_;
	
    /**
     * @brief The output buffer size
     */
    long out_buffer_size_;
	
    /**
     * @brief The flit size
     */
    long flit_size_;
	
    /**
     * @brief The link length
     */
    double link_length_;
	
    /**
     * @brief The simulation time length
     */
    TimeType sim_length_;
	
    /**
     * @brief The simulation input file name
     */
    std::string trace_fname_;
	
    /**
     * @brief The routing algorithm
     */
    long routing_alg_;
	
    /**
     * @brief The VC sharing, based on routing algorithms
     */
    VCShareType vc_share_;
	
    /**
     * @brief The topo file path
     */
    std::string topo_file_path_;
	
    /**
     * @brief The report period
     */
    TimeType report_period_;
	
    /**
     * @brief The reconfiguration file path
     */
    std::string reconfig_file_path_;
	
    /**
     * @brief Whether packet loss is enabled
     */
    bool packet_loss_;
	
    /**
     * @brief The delay file name
     */
    std::string delay_fname_;

    /**
     * @brief The log file name
     */
    std::string log_fname_;
	
    /**
     * @brief Whether the sync protocol is enabled
     */
    bool sync_protocol_enable_;

    std::unique_ptr<long> random_seed_;

    void fromJson(const std::string& fname);

    void fromCMD(int argc, char * const argv []);

public:

    /**
     * @brief The constructor for the Config class
     */
    Config(int argc, char * const argv []);

    /**
     * @brief The getter for the ary_number_ parameter
     */
    long getAryNumber() const;
    
    /**
     * @brief The getter for the cube_number_ parameter
     */
    long getCubeNumber() const;
    
    /**
     * @brief The getter for the virtual_channel_number_ parameter
     */
    long getVirtualChannelNumber() const;

    /**
     * @brief The getter for the physical_port_number_ parameter
     */
    long getPhysicalPortNumber() const;
    
    /**
     * @brief The getter for the in_buffer_size_ parameter
     */
    long getInBufferSize() const;
    
    /**
     * @brief The getter for the out_buffer_size_ parameter
     */
    long getOutBufferSize() const;
    
    /**
     * @brief The getter for the flit_size_ parameter
     */
    long getFlitSize() const;
    
    /**
     * @brief The getter for the link_length_ parameter
     */
    double getLinkLength() const;
    
    /**
     * @brief The getter for the sim_length_ parameter
     */
    TimeType getSimLength() const;
    
    /**
     * @brief The getter for the trace_fname_ parameter
     */
    const std::string& getTraceFname() const;
    
    /**
     * @brief The getter for the routing_alg_ parameter
     */
    RoutingType getRoutingAlg() const;
    
    /**
     * @brief The getter for the vc_share_ parameter
     */
    VCShareType getVcShare() const;
    
    /**
     * @brief The getter for the topo_file_path_ parameter
     */
    const std::string& getTopoFilePath() const;

    /**
     * @brief The getter for the log_file_path_ parameter
     */
    const std::string& getLogFilePath() const;
    
    /**
     * @brief The getter for the report_period_ parameter
     */
    TimeType getReportPeriod() const;
    
    /**
     * @brief The getter for the reconfig_file_path_ parameter
     */
    const std::string& getReconfigFilePath() const;
    
    /**
     * @brief The getter for the packet_loss_ parameter
     */
    bool isPacketLoss() const;
    
    /**
     * @brief The getter for the delay_fname_ parameter
     */
    const std::string& getDelayFname() const;
    
    /**
     * @brief The getter for the sync_protocol_enable_ parameter
     */
    bool isSyncProtocolEnable() const;

    /**
     * @brief The getter for the random_seed_ parameter
     */
    std::optional<long> getRandomSeed() const;

};

/**
 * @brief The overloaded operator<< for the Config class
 */
std::ostream& operator<<(std::ostream& os, const Config& cf);

# endif
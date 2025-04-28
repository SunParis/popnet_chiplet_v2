# pragma once

/**
 * @file defines.h
 * @brief This file contains the definition of global variables and types.
 */

# ifndef __GLOBAL_H_
# define __GLOBAL_H_ 1

# include <iostream>
# include <vector>
# include <unordered_set>
# include <limits>
# include <sstream>

// # define REPORT_PERIOD_ 1e10
// # define FILTERING

# define FREQUENCY                              ((TimeType)(1))
# define SHORT_WIRE_DELAY                       ((TimeType)(9.57 * FREQUENCY))
# define LONG_WIRE_DELAY_X                      ((TimeType)(153.16 * FREQUENCY))
# define LONG_WIRE_DELAY_Y                      ((TimeType)(153.16 * FREQUENCY))
# define STAR_TOPO_2_2                          ((TimeType)(76.5769 *  FREQUENCY))
# define STAR_TOPO_4_2_INNER                    ((TimeType)(47.8721 * FREQUENCY))
# define STAR_TOPO_4_2_OUTER                    ((TimeType)(124.4789 * FREQUENCY))
# define STAR_TOPO_4_4_INNER                    ((TimeType)(19.1442 * FREQUENCY))
# define STAR_TOPO_4_4_OUTER                    ((TimeType)(95.7453 * FREQUENCY))
# define STAR_TOPO_4_4_CORNER                   ((TimeType)(172.2980 * FREQUENCY))
# define BUFF_BOUND_                            100
# define WIRE_DELAY_                            0.9
# define PIPE_DELAY_                            1.0
# define CREDIT_DELAY_                          1.0
# define REPORT_PERIOD_                         2000
# define S_ELPS_                                0.00000001
# define ATOM_WIDTH_                            64
# define ZERO_                                  0
# define MAX_64_                                0xffffffffffffffffLL
# define CORR_EFF_                              0.8
# define POWER_NOM_                             1e9
# define START_TIME                             (5.644e8-1)
# define START_RECONFIGURATION_PERIOD_NUMBER    2822
# define VC_NULL                                (VCType(-1, -1))
# define LOCAL_INPUT_TIME_0                     (std::numeric_limits<TimeType>::infinity())
# define LOCAL_INPUT_TIME_0_0                   (-std::numeric_limits<TimeType>::infinity())
# define VIRTUAL_CHANNEL_COUNT_0                (2)

# define SPD_LAUNCH                             0x10000
# define SPD_BARRIER                            0x20000
# define SPD_LOCK                               0x40000
# define SPD_UNLOCK                             0x80000
# define SPD_BARRIER_COUNT_MASK                 0x0FFFF
# define PARAM_1                                std::placeholders::_1
# define PARAM_2                                PARAM_1,std::placeholders::_2
# define PARAM_3                                PARAM_2,std::placeholders::_3
# define PARAM_4                                PARAM_3,std::placeholders::_4

# define VOLA volatile

# ifdef FILTERING
    # define FILTERING_BOOL                     (true)
    # define RECONFIGURATION_PERIOD_NUMBER_0    (START_RECONFIGURATION_PERIOD_NUMBER)
# else
    # define FILTERING_BOOL                     (false)
    # define RECONFIGURATION_PERIOD_NUMBER_0    (0)
# endif

# define RECONFIGURATION_INTERVAL               (20)

# ifndef PI
    # define PI (3.14159265358979323846)
# endif

using TimeType = double;

using TAddressNumber = long;

using AddrType = std::vector<long>;

using VCType = std::pair<long, long>;

using DataType = std::vector<unsigned long long>;

using AtomType = unsigned long long;

using TId = std::size_t;

using TFlitId = long;

using TPacketId = std::size_t;

/**
 * @brief Message type.
 */
enum class MessType: unsigned char {
    EVG = 0,
    ROUTER = 1,
    WIRE = 2, 
    CREDIT = 3,
    RECONFIGURATION = 4
};

/**
 * @brief operator<< overload for `MessType`.
 */
std::ostream& operator<<(std::ostream& os, const MessType& MessType_);

/**
 * @brief Routing type.
 */
enum class RoutingType {
    XY = 0,
    TXY = 1,
    CHIPLET_ROUTING_MESH = 2,
    CHIPLET_STAR_TOPO_ROUTING = 3,
    GRAPH_TOPO = 4,
    RECONFIGURABLE_GRAPH_TOPO = 5
};

/**
 * @brief operator<< overload for `RoutingType`.
 */
std::ostream& operator<<(std::ostream& os, const RoutingType& RoutingType_);

/**
 * @brief Virtual Channel state type.
 */
enum class VCStateType {
    INIT,
    ROUTING,
    VC_AB,
    SW_AB,
    SW_TR,
    HOME
};

/**
 * @brief operator<< overload for `VCStateType`.
 */
std::ostream& operator<<(std::ostream& os, const VCStateType& VCStateType_);

/**
 * @brief Flit type.
 */
enum class FlitType {
    HEADER,
    BODY,
    TAIL,
    SINGLE
};

/**
 * @brief operator<< overload for `FlitType`.
 */
std::ostream& operator<<(std::ostream& os, const FlitType& FlitType_);

/**
 * @brief Virtual Channel share type.
 */
enum class VCShareType {
    SHARE,
    MONO
};

/**
 * @brief operator<< overload for `VCShareType`.
 */
std::ostream& operator<<(std::ostream& os, const VCShareType& VCShareType_);

/**
 * @brief Virtual Channel usage type.
 */
enum class VCUsageType {
    USED,
    FREE
};

/**
 * @brief operator<< overload for `VCUsageType`.
 */
std::ostream& operator<<(std::ostream& os, const VCUsageType& VCUsageType_);

/**
 * @brief Behavior descriptor of synchronization protocol.
 */
enum class SyncProtocolDesc {
    SPD_ACK,
    SPD_PRE_SYNC,
    SPD_POST_SYNC
};

/**
 * @brief operator<< overload for `SyncProtocolDesc`.
 */
std::ostream& operator<<(std::ostream& os, const SyncProtocolDesc& spd);

/**
 * @brief State of synchronization protocol.
 */
enum class ProtoState {
    IDLE,
    PRE_SYNC,
    DATA_TRANS,
    ACK_TRANS,
    POST_SYNC,
    DONE
};

/**
 * @brief operator<< overload for `ProtoState`.
 */
std::ostream& operator<<(std::ostream& os, const ProtoState& state);

/**
 * @brief operator<< overload for `AddrType`.
 */
std::ostream& operator<<(std::ostream& os, const AddrType& address);

# endif

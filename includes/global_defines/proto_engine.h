# pragma once

# ifndef _PROTO_ENGINE_H_
# define _PROTO_ENGINE_H_ 1

# include <vector>

# include "global_defines/SStd.h"
# include "global_defines/defines.h"
# include "global_defines/packet_defines.h"

class ProtoStateMachine {

public:

    TimeType src_time, des_time;
    AddrType src_addr, des_addr;
    long packet_size;
    long protoDesc;
    TId id;
    ProtoState status;
    std::vector<TimeType> packetDelay;

    ProtoStateMachine(const ProtoPacket& trans);
};

/**
 * @brief Add a transaction to the transaction list
 * @param trans The transaction
 * @return The packet
 */
SPacket ___add_trans___(const ProtoPacket& trans);

/**
 * @brief Get a transaction from the transaction list
 * @param id The id of the transaction
 * @return The transaction
 */
ProtoStateMachine& ___get_trans___(TPacketId id);


# endif
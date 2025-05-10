# pragma once

# ifndef _GLOBAL_H_
# define _GLOBAL_H_ 1

# include "global_defines/defines.h"
# include "global_defines/packet_defines.h"
# include "global_defines/RGen.h"
# include "global_defines/proto_engine.h"
# include "global_defines/message_define.h"
# include "global_defines/input_trace.h"
# include "global_defines/SStd.h"

# include "logger/logger.hpp"

namespace Global {

/**
 * @brief The Random number generator
 */
inline RGen RandomGen;

/**
 * @brief Inputtrace
 */
inline InputTrace* inputTrace = nullptr;

/**
 * @brief The message queue
 */
inline MessQueue messageQueue;

/**
 * @brief The simulation current time
 */
inline TimeType CurrTime = 0;

/**
 * @brief The total fin number of packets
 */
inline std::size_t TotalFin = 0;

/**
 * @brief Routing period set.
 */
inline std::unordered_set<TimeType> RoutingPeriods;

/**
 * @brief Abandoned packet IDs.
 */
inline std::unordered_set<TPacketId> AbandonedPackets;

/**
 * @brief The vc mask
 */
inline std::vector<u_int64_t> VC_MASK;

/**
 * @brief Get the current time of the simulation
 * @return The current time
 */
inline TimeType getCurrTime() {
    return Global::CurrTime;
}

/**
 * @brief Set the current time of the simulation
 * @param time The new time
 */
inline void setCurrTime(TimeType time) {
# ifdef INTERCHIPLET
    std::cout << "[INTERCMD] CYCLE " << time << std::endl;
# endif
    Global::CurrTime = time;
}

/**
 * @brief Get the total fin number of packets
 * @return The total fin number of packets
 */
inline std::size_t getTotalFin() {
    return Global::TotalFin;
}

/**
 * @brief Increase the total fin number of packets
 */
inline void incTotalFin() {
    Global::TotalFin += 1;
}

/**
 * @brief Add a transaction to the transaction list
 * @param trans The transaction
 * @return The packet
 */
inline SPacket addTrans(const ProtoPacket& trans) {
    return ___add_trans___(trans);
}

/**
 * @brief Get a transaction from the transaction list
 * @param id The id of the transaction
 * @return The transaction
 */
inline ProtoStateMachine& getTrans(TPacketId id) {
    return ___get_trans___(id);
}

};


# endif

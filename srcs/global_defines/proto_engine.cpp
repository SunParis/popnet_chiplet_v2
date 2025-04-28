# include "global_defines/proto_engine.h"

std::vector<ProtoStateMachine> __trans_list__;

ProtoStateMachine::ProtoStateMachine(const ProtoPacket& trans)
:   src_time(trans.src_time),
    des_time(trans.des_time),
    src_addr(trans.src_addr),
    des_addr(trans.des_addr),
    packet_size(trans.packet_size),
    protoDesc(trans.proto_dsc),
    id(trans.id),
    status(ProtoState::IDLE)
{}

/**
 * @brief Add a transaction to the transaction list
 * @param trans The transaction
 * @return The packet
 */
SPacket ___add_trans___(const ProtoPacket& trans) {
    ProtoStateMachine sm(trans);
    sm.status = ProtoState::DATA_TRANS;

    // Generate first packet.
    SPacket packet(trans.src_addr.size());
    packet.start_time = trans.src_time;
    packet.src_addr = trans.src_addr;
    packet.des_addr = trans.des_addr;
    if (sm.protoDesc & (SPD_BARRIER | SPD_LOCK | SPD_UNLOCK)) {
        for (auto& it: packet.des_addr) {
            it = 0;
        }
    }
    packet.packet_size = trans.packet_size;
    packet.id = trans.id;
    __trans_list__.push_back(sm);
    return packet;
}

/**
 * @brief Get a transaction from the transaction list
 * @param id The id of the transaction
 * @return The transaction
 */
ProtoStateMachine& ___get_trans___(TPacketId id) {
    for (auto& it: __trans_list__) {
        if (it.id == id) {
            return it;
        }
    }
    Sassert(false, "No such transaction.");
    return *(__trans_list__.end());
}
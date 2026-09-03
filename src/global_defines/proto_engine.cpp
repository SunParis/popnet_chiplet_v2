#include "global_defines/proto_engine.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

ProtoStateMachine::ProtoStateMachine(const ProtoPacket& transaction)
    : src_time(transaction.src_time), des_time(transaction.des_time),
      src_addr(transaction.src_addr), des_addr(transaction.des_addr),
      packet_size(transaction.packet_size), protoDesc(transaction.proto_dsc), id(transaction.id),
      status(ProtoState::IDLE) {}

SPacket ProtocolEngine::add(const ProtoPacket& transaction) {
    ProtoStateMachine state(transaction);
    state.status = ProtoState::DATA_TRANS;

    SPacket packet(transaction.src_addr.size());
    packet.start_time = transaction.src_time;
    packet.src_addr = transaction.src_addr;
    packet.des_addr = transaction.des_addr;
    if ((state.protoDesc & (SPD_BARRIER | SPD_LOCK | SPD_UNLOCK)) != 0) {
        std::fill(packet.des_addr.begin(), packet.des_addr.end(), 0);
    }
    packet.packet_size = transaction.packet_size;
    packet.id = transaction.id;

    const auto index = transactions_.size();
    const auto [unused, inserted] = index_by_id_.emplace(transaction.id, index);
    if (!inserted) {
        throw std::runtime_error("Duplicate transaction ID: " + std::to_string(transaction.id));
    }
    transactions_.push_back(std::move(state));
    return packet;
}

ProtoStateMachine& ProtocolEngine::get(TPacketId id) {
    const auto found = index_by_id_.find(id);
    if (found == index_by_id_.end()) {
        throw std::out_of_range("No such transaction: " + std::to_string(id));
    }
    return transactions_[found->second];
}

const ProtoStateMachine& ProtocolEngine::get(TPacketId id) const {
    const auto found = index_by_id_.find(id);
    if (found == index_by_id_.end()) {
        throw std::out_of_range("No such transaction: " + std::to_string(id));
    }
    return transactions_[found->second];
}

void ProtocolEngine::clear() noexcept {
    transactions_.clear();
    index_by_id_.clear();
}

std::size_t ProtocolEngine::size() const noexcept {
    return transactions_.size();
}

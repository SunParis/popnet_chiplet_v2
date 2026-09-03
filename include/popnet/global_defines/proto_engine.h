#pragma once

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "global_defines/defines.h"
#include "global_defines/packet_defines.h"

class ProtoStateMachine {
  public:
    explicit ProtoStateMachine(const ProtoPacket& transaction);

    TimeType src_time;
    TimeType des_time;
    AddrType src_addr;
    AddrType des_addr;
    long packet_size;
    long protoDesc;
    TId id;
    ProtoState status;
    std::vector<TimeType> packetDelay;
};

class ProtocolEngine {
  public:
    SPacket add(const ProtoPacket& transaction);
    ProtoStateMachine& get(TPacketId id);
    const ProtoStateMachine& get(TPacketId id) const;
    void clear() noexcept;
    std::size_t size() const noexcept;

  private:
    std::vector<ProtoStateMachine> transactions_;
    std::unordered_map<TPacketId, std::size_t> index_by_id_;
};

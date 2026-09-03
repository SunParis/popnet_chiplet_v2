#pragma once

#include <cstddef>
#include <deque>
#include <fstream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "global_defines/defines.h"
#include "global_defines/packet_defines.h"
#include "global_defines/proto_engine.h"

class InputTrace {
  public:
    InputTrace(const std::string& trace_file_name, bool sync_protocol_enable, std::size_t dimension,
               ProtocolEngine& protocol_engine, bool follow = false);

    void readTraceFile();
    void addTrace(const SPacket& packet);

    bool isEmpty();
    bool isEmpty(const AddrType& address) const;
    bool isReadFin() const noexcept;

    void popFront();
    void popFront(const AddrType& address);

    const SPacket& front() const;
    const SPacket& front(const AddrType& address) const;
    std::size_t packetCount() const noexcept;

  private:
    struct PacketLater {
        bool operator()(const SPacket* left, const SPacket* right) const noexcept;
    };

    using PacketQueue =
        std::priority_queue<const SPacket*, std::vector<const SPacket*>, PacketLater>;

    bool readAddress(AddrType& address);
    [[noreturn]] void malformedRecord(std::streamoff offset) const;

    std::string trace_file_name_;
    std::ifstream trace_file_;
    std::streamoff has_read_{0};
    bool sync_protocol_enable_;
    bool follow_;
    bool read_end_{false};
    std::size_t count_{0};
    std::size_t dimension_;
    ProtocolEngine& protocol_engine_;
    std::deque<SPacket> packets_;
    std::unordered_map<AddrType, PacketQueue, AddrTypeHash> router_traces_;
    PacketQueue input_traces_;
};

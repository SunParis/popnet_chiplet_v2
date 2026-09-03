#pragma once

#include <cstddef>
#include <optional>
#include <set>
#include <unordered_set>
#include <vector>

#include "global_defines/RGen.h"
#include "global_defines/message_define.h"
#include "global_defines/proto_engine.h"

class SimulationState {
  public:
    SimulationState(long physical_ports, long virtual_channels, std::optional<long> seed);

    TimeType currentTime() const noexcept;
    void setCurrentTime(TimeType time) noexcept;

    std::size_t totalFinished() const noexcept;
    std::size_t totalResolved() const noexcept;
    std::size_t totalAbandoned() const noexcept;
    void markFinished() noexcept;
    bool markAbandoned(TPacketId packet_id);
    bool isAbandoned(TPacketId packet_id) const noexcept;
    void ensureVcMasks(std::size_t count);

    RGen random;
    MessQueue events;
    ProtocolEngine protocols;
    std::set<TimeType> routingPeriods;
    std::vector<AtomType> vcMasks;

  private:
    TimeType current_time_{0};
    std::size_t total_finished_{0};
    std::unordered_set<TPacketId> abandoned_packets_;
};

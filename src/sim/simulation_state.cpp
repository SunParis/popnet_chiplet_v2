#include "sim/simulation_state.h"

#include <limits>
#include <stdexcept>

SimulationState::SimulationState(long physical_ports, long virtual_channels,
                                 std::optional<long> seed) {
    if (seed.has_value()) {
        random.reset_seed(*seed);
    }

    const auto mask_count = physical_ports * virtual_channels;
    if (mask_count < 0 || mask_count > std::numeric_limits<AtomType>::digits) {
        throw std::invalid_argument(
            "physical ports multiplied by virtual channels must be in [0, 64]");
    }

    ensureVcMasks(static_cast<std::size_t>(mask_count));
}

TimeType SimulationState::currentTime() const noexcept {
    return current_time_;
}

void SimulationState::setCurrentTime(TimeType time) noexcept {
#ifdef INTERCHIPLET
    std::cout << "[INTERCMD] CYCLE " << time << std::endl;
#endif
    current_time_ = time;
}

std::size_t SimulationState::totalFinished() const noexcept {
    return total_finished_;
}

std::size_t SimulationState::totalResolved() const noexcept {
    return total_finished_ + abandoned_packets_.size();
}

std::size_t SimulationState::totalAbandoned() const noexcept {
    return abandoned_packets_.size();
}

void SimulationState::markFinished() noexcept {
    ++total_finished_;
}

bool SimulationState::markAbandoned(TPacketId packet_id) {
    return abandoned_packets_.insert(packet_id).second;
}

bool SimulationState::isAbandoned(TPacketId packet_id) const noexcept {
    return abandoned_packets_.contains(packet_id);
}

void SimulationState::ensureVcMasks(std::size_t count) {
    if (count > std::numeric_limits<AtomType>::digits) {
        throw std::invalid_argument("router port/VC count must not exceed 64");
    }
    vcMasks.reserve(count);
    while (vcMasks.size() < count) {
        vcMasks.push_back(AtomType{1} << vcMasks.size());
    }
}

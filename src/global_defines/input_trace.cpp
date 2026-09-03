#include "global_defines/input_trace.h"

#include <sstream>
#include <stdexcept>

#include "logger/logger.hpp"

bool InputTrace::PacketLater::operator()(const SPacket* left, const SPacket* right) const noexcept {
    if (left->start_time != right->start_time) {
        return left->start_time > right->start_time;
    }
    return left->id > right->id;
}

InputTrace::InputTrace(const std::string& trace_file_name, bool sync_protocol_enable,
                       std::size_t dimension, ProtocolEngine& protocol_engine, bool follow)
    : trace_file_name_(trace_file_name), trace_file_(trace_file_name),
      sync_protocol_enable_(sync_protocol_enable), follow_(follow), dimension_(dimension),
      protocol_engine_(protocol_engine) {
    if (!trace_file_.is_open()) {
        throw std::runtime_error("Failed to open trace file: " + trace_file_name_);
    }
}

bool InputTrace::readAddress(AddrType& address) {
    address.resize(dimension_);
    for (auto& coordinate : address) {
        if (!(trace_file_ >> coordinate)) {
            return false;
        }
    }
    return true;
}

void InputTrace::readTraceFile() {
    if (read_end_) {
        return;
    }

    trace_file_.clear();
    trace_file_.seekg(has_read_, std::ios::beg);
    if (!trace_file_) {
        throw std::runtime_error("Failed to seek trace file: " + trace_file_name_);
    }

    const auto first_new_packet = count_;
    while (true) {
        trace_file_ >> std::ws;
        const auto record_offset = trace_file_.tellg();

        TimeType start_time{};
        if (!(trace_file_ >> start_time)) {
            trace_file_.clear();
            trace_file_.seekg(has_read_, std::ios::beg);
            if (!follow_) {
                read_end_ = true;
            }
            break;
        }

        if (start_time == -1) {
            read_end_ = true;
            const auto position = trace_file_.tellg();
            if (position >= 0) {
                has_read_ = position;
            }
            break;
        }

        if (!sync_protocol_enable_) {
            SPacket packet(dimension_);
            packet.start_time = start_time;
            if (!readAddress(packet.src_addr) || !readAddress(packet.des_addr) ||
                !(trace_file_ >> packet.packet_size)) {
                if (!follow_) {
                    malformedRecord(record_offset);
                }
                trace_file_.clear();
                trace_file_.seekg(record_offset, std::ios::beg);
                break;
            }
            packet.id = count_++;
            addTrace(packet);
        } else {
            ProtoPacket packet(dimension_);
            packet.src_time = start_time;
            if (!(trace_file_ >> packet.des_time) || !readAddress(packet.src_addr) ||
                !readAddress(packet.des_addr) ||
                !(trace_file_ >> packet.packet_size >> packet.proto_dsc)) {
                if (!follow_) {
                    malformedRecord(record_offset);
                }
                trace_file_.clear();
                trace_file_.seekg(record_offset, std::ios::beg);
                break;
            }
            packet.id = count_++;
            addTrace(protocol_engine_.add(packet));
        }

        const auto position = trace_file_.tellg();
        if (position >= 0) {
            has_read_ = position;
        }
    }

    Logger::info("Read packets: {}", count_ - first_new_packet);
}

void InputTrace::addTrace(const SPacket& packet) {
    packets_.push_back(packet);
    const auto* stored_packet = &packets_.back();
    input_traces_.push(stored_packet);
    router_traces_[stored_packet->src_addr].push(stored_packet);
}

bool InputTrace::isEmpty() {
    readTraceFile();
    return input_traces_.empty();
}

bool InputTrace::isEmpty(const AddrType& address) const {
    const auto found = router_traces_.find(address);
    return found == router_traces_.end() || found->second.empty();
}

bool InputTrace::isReadFin() const noexcept {
    return read_end_;
}

void InputTrace::popFront() {
    if (input_traces_.empty()) {
        throw std::out_of_range("Input trace is empty");
    }
    input_traces_.pop();
}

void InputTrace::popFront(const AddrType& address) {
    const auto found = router_traces_.find(address);
    if (found == router_traces_.end() || found->second.empty()) {
        throw std::out_of_range("Address not found in router traces");
    }
    found->second.pop();
}

const SPacket& InputTrace::front() const {
    if (input_traces_.empty()) {
        throw std::out_of_range("Input trace is empty");
    }
    return *input_traces_.top();
}

const SPacket& InputTrace::front(const AddrType& address) const {
    const auto found = router_traces_.find(address);
    if (found == router_traces_.end() || found->second.empty()) {
        throw std::out_of_range("Address not found in router traces");
    }
    return *found->second.top();
}

std::size_t InputTrace::packetCount() const noexcept {
    return count_;
}

[[noreturn]] void InputTrace::malformedRecord(std::streamoff offset) const {
    std::ostringstream message;
    message << "Malformed trace record at byte " << offset << " in " << trace_file_name_;
    throw std::runtime_error(message.str());
}

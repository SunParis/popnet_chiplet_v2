#include "router/modules.h"

#include <array>
#include <stdexcept>
#include <utility>

extern "C" {
#include "SIM_power.h"
#include "SIM_power_router.h"
#include "SIM_router_power.h"
}

#ifndef FUNC
#define FUNC(...) // NOLINT
#endif

InputModules::InputModules(long physical_ports, long virtual_channels)
    : physical_ports_(physical_ports), virtual_channels_(virtual_channels),
      input_(static_cast<std::size_t>(physical_ports * virtual_channels)),
      states_(static_cast<std::size_t>(physical_ports * virtual_channels), VCStateType::INIT),
      routing_(static_cast<std::size_t>(physical_ports * virtual_channels)),
      selected_routing_(static_cast<std::size_t>(physical_ports * virtual_channels), VC_NULL) {
    for (auto& candidates : routing_) {
        candidates.reserve(static_cast<std::size_t>(virtual_channels));
    }
}

bool InputModules::isIBuffFull() const noexcept {
    return injection_buffer_full_;
}

void InputModules::setIBuffFull(bool full) noexcept {
    injection_buffer_full_ = full;
}

std::size_t InputModules::getBufferSize(long port, long vc) const {
    return input_[index(port, vc)].size();
}

void InputModules::setState(long port, long vc, VCStateType state) {
    states_[index(port, vc)] = state;
}

VCStateType InputModules::getState(long port, long vc) const {
    return states_[index(port, vc)];
}

void InputModules::addFlit(long port, long vc, const Flit& flit) {
    input_[index(port, vc)].push_back(flit);
}

void InputModules::addFlit(long port, long vc, Flit&& flit) {
    input_[index(port, vc)].push_back(std::move(flit));
}

Flit& InputModules::getFlit(long port, long vc) {
    auto& buffer = input_[index(port, vc)];
    if (buffer.empty()) {
        throw std::out_of_range("Input VC buffer is empty");
    }
    return buffer.front();
}

const Flit& InputModules::getFlit(long port, long vc) const {
    const auto& buffer = input_[index(port, vc)];
    if (buffer.empty()) {
        throw std::out_of_range("Input VC buffer is empty");
    }
    return buffer.front();
}

void InputModules::removeFlit(long port, long vc) {
    auto& buffer = input_[index(port, vc)];
    if (buffer.empty()) {
        throw std::out_of_range("Input VC buffer is empty");
    }
    buffer.pop_front();
}

std::vector<VCType>& InputModules::getRouting(long port, long vc) {
    return routing_[index(port, vc)];
}

const std::vector<VCType>& InputModules::getRouting(long port, long vc) const {
    return routing_[index(port, vc)];
}

void InputModules::addRouting(long port, long vc, VCType route) {
    routing_[index(port, vc)].push_back(route);
}

void InputModules::clearRouting(long port, long vc) {
    routing_[index(port, vc)].clear();
}

void InputModules::setCRouting(long port, long vc, VCType route) {
    selected_routing_[index(port, vc)] = route;
}

VCType InputModules::getCRouting(long port, long vc) const {
    return selected_routing_[index(port, vc)];
}

std::size_t InputModules::index(long port, long vc) const {
    if (port < 0 || port >= physical_ports_ || vc < 0 || vc >= virtual_channels_) {
        throw std::out_of_range("Input port or virtual channel is out of range");
    }
    return static_cast<std::size_t>(port * virtual_channels_ + vc);
}

OutputModules::OutputModules(long physical_ports, long virtual_channels, long input_buffer_size,
                             long output_buffer_size)
    : physical_ports_(physical_ports), virtual_channels_(virtual_channels),
      credits_(static_cast<std::size_t>(physical_ports * virtual_channels), input_buffer_size),
      usage_(static_cast<std::size_t>(physical_ports * virtual_channels), VCUsageType::FREE),
      output_buffers_(static_cast<std::size_t>(physical_ports)),
      output_addresses_(static_cast<std::size_t>(physical_ports)),
      local_credits_(static_cast<std::size_t>(physical_ports), output_buffer_size) {}

void OutputModules::incCounter(long port, long vc) {
    ++credits_[channelIndex(port, vc)];
}

void OutputModules::decCounter(long port, long vc) {
    --credits_[channelIndex(port, vc)];
}

long OutputModules::getCounter(long port, long vc) const {
    return credits_[channelIndex(port, vc)];
}

long OutputModules::getLocalCounter(long port) const {
    return local_credits_[portIndex(port)];
}

VCUsageType OutputModules::getUsage(long port, long vc) const {
    return usage_[channelIndex(port, vc)];
}

void OutputModules::acquireChannel(long port, long vc, VCType input) {
    static_cast<void>(input);
    usage_[channelIndex(port, vc)] = VCUsageType::USED;
}

void OutputModules::releaseChannel(long port, long vc) {
    usage_[channelIndex(port, vc)] = VCUsageType::FREE;
}

std::size_t OutputModules::getOutBufferSize(long port) const {
    return output_buffers_[portIndex(port)].size();
}

void OutputModules::addFlit(long port, const Flit& flit) {
    const auto port_index = portIndex(port);
    output_buffers_[port_index].push_back(flit);
    --local_credits_[port_index];
}

void OutputModules::addFlit(long port, Flit&& flit) {
    const auto port_index = portIndex(port);
    output_buffers_[port_index].push_back(std::move(flit));
    --local_credits_[port_index];
}

Flit& OutputModules::getFlit(long port) {
    auto& buffer = output_buffers_[portIndex(port)];
    if (buffer.empty()) {
        throw std::out_of_range("Output buffer is empty");
    }
    return buffer.front();
}

const Flit& OutputModules::getFlit(long port) const {
    const auto& buffer = output_buffers_[portIndex(port)];
    if (buffer.empty()) {
        throw std::out_of_range("Output buffer is empty");
    }
    return buffer.front();
}

void OutputModules::removeFlit(long port) {
    const auto port_index = portIndex(port);
    auto& buffer = output_buffers_[port_index];
    if (buffer.empty()) {
        throw std::out_of_range("Output buffer is empty");
    }
    buffer.pop_front();
    ++local_credits_[port_index];
}

void OutputModules::addAddr(long port, const VCType& address) {
    output_addresses_[portIndex(port)].push_back(address);
}

VCType OutputModules::getAddr(long port) const {
    const auto& addresses = output_addresses_[portIndex(port)];
    if (addresses.empty()) {
        throw std::out_of_range("Output address buffer is empty");
    }
    return addresses.front();
}

void OutputModules::removeAddr(long port) {
    auto& addresses = output_addresses_[portIndex(port)];
    if (addresses.empty()) {
        throw std::out_of_range("Output address buffer is empty");
    }
    addresses.pop_front();
}

std::size_t OutputModules::channelIndex(long port, long vc) const {
    if (port < 0 || port >= physical_ports_ || vc < 0 || vc >= virtual_channels_) {
        throw std::out_of_range("Output port or virtual channel is out of range");
    }
    return static_cast<std::size_t>(port * virtual_channels_ + vc);
}

std::size_t OutputModules::portIndex(long port) const {
    if (port < 0 || port >= physical_ports_) {
        throw std::out_of_range("Output port is out of range");
    }
    return static_cast<std::size_t>(port);
}

struct PowerModules::OrionState {
    SIM_power_router_info_t router_info{};
    SIM_power_router_t router_power{};
    SIM_power_arbiter_t arbiter_vc_power{};
    SIM_power_bus_t link_power{};
};

PowerModules::PowerModules(long physical_ports, long virtual_channels, long flit_size,
                           double link_length)
    : flit_size_(flit_size), orion_(std::make_unique<OrionState>()),
      buffer_write_(static_cast<std::size_t>(physical_ports),
                    DataType(static_cast<std::size_t>(flit_size), 0)),
      buffer_read_(static_cast<std::size_t>(physical_ports),
                   DataType(static_cast<std::size_t>(flit_size), 0)),
      crossbar_read_(static_cast<std::size_t>(physical_ports),
                     DataType(static_cast<std::size_t>(flit_size), 0)),
      crossbar_write_(static_cast<std::size_t>(physical_ports),
                      DataType(static_cast<std::size_t>(flit_size), 0)),
      link_traversal_(static_cast<std::size_t>(physical_ports),
                      DataType(static_cast<std::size_t>(flit_size), 0)),
      crossbar_input_(static_cast<std::size_t>(physical_ports), 0),
      arbiter_vc_req_(static_cast<std::size_t>(physical_ports),
                      std::vector<AtomType>(static_cast<std::size_t>(virtual_channels), 1)),
      arbiter_vc_grant_(static_cast<std::size_t>(physical_ports),
                        std::vector<unsigned long>(static_cast<std::size_t>(virtual_channels), 1)) {
    FUNC(SIM_router_power_init, &orion_->router_info, &orion_->router_power);
    SIM_arbiter_init(&orion_->arbiter_vc_power, 1, 1, physical_ports * virtual_channels, 0,
                     nullptr);
    SIM_bus_init(&orion_->link_power, GENERIC_BUS, IDENT_ENC, ATOM_WIDTH_, 0, 1, 1, link_length, 0);
}

PowerModules::~PowerModules() = default;

void PowerModules::addBufferReadPwr(long input_port, const DataType& data) {
    for (long index = 0; index < flit_size_; ++index) {
        FUNC(SIM_buf_power_data_read, &orion_->router_info.in_buf_info,
             &orion_->router_power.in_buf, data[static_cast<std::size_t>(index)]);
        buffer_read_[static_cast<std::size_t>(input_port)][static_cast<std::size_t>(index)] =
            data[static_cast<std::size_t>(index)];
    }
}

void PowerModules::addBufferWritePwr(long input_port, const DataType& data) {
    for (long index = 0; index < flit_size_; ++index) {
        auto& old_data =
            buffer_write_[static_cast<std::size_t>(input_port)][static_cast<std::size_t>(index)];
        const AtomType new_data = data[static_cast<std::size_t>(index)];
        std::array<AtomType, 4> orion_data{old_data, new_data, old_data, new_data};
        FUNC(SIM_buf_power_data_write, &orion_->router_info.in_buf_info,
             &orion_->router_power.in_buf, reinterpret_cast<char*>(&orion_data[0]),
             reinterpret_cast<char*>(&orion_data[0]), reinterpret_cast<char*>(&orion_data[1]));
        old_data = new_data;
    }
}

void PowerModules::addCrossbarTravPwr(long input_port, long output_port, const DataType& data) {
    for (long index = 0; index < flit_size_; ++index) {
        const auto element = static_cast<std::size_t>(index);
        const auto input = static_cast<std::size_t>(input_port);
        const auto output = static_cast<std::size_t>(output_port);
        SIM_crossbar_record(&orion_->router_power.crossbar, 1, data[element],
                            crossbar_read_[input][element], 1, 1);
        SIM_crossbar_record(&orion_->router_power.crossbar, 0, data[element],
                            crossbar_write_[output][element], crossbar_input_[output], input_port);
        crossbar_read_[input][element] = data[element];
        crossbar_write_[output][element] = data[element];
        crossbar_input_[output] = input_port;
    }
}

void PowerModules::addVCArbitPwr(long port, long vc, AtomType request, unsigned long grant) {
    const auto port_index = static_cast<std::size_t>(port);
    const auto vc_index = static_cast<std::size_t>(vc);
    SIM_arbiter_record(&orion_->arbiter_vc_power, request, arbiter_vc_req_[port_index][vc_index],
                       grant, arbiter_vc_grant_[port_index][vc_index]);
    arbiter_vc_req_[port_index][vc_index] = request;
    arbiter_vc_grant_[port_index][vc_index] = grant;
}

void PowerModules::addLinkTravPwr(long input_port, const DataType& data) {
    auto& previous = link_traversal_[static_cast<std::size_t>(input_port)];
    for (long index = 0; index < flit_size_; ++index) {
        const auto element = static_cast<std::size_t>(index);
        SIM_bus_record(&orion_->link_power, previous[element], data[element]);
        previous[element] = data[element];
    }
}

double PowerModules::getBufferPower() {
    return SIM_array_power_report(&orion_->router_info.in_buf_info, &orion_->router_power.in_buf);
}

double PowerModules::getLinkPower() {
    return SIM_bus_report(&orion_->link_power);
}

double PowerModules::getCrossbarPower() {
    return SIM_crossbar_report(&orion_->router_power.crossbar);
}

double PowerModules::getArbiterPower() {
    return SIM_arbiter_report(&orion_->arbiter_vc_power);
}

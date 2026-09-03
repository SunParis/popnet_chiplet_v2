#include "sim/Sim.h"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <thread>

#include "global_defines/input_trace.h"
#include "io/delay_writer.h"
#include "logger/logger.hpp"
#include "preprocess/config.h"
#include "router/router.h"
#include "router/topo_router.h"
#include "sim/simulation_state.h"

class Sim::Impl {
  public:
    explicit Impl(const Config& config);

    void mainProcess();
    SimulationResults results();
    void getResults();
    const SimulationState& state() const noexcept;

  private:
    static constexpr std::size_t message_type_count = 5;

    void createTopology();
    void createRouters();
    void setInitEvent();
    void receiveEVG(const MessEvent& message);
    void receiveRouter(const MessEvent& message);
    void receiveWire(MessEvent& message);
    void receiveCredit(const MessEvent& message);
    void receiveReconfiguration(const MessEvent& message);
    BaseRouter& router(const AddrType& address);
    bool waitForNewTrace();

    Config config_;
    SimulationState state_;
    InputTrace input_trace_;
    DelayWriter delay_writer_;
    std::size_t router_count_{0};
    RouterList inter_network_;
    std::unique_ptr<TopoInfo> topology_;
    std::unique_ptr<ReconfigTopoInfo> reconfigurable_topology_;
    bool has_run_{false};
};

Sim::Impl::Impl(const Config& config)
    : config_(config), state_(config_.getPhysicalPortNumber(), config_.getVirtualChannelNumber(),
                              config_.getRandomSeed()),
      input_trace_(config_.getTraceFname(), config_.isSyncProtocolEnable(),
                   static_cast<std::size_t>(config_.getCubeNumber()), state_.protocols,
                   config_.isEndWithMinus1()),
      delay_writer_(config_.getDelayFname()) {
    createTopology();
    createRouters();

    input_trace_.readTraceFile();
    if (!input_trace_.isEmpty()) {
        state_.events.addMessage(MessEvent(input_trace_.front().start_time, MessType::EVG));
    }
}

void Sim::Impl::createTopology() {
    std::size_t configured_router_count = 1;
    for (long dimension = 0; dimension < config_.getCubeNumber(); ++dimension) {
        const auto vertices = static_cast<std::size_t>(config_.getAryNumber());
        if (configured_router_count > std::numeric_limits<std::size_t>::max() / vertices) {
            throw std::overflow_error("Configured router count is too large");
        }
        configured_router_count *= vertices;
    }

    switch (config_.getRoutingAlg()) {
    case RoutingType::GRAPH_TOPO:
        topology_ = std::make_unique<TopoInfo>(config_.getTopoFilePath());
        router_count_ = static_cast<std::size_t>(topology_->vertexCnt);
        state_.routingPeriods.insert(topology_->routingPeriods.begin(),
                                     topology_->routingPeriods.end());
        break;
    case RoutingType::RECONFIGURABLE_GRAPH_TOPO:
        reconfigurable_topology_ = std::make_unique<ReconfigTopoInfo>(
            config_.getTopoFilePath(), config_.getReconfigFilePath());
        router_count_ = static_cast<std::size_t>(reconfigurable_topology_->vertexCnt);
        state_.routingPeriods.insert(reconfigurable_topology_->routingPeriods.begin(),
                                     reconfigurable_topology_->routingPeriods.end());
        break;
    default:
        router_count_ = configured_router_count;
        break;
    }

    if (router_count_ != configured_router_count) {
        throw std::invalid_argument(
            "vertices^dimension must equal the number of vertices in the topology");
    }
    for (const auto period : state_.routingPeriods) {
        if (!std::isfinite(period) || period <= 0) {
            throw std::invalid_argument("Topology pipeline_stage_delay must be greater than zero");
        }
    }
}

void Sim::Impl::createRouters() {
    AddrType address(static_cast<std::size_t>(config_.getCubeNumber()), 0);
    inter_network_.reserve(router_count_);

    for (std::size_t index = 0; index < router_count_; ++index) {
        switch (config_.getRoutingAlg()) {
        case RoutingType::XY:
            inter_network_.push_back(std::make_unique<CXYRouter>(
                config_, address, inter_network_, state_, input_trace_, delay_writer_));
            break;
        case RoutingType::TXY:
            inter_network_.push_back(std::make_unique<CTXYRouter>(
                config_, address, inter_network_, state_, input_trace_, delay_writer_));
            break;
        case RoutingType::CHIPLET_ROUTING_MESH:
            inter_network_.push_back(std::make_unique<CChipletMeshRouter>(
                config_, address, inter_network_, state_, input_trace_, delay_writer_));
            break;
        case RoutingType::CHIPLET_STAR_TOPO_ROUTING:
            inter_network_.push_back(std::make_unique<CChipletStarRouter>(
                config_, address, inter_network_, state_, input_trace_, delay_writer_));
            break;
        case RoutingType::GRAPH_TOPO:
            inter_network_.push_back(std::make_unique<CGraphTopo>(
                config_, address, inter_network_, *topology_, state_, input_trace_, delay_writer_));
            break;
        case RoutingType::RECONFIGURABLE_GRAPH_TOPO:
            inter_network_.push_back(std::make_unique<CReconfigTopoRouter>(
                config_, address, inter_network_, *reconfigurable_topology_, state_, input_trace_,
                delay_writer_));
            break;
        }

        for (std::size_t coordinate = address.size(); coordinate-- > 0;) {
            ++address[coordinate];
            if (address[coordinate] < config_.getAryNumber()) {
                break;
            }
            address[coordinate] = 0;
        }
    }
}

void Sim::Impl::setInitEvent() {
    switch (config_.getRoutingAlg()) {
    case RoutingType::RECONFIGURABLE_GRAPH_TOPO:
        for (const auto period : state_.routingPeriods) {
            state_.events.addMessage(MessEvent(-S_ELPS_ / 2, MessType::ROUTER, period));
        }
        if (reconfigurable_topology_->hasNextReconfiguration()) {
            state_.events.addMessage(MessEvent(0, MessType::RECONFIGURATION));
        }
        break;
    case RoutingType::GRAPH_TOPO:
        for (const auto period : state_.routingPeriods) {
            state_.events.addMessage(MessEvent(0, MessType::ROUTER, period));
        }
        break;
    default:
        state_.events.addMessage(MessEvent(0, MessType::ROUTER));
        break;
    }
}

void Sim::Impl::receiveEVG(const MessEvent&) {
    const auto source = input_trace_.front().src_addr;
    router(source).recvPacket();
    input_trace_.popFront();
    if (!input_trace_.isEmpty()) {
        state_.events.addMessage(MessEvent(input_trace_.front().start_time, MessType::EVG));
    }
}

void Sim::Impl::receiveRouter(const MessEvent& message) {
    const auto period = message.getRoutingPeriod();
    state_.events.addMessage(MessEvent(message.getEventStart() + period, MessType::ROUTER, period));
    for (auto& router_ptr : inter_network_) {
        router_ptr->routingPipeStage(period);
    }
}

void Sim::Impl::receiveWire(MessEvent& message) {
    router(message.getDes())
        .recvFlit(message.getPC(), message.getVC(), std::move(message.getFlit()));
}

void Sim::Impl::receiveCredit(const MessEvent& message) {
    router(message.getDes()).recvCredit(message.getPC(), message.getVC());
}

void Sim::Impl::receiveReconfiguration(const MessEvent& message) {
    const auto period_number = reconfigurable_topology_->getCurrentReconfigurationPeriod();
    const auto next_time =
        message.getEventStart() + reconfigurable_topology_->getReconfigurationPeriod();
    reconfigurable_topology_->reconfigurate(message.getEventStart(), next_time);
    Logger::info("Enter reconfiguration period {}.", period_number);
    if (reconfigurable_topology_->hasNextReconfiguration()) {
        state_.events.addMessage(MessEvent(next_time, MessType::RECONFIGURATION));
    }
}

BaseRouter& Sim::Impl::router(const AddrType& address) {
    if (address.size() != static_cast<std::size_t>(config_.getCubeNumber())) {
        throw std::out_of_range("Router address has the wrong dimension");
    }

    std::size_t index = 0;
    for (const auto coordinate : address) {
        if (coordinate < 0 || coordinate >= config_.getAryNumber()) {
            throw std::out_of_range("Router coordinate is outside the configured network");
        }
        index = static_cast<std::size_t>(coordinate) +
                index * static_cast<std::size_t>(config_.getAryNumber());
    }
    if (index >= inter_network_.size()) {
        throw std::out_of_range("Router address does not exist");
    }
    return *inter_network_[index];
}

void Sim::Impl::mainProcess() {
    if (has_run_) {
        throw std::logic_error("A simulation instance can only be run once");
    }
    has_run_ = true;

    long total_incoming = 0;
    setInitEvent();

    while (!state_.events.empty()) {
        if (state_.events.getTop().getEventStart() > config_.getSimLength() + S_ELPS_) {
            break;
        }

        MessEvent current_message = state_.events.popMessage();
        state_.setCurrentTime(current_message.getEventStart());

        Logger::debug("Get a message:: {}.", Logger::stream_to_string<MessEvent>(current_message));
        switch (current_message.getEventType()) {
        case MessType::EVG:
            receiveEVG(current_message);
            ++total_incoming;
            break;
        case MessType::ROUTER:
            receiveRouter(current_message);
            break;
        case MessType::WIRE:
            Logger::info("From Router {} to Router {} Port {} Virtual Channel {}.",
                         Logger::stream_to_string<AddrType>(current_message.getSrc()),
                         Logger::stream_to_string<AddrType>(current_message.getDes()),
                         current_message.getPC(), current_message.getVC());
            receiveWire(current_message);
            break;
        case MessType::CREDIT:
            Logger::info("From Router {} to Router {} Port {} Virtual Channel {}.",
                         Logger::stream_to_string<AddrType>(current_message.getSrc()),
                         Logger::stream_to_string<AddrType>(current_message.getDes()),
                         current_message.getPC(), current_message.getVC());
            receiveCredit(current_message);
            break;
        case MessType::RECONFIGURATION:
            receiveReconfiguration(current_message);
            break;
        }

        if (static_cast<std::size_t>(total_incoming) != state_.totalResolved()) {
            continue;
        }

        double first_event_time = -1;
        double first_router_event_time = -1;
        while (true) {
            first_event_time = -1;
            first_router_event_time = -1;
            for (std::size_t type = 0; type < message_type_count; ++type) {
                const auto message_type = static_cast<MessType>(type);
                if (state_.events.empty(message_type)) {
                    continue;
                }
                const auto time = state_.events.getTop(message_type).getEventStart();
                if (message_type == MessType::ROUTER) {
                    if (first_router_event_time < 0 || time < first_router_event_time) {
                        first_router_event_time = time;
                    }
                } else if (first_event_time < 0 || time < first_event_time) {
                    first_event_time = time;
                }
            }

            if (first_event_time >= 0 || !waitForNewTrace()) {
                break;
            }
        }

        if (first_event_time < 0) {
            state_.events.clear();
            break;
        }
        if (first_router_event_time < 0) {
            state_.events.clear();
            continue;
        }
        if (first_router_event_time < first_event_time) {
            state_.events.advanceRouterCycles(first_event_time);
            Logger::info("Direct forward to cycle: {}.",
                         state_.events.getTop(MessType::ROUTER).getEventStart());
        }
    }
}

SimulationResults Sim::Impl::results() {
    SimulationResults value;
    value.total_finished = state_.totalFinished();

    double total_delay = 0;
    double raw_memory_power = 0;
    double raw_crossbar_power = 0;
    double raw_arbiter_power = 0;
    double raw_link_power = 0;
    for (auto& router_ptr : inter_network_) {
        total_delay += router_ptr->getTotalDelay();
        raw_memory_power += router_ptr->getBufferPower();
        raw_crossbar_power += router_ptr->getCrossbarPower();
        raw_arbiter_power += router_ptr->getArbiterPower();
        raw_link_power += router_ptr->getLinkPower();
    }

    if (value.total_finished > 0) {
        value.average_delay = total_delay / static_cast<double>(value.total_finished);
    }
    if (state_.currentTime() > 0) {
        const auto scale = POWER_NOM_ / state_.currentTime();
        value.memory_power = raw_memory_power * scale;
        value.crossbar_power = raw_crossbar_power * scale;
        value.arbiter_power = raw_arbiter_power * scale;
        value.link_power = raw_link_power * scale;
    }
    value.total_power =
        value.memory_power + value.crossbar_power + value.arbiter_power + value.link_power;
    return value;
}

void Sim::Impl::getResults() {
    const auto value = results();
    Logger::info("\nTotal finished:       {}.\n"
                 "Average delay:        {:.6g}.\n"
                 "Total memory power:   {:.6g}.\n"
                 "Total crossbar power: {:.6g}.\n"
                 "Total arbiter power:  {:.6g}.\n"
                 "Total link power:     {:.6g}.\n"
                 "Total power:          {:.6g}.",
                 value.total_finished, value.average_delay, value.memory_power,
                 value.crossbar_power, value.arbiter_power, value.link_power, value.total_power);
    delay_writer_.flush();
    Logger::flush();
}

const SimulationState& Sim::Impl::state() const noexcept {
    return state_;
}

bool Sim::Impl::waitForNewTrace() {
    if (input_trace_.isReadFin() || !config_.isEndWithMinus1()) {
        return false;
    }
    while (!input_trace_.isReadFin()) {
        if (!input_trace_.isEmpty()) {
            state_.events.addMessage(MessEvent(input_trace_.front().start_time, MessType::EVG));
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return false;
}

Sim::Sim(const Config& config) : impl_(std::make_unique<Impl>(config)) {}

Sim::~Sim() = default;

void Sim::mainProcess() {
    impl_->mainProcess();
}

SimulationResults Sim::results() {
    return impl_->results();
}

void Sim::getResults() {
    impl_->getResults();
}

const SimulationState& Sim::state() const noexcept {
    return impl_->state();
}

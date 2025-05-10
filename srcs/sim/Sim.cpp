# include "sim/Sim.h"

void Sim::setInitEvent() {
    switch (this->config_.getRoutingAlg()) {
        case RoutingType::RECONFIGURABLE_GRAPH_TOPO:
            Global::messageQueue.addMessage(
                MessEvent(
                    0 - S_ELPS_/  2,
                    MessType::ROUTER
                )
            );
            break;
        case RoutingType::GRAPH_TOPO:
            for (auto& p: Global::RoutingPeriods) {
                Global::messageQueue.addMessage(
                    MessEvent(
                        0,
                        MessType::ROUTER,
                        p
                    )
                );
            }
            break;
        default:
            Global::messageQueue.addMessage(
                MessEvent(
                    0,
                    MessType::ROUTER
                )
            );
            break;
    }
}

void Sim::receive_EVG_message(MessEvent mesg) {
    AddrType tmp = Global::inputTrace->front().src_addr;
    this->router(tmp).recvPacket();
    Global::inputTrace->popFront();
    if (!Global::inputTrace->isEmpty()) {
        Global::messageQueue.addMessage(
            MessEvent(
                Global::inputTrace->front().start_time,
                MessType::EVG
            )
        );
    }
}
    
void Sim::receive_ROUTER_message(MessEvent mesg) {
    TimeType p = mesg.getRoutingPeriod();
    Global::messageQueue.addMessage(
        MessEvent(
            mesg.getEventStart() + p,
            MessType::ROUTER,
            p
        )
    );
    for (auto& router : this->inter_network_) {
        router->routingPipeStage(p);
    }
}

void Sim::receive_WIRE_message(MessEvent mesg) {
    AddrType des_t = mesg.getDes();
    long pc_t = mesg.getPC();
	long vc_t = mesg.getVC();
	Flit& flits_t = mesg.getFlit();
	this->router(des_t).recvFlit(pc_t, vc_t, flits_t);
}

void Sim::receive_CREDIT_message(MessEvent mesg) {
    AddrType des_t = mesg.getDes();
    long pc_t = mesg.getPC();
	long vc_t = mesg.getVC();
	this->router(des_t).recvCredit(pc_t, vc_t);
}

void Sim::receive_RECONFIGURATION_message(MessEvent mesg) {
    TimeType eventTime = mesg.getEventStart();
    TimeType nextReconfigTime = eventTime + this->topo_info.reconfig_topo_info->getCurrentReconfigurationPeriod();
	this->topo_info.reconfig_topo_info->reconfigurate(eventTime, nextReconfigTime);
    Logger::info("Enter reconfiguration period {}.", this->topo_info.reconfig_topo_info->getCurrentReconfigurationPeriod());
    Global::messageQueue.addMessage(
        MessEvent(
            nextReconfigTime,
            MessType::RECONFIGURATION
        )
    );
}

BaseRouter& Sim::router(const AddrType& addr) {
    long index = 0;
    for (auto& iter: addr) {
        index = iter + index * this->config_.getAryNumber();
    }
    return *this->inter_network_[index];
}

Sim::Sim(const Config& config)
:   config_(config),
    last_time_(0),
    mess_count_(0)
{
    if (config.getRandomSeed() != std::nullopt) {
        Global::RandomGen.reset_seed(config.getRandomSeed().value());
    }
    
    Global::inputTrace = new InputTrace(config.getTraceFname(),
        config.isSyncProtocolEnable(), config.getCubeNumber());
    
    this->router_count_ = config.getAryNumber();
    
    for (long i = 0; i < config.getCubeNumber() - 1; i++) {
        this->router_count_ = this->router_count_ * config.getAryNumber();
    }

    AddrType addr;
    addr.resize(config.getCubeNumber(), 0);
    this->inter_network_.reserve(this->router_count_);

    for (long i = 0; i < this->router_count_; i++) {

        switch (config.getRoutingAlg()) {
            case RoutingType::XY:
                this->inter_network_.push_back(
                    new CXYRouter(
                        config, addr,
                        this->inter_network_
                    )
                );
                break;
            case RoutingType::TXY:
                this->inter_network_.push_back(
                    new CTXYRouter(
                        config, addr,
                        this->inter_network_
                    )
                );
                break;
            case RoutingType::CHIPLET_ROUTING_MESH:
                this->inter_network_.push_back(
                    new CChipletMeshRouter(
                        config, addr,
                        this->inter_network_
                    )
                );
                break;
            case RoutingType::CHIPLET_STAR_TOPO_ROUTING:
			    this->inter_network_.push_back(
                    new CChipletStarRouter(
                        config, addr,
                        this->inter_network_
                    )
                );
                break;
		    case RoutingType::GRAPH_TOPO:
			    if (this->topo_info.topo_info == nullptr) {
                    this->topo_info.topo_info = new TopoInfo(
                        config.getTopoFilePath()
                    );
                }
                this->inter_network_.push_back(
                    new CGraphTopo(
                        config, addr,
                        this->inter_network_,
                        *this->topo_info.topo_info
                    )
                );
                break;
		    case RoutingType::RECONFIGURABLE_GRAPH_TOPO:
			    if (this->topo_info.reconfig_topo_info == nullptr) {
                    this->topo_info.reconfig_topo_info = new ReconfigTopoInfo(
                        config.getTopoFilePath(),
                        config.getReconfigFilePath()
                    );
                }
                this->inter_network_.push_back(
                    new CReconfigTopoRouter(
                        config, addr,
                        this->inter_network_,
                        *this->topo_info.reconfig_topo_info
                    )
                );
                break;
            default:
                Sassert(false, "Invalid routing algorithm.");
                break;
        }

        addr[config.getCubeNumber() - 1] += 1;
        for (long j = config.getCubeNumber() - 1; j > 0; j--) {
            if (addr[j] == config.getAryNumber()) {
                addr[j] = 0;
                addr[j - 1] += 1;
            }
        }
    }
    Global::inputTrace->readTraceFile();
    Global::messageQueue.addMessage(
        MessEvent(
            Global::inputTrace->front().start_time,
            MessType::EVG
        )
    );

    Global::VC_MASK.push_back(1);
    for (u_int64_t i = 1; i < config_.getPhysicalPortNumber() * config_.getVirtualChannelNumber(); i++) {
        Global::VC_MASK.push_back(Global::VC_MASK[i - 1] << 1);
    }
}

Sim::~Sim() {
    delete Global::inputTrace;
    for (auto& router: this->inter_network_) {
        delete router;
    }
    if (this->topo_info.topo_info != nullptr) {
        delete this->topo_info.topo_info;
    }
    if (this->topo_info.reconfig_topo_info != nullptr) {
        delete this->topo_info.reconfig_topo_info;
    }
}

void Sim::mainProcess() {
    TimeType report_t = 0;
    long total_incoming = 0;
    this->setInitEvent();

    while (Global::getCurrTime() <= this->config_.getSimLength()) {
        if (Global::messageQueue.empty()) {
            if (Global::inputTrace->isReadFin()) {
                break;
            }
            else if (this->config_.isEndWithMinus1()) {
                while (Global::inputTrace->isEmpty()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
                Global::messageQueue.addMessage(
                    MessEvent(
                        Global::inputTrace->front().start_time,
                        MessType::EVG
                    )
                );
            }
            else {
                break;//qingxiaangdaren daociyiyou
            }
        }
        
        MessEvent current_message(Global::messageQueue.getTop());

        Global::messageQueue.popFront();
        Global::setCurrTime(current_message.getEventStart());
        Sassert(Global::getCurrTime() <= ((current_message.getEventStart()) + S_ELPS_),
            "Current time is greater than event start time.");
        
        Logger::debug("Get a message:: {}.", Logger::stream_to_string<MessEvent>(current_message));
        switch (current_message.getEventType()) {
            case MessType::EVG:
                this->receive_EVG_message(current_message);
                total_incoming++;
                break;
            case MessType::ROUTER:
                this->receive_ROUTER_message(current_message);
                break;
            case MessType::WIRE:
                Logger::info("From Router {} to Router {} Port {} Virtual Channel {}.",
                    Logger::stream_to_string<AddrType>(current_message.getSrc()),
                    Logger::stream_to_string<AddrType>(current_message.getDes()),
                    current_message.getPC(), current_message.getVC());
                this->receive_WIRE_message(current_message);
                break;
            case MessType::CREDIT:
                Logger::info("From Router {} to Router {} Port {} Virtual Channel {}.",
                    Logger::stream_to_string<AddrType>(current_message.getSrc()),
                    Logger::stream_to_string<AddrType>(current_message.getDes()),
                    current_message.getPC(), current_message.getVC());
                this->receive_CREDIT_message(current_message);
                break;
            case MessType::RECONFIGURATION:
                this->receive_RECONFIGURATION_message(current_message);
                break;
            default:
                Sassert(false, "This message type is not supported.");
                break;
        }

        if (total_incoming == Global::getTotalFin()) {
            double first_event_time = -1;
            double first_router_event_time = -1;

            for (unsigned char idx = 0; idx < 5; idx++) {
                if (!Global::messageQueue.empty(MessType(idx))) {
                    if (idx == static_cast<unsigned char>(MessType::ROUTER)) {
                        first_router_event_time =
                            Global::messageQueue.getTop(MessType::ROUTER).getEventStart();
                    }
                    else if (first_event_time == -1
                        || Global::messageQueue.getTop(MessType(idx)).getEventStart() < first_event_time
                    ) {
                        first_event_time = Global::messageQueue.getTop(MessType(idx)).getEventStart();
                    }
                }
            }

            if (first_event_time < 0) {
                Global::messageQueue.clear();
                break;
            }
            if (first_router_event_time < 0) {
                Global::messageQueue.clear();
                continue;
            }

            if (first_router_event_time < first_event_time) {
                double t = (first_event_time - first_router_event_time) / PIPE_DELAY_;
                double new_router_event_time = first_router_event_time + round(t) * PIPE_DELAY_;

                Global::messageQueue.clear(MessType::ROUTER);

                Global::messageQueue.addMessage(
                    MessEvent(
                        new_router_event_time,
                        MessType::ROUTER
                    )
                );
                Global::setCurrTime(new_router_event_time);
                Logger::info("Direct forward to cycle: {}.", Global::getCurrTime());
            }
        }
    }
}

void Sim::getResults() {
    double total_delay = 0;
    double total_mem_power = 0;
    double total_crossbar_power = 0;
    double total_arbiter_power = 0;
    double total_power = 0;
    double total_link_power = 0;
    for (auto& router: this->inter_network_) {
        total_delay += router->getTotalDelay();
        total_mem_power += router->getBufferPower();
        total_crossbar_power += router->getCrossbarPower();
        total_arbiter_power += router->getArbiterPower();
        total_link_power += router->getLinkPower();
    }
    total_mem_power /= Global::getCurrTime();
    total_crossbar_power /= Global::getCurrTime();
    total_link_power /= Global::getCurrTime();
    total_arbiter_power /= Global::getCurrTime();
    total_power = total_mem_power + total_crossbar_power + total_arbiter_power + total_link_power;

    std::string out_str = "\nTotal finished:       {}.\n";
                out_str += "Average delay:        {:.6g}.\n";
                out_str += "Total memory power:   {:.6g}.\n";
                out_str += "Total crossbar power: {:.6g}.\n";
                out_str += "Total arbiter power:  {:.6g}.\n";
                out_str += "Total link power:     {:.6g}.\n";
                out_str += "Total power:          {:.6g}.";

    Logger::info(
        fmt::runtime(out_str),
        Global::getTotalFin(),
        total_delay / Global::getTotalFin(),
        total_mem_power * POWER_NOM_,
        total_crossbar_power * POWER_NOM_,
        total_arbiter_power * POWER_NOM_,
        total_link_power * POWER_NOM_,
        total_power * POWER_NOM_
    );
}
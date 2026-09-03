#include "router/base_router.h"

#include <algorithm>

TimeType BaseRouter::getWireDelay(long port) {
    return this->curr_wireDelayFunc(port);
}

TimeType BaseRouter::getWireDelay_mesh(long) {
    return WIRE_DELAY_;
}

TimeType BaseRouter::getWireDelay_chipletMesh(long port) {
    return (port <= 5 ? (port < 3 ? LONG_WIRE_DELAY_X : LONG_WIRE_DELAY_Y) : SHORT_WIRE_DELAY);
}

TimeType BaseRouter::getWireDelay_chipletStar(long port) {
    TimeType r = 0;
    long centerXY = this->ary_size_ - 1;
    if (isCentral(this->address_, centerXY)) {
        long chipletX = (port - 1) / centerXY;
        bool inner = (chipletX == 1 || chipletX == 2);
        r = inner ? STAR_TOPO_4_2_INNER : STAR_TOPO_4_2_OUTER;
    } else if (((long)this->address_.size() << 1) + 1 == port) {
        r = (this->address_[0] == 1 || this->address_[0] == 2) ? STAR_TOPO_4_2_INNER
                                                               : STAR_TOPO_4_2_OUTER;
    } else {
        r = SHORT_WIRE_DELAY;
    }
    return r;
}

void BaseRouter::getNextAddress(AddrType& nextAddress, long port) {
    this->curr_nextAddFunc(nextAddress, port);
}

void BaseRouter::getNextAddress_chipletMesh(AddrType& nextAddress, long port) {
    this->getNextAddress_mesh(nextAddress, port);
}

void BaseRouter::getNextAddress_chipletStar(AddrType& nextAddress, long port) {
    long t = port - 1;
    long centerXY = this->ary_size_ - 1;
    if (isCentral(this->address_, centerXY)) {
        ldiv_t xy = ldiv(t, centerXY);
        nextAddress = {xy.quot, xy.rem, 0, 0};
    } else {
        const auto dimension = static_cast<std::size_t>(t >> 1);
        if (dimension == this->address_.size()) {
            nextAddress = {centerXY, centerXY, 0, 0};
        } else {
            nextAddress = this->address_;
            if (t & 1) {
                nextAddress[dimension]++;
                if (nextAddress[dimension] == this->ary_size_) {
                    nextAddress[dimension] = 0;
                }
            } else {
                nextAddress[dimension]--;
                if (nextAddress[dimension] == -1) {
                    nextAddress[dimension] = this->ary_size_ - 1;
                }
            }
        }
    }
}

void BaseRouter::getNextAddress_mesh(AddrType& nextAddress, long port) {
    long t = port - 1;
    nextAddress = this->address_;
    if (t & 1) {
        remainderAdd(nextAddress[t >> 1], this->ary_size_);
    } else {
        remainderReduce(nextAddress[t >> 1], this->ary_size_);
    }
}

long BaseRouter::getWirePc(long port) {
    return this->curr_wirePcFunc(port);
}

long BaseRouter::getWirePc_mesh(long port) {
    return port - 1 + ((port & 1) << 1);
}

long BaseRouter::getWirePc_chipletStar(long port) {
    long centerPort = ((long)this->address_.size() << 1) + 1;
    long centerXY = this->ary_size_ - 1;
    if (isCentral(this->address_, centerXY)) {
        return centerPort;
    }
    if (port == centerPort) {
        return this->address_[0] * centerXY + this->address_[1] + 1;
    } else {
        return port + (port & 1 ? 1 : -1);
    }
}

void BaseRouter::getFromRouter(AddrType& from, long port) {
    this->curr_prevRouterFunc(from, port);
}

void BaseRouter::getFromRouter_mesh(AddrType& from, long port) {
    this->getNextAddress_mesh(from, port);
}

void BaseRouter::getFromRouter_chipletStar(AddrType& from, long port) {
    long centerXY = this->ary_size_ - 1;
    long t = port - 1;
    if (isCentral(this->address_, centerXY)) {
        ldiv_t xy = ldiv(t, centerXY);
        from = {xy.quot, xy.rem, 0, 0};
    } else {
        const auto dimension = static_cast<std::size_t>(t >> 1);
        if (dimension == this->address_.size()) {
            from = {centerXY, centerXY, 0, 0};
        } else {
            from = this->address_;
            if (t & 1) {
                from[dimension]++;
                if (from[dimension] == this->ary_size_) {
                    from[dimension] = 0;
                }
            } else {
                from[dimension]--;
                if (from[dimension] == -1) {
                    from[dimension] = this->ary_size_ - 1;
                }
            }
        }
    }
}

long BaseRouter::getFromPort(long port) {
    return this->curr_prevPortFunc(port);
}

long BaseRouter::getFromPort_mesh(long port) {
    return this->getWirePc_mesh(port);
}

long BaseRouter::getFromPort_chipletStar(long port) {
    long centerXY = this->ary_size_ - 1;
    long centerPort = ((long)this->address_.size() << 1) + 1;
    if (isCentral(this->address_, centerXY)) {
        return centerPort;
    }
    if (centerPort == port) {
        return this->address_[0] * centerXY + this->address_[1] + 1;
    } else {
        return port + (port & 1 ? 1 : -1);
    }
}

void BaseRouter::routingAlg(const AddrType& destination, const AddrType& source, long s_ph,
                            long s_vc) {
    this->curr_algorithm(destination, source, s_ph, s_vc);
}

void BaseRouter::setRoutingType() {

    switch (this->routing_alg_) {

    case RoutingType::XY:
        this->curr_algorithm = std::bind(&BaseRouter::XY_algorithm, this, PARAM_4);
        this->curr_prevRouterFunc = std::bind(&BaseRouter::getFromRouter_mesh, this, PARAM_2);
        this->curr_prevPortFunc = std::bind(&BaseRouter::getFromPort_mesh, this, PARAM_1);
        this->curr_wireDelayFunc = std::bind(&BaseRouter::getWireDelay_mesh, this, PARAM_1);
        this->curr_nextAddFunc = std::bind(&BaseRouter::getNextAddress_mesh, this, PARAM_2);
        this->curr_wirePcFunc = std::bind(&BaseRouter::getWirePc_mesh, this, PARAM_1);
        break;

    case RoutingType::TXY:
        this->curr_algorithm = std::bind(&BaseRouter::TXY_algorithm, this, PARAM_4);
        this->curr_prevRouterFunc = std::bind(&BaseRouter::getFromRouter_mesh, this, PARAM_2);
        this->curr_prevPortFunc = std::bind(&BaseRouter::getFromPort_mesh, this, PARAM_1);
        this->curr_wireDelayFunc = std::bind(&BaseRouter::getWireDelay_mesh, this, PARAM_1);
        this->curr_nextAddFunc = std::bind(&BaseRouter::getNextAddress_mesh, this, PARAM_2);
        this->curr_wirePcFunc = std::bind(&BaseRouter::getWirePc_mesh, this, PARAM_1);
        break;

    case RoutingType::CHIPLET_ROUTING_MESH:
        this->curr_algorithm = std::bind(&BaseRouter::chiplet_routing_alg, this, PARAM_4);
        this->curr_prevRouterFunc = std::bind(&BaseRouter::getFromRouter_mesh, this, PARAM_2);
        this->curr_prevPortFunc = std::bind(&BaseRouter::getFromPort_mesh, this, PARAM_1);
        this->curr_wireDelayFunc = std::bind(&BaseRouter::getWireDelay_chipletMesh, this, PARAM_1);
        this->curr_nextAddFunc = std::bind(&BaseRouter::getNextAddress_chipletMesh, this, PARAM_2);
        this->curr_wirePcFunc = std::bind(&BaseRouter::getWirePc_mesh, this, PARAM_1);
        break;

    case RoutingType::CHIPLET_STAR_TOPO_ROUTING:
        this->curr_algorithm = std::bind(&BaseRouter::chiplet_star_topo_routing_alg, this, PARAM_4);
        this->curr_prevRouterFunc =
            std::bind(&BaseRouter::getFromRouter_chipletStar, this, PARAM_2);
        this->curr_prevPortFunc = std::bind(&BaseRouter::getFromPort_chipletStar, this, PARAM_1);
        this->curr_wireDelayFunc = std::bind(&BaseRouter::getWireDelay_chipletStar, this, PARAM_1);
        this->curr_nextAddFunc = std::bind(&BaseRouter::getNextAddress_chipletStar, this, PARAM_2);
        this->curr_wirePcFunc = std::bind(&BaseRouter::getWirePc_chipletStar, this, PARAM_1);
        break;

    case RoutingType::GRAPH_TOPO:
    case RoutingType::RECONFIGURABLE_GRAPH_TOPO:
        break;

    default:
        Sassert(false, "Error: Wrong type of routing algorithm");
        break;
    }
}

TimeType BaseRouter::getPipeStageDelay() const {
    return PIPE_DELAY_;
}

BaseRouter& BaseRouter::getRouter(const AddrType& addr) {
    std::size_t index = 0;
    for (const auto coordinate : addr) {
        index = index * static_cast<std::size_t>(this->ary_size_) +
                static_cast<std::size_t>(coordinate);
    }
    try {
        return *this->router_list_.at(index);
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
        throw;
    }
}

void BaseRouter::updateTrans(TimeType new_time, const Flit& flit) {
    ProtoStateMachine& trans_it = this->state_.protocols.get(flit.getPacketId());

    trans_it.packetDelay.push_back(flit.getSendFinTime() - flit.getStartTime());
    trans_it.packetDelay.push_back(new_time - flit.getStartTime());

    if (trans_it.protoDesc & (SPD_LAUNCH | SPD_BARRIER | SPD_LOCK | SPD_UNLOCK)) { // Launch
        this->updateTransACK(new_time, trans_it);
    } else {
        this->updateTransNormal(new_time, trans_it);
    }

    if (trans_it.status == ProtoState::DONE) {

        this->delay_writer_.writeProtocolCompletion(trans_it);
    }
}

void BaseRouter::updateTransACK(TimeType new_time, ProtoStateMachine& trans) {
    if (trans.status == ProtoState::DATA_TRANS) {
        // Add new packet.
        SPacket packet(trans.src_addr.size());
        packet.start_time = (new_time > trans.des_time) ? new_time : trans.des_time;
        packet.src_addr = trans.des_addr;
        if (trans.protoDesc & (SPD_BARRIER | SPD_LOCK | SPD_UNLOCK)) {
            std::fill(packet.src_addr.begin(), packet.src_addr.end(), 0);
        }
        packet.des_addr = trans.src_addr;
        packet.packet_size = 1;
        packet.id = trans.id;

        if (this->input_trace_.isEmpty(this->address_) ||
            this->input_trace_.front(this->address_).start_time > packet.start_time) {
            this->setLocalTime(packet.start_time);
        }
        this->input_trace_.addTrace(packet);
        this->state_.events.updateEVGCycle(packet.start_time);

        trans.status = ProtoState::ACK_TRANS;
    } else {
        trans.status = ProtoState::DONE;
    }
}

void BaseRouter::updateTransNormal(TimeType, ProtoStateMachine& trans) {
    trans.status = ProtoState::DONE;
}

TimeType BaseRouter::getLocalTime() const {
    return this->local_time_;
}

void BaseRouter::setLocalTime(TimeType new_local_time) {
    this->local_time_ = new_local_time;
}

void BaseRouter::acceptFlit(TimeType accept_time, const Flit& target_flit) {

    if (!isTail(target_flit.getFlitType())) {
        return;
    }

    if (this->config_.isSyncProtocolEnable() == false) {
        this->state_.markFinished();
        TimeType t = accept_time - target_flit.getStartTime();
        this->updateDelay(t);
        this->delay_writer_.writePacket(target_flit, t);
    } else {
        this->state_.markFinished();
        TimeType t = accept_time - target_flit.getStartTime();
        this->updateDelay(t);
        this->updateTrans(accept_time, target_flit);
    }
}

double BaseRouter::getBufferPower() {
    return this->power_module_.getBufferPower();
}

double BaseRouter::getLinkPower() {
    return this->power_module_.getLinkPower();
}

double BaseRouter::getCrossbarPower() {
    return this->power_module_.getCrossbarPower();
}

double BaseRouter::getArbiterPower() {
    return this->power_module_.getArbiterPower();
}

void BaseRouter::updateDelay(TimeType new_delay) {
    this->total_delay_ += new_delay;
}

TimeType BaseRouter::getTotalDelay() const {
    return this->total_delay_;
}

void BaseRouter::recvCredit(long phy_idx, long vc_idx) {
    this->output_module_.incCounter(phy_idx, vc_idx);
}

void BaseRouter::recvPacket() {
    if (this->input_trace_.isEmpty(this->address_)) {
        this->local_time_ = LOCAL_INPUT_TIME_0;
        return;
    }

    TimeType event_time = this->state_.currentTime();
    this->local_time_ = this->input_trace_.front(this->address_).start_time;

    while (this->input_module_.isIBuffFull() == false &&
           this->local_time_ <= (event_time + S_ELPS_)) {
        const SPacket& p = this->input_trace_.front(this->address_);

        this->injctPacket(packet_counter_, p.src_addr, p.des_addr, p.start_time, p.packet_size,
                          p.id);

        packet_counter_++;

        this->input_trace_.popFront(this->address_);

        if (this->input_trace_.isEmpty(this->address_)) {
            this->local_time_ = LOCAL_INPUT_TIME_0;
            break;
        }
        this->local_time_ = this->input_trace_.front(this->address_).start_time;
    }

    if (FILTERING_BOOL && this->input_module_.isIBuffFull()) {
        std::ostringstream os;
        os << this->address_;
        Logger::info("Jam at time {} in router {}.", this->state_.currentTime(), os.str());
    }
}

void BaseRouter::injctPacket(long flit_id, const AddrType& src_addr, const AddrType& des_addr,
                             TimeType start_time, long packet_size, TId packet_id) {
    VCType vc_t;

    for (auto& iter : src_addr) {
        if (iter >= this->ary_size_ || iter < 0) {
            Sassert(false, "Coordinate out of range.");
        }
    }
    for (auto& iter : des_addr) {
        if (iter >= this->ary_size_ || iter < 0) {
            Sassert(false, "Coordinate out of range.");
        }
    }

    for (long idx = 0; idx < packet_size; idx++) {
        DataType flit_data(static_cast<std::size_t>(this->flit_size_));
        for (long flit_idx = 0; flit_idx < this->flit_size_; flit_idx++) {
            this->init_data_[flit_idx] =
                static_cast<AtomType>(this->init_data_[flit_idx] * CORR_EFF_ +
                                      this->state_.random.random_u_long_long(0, MAX_64_));
            flit_data[static_cast<std::size_t>(flit_idx)] = this->init_data_[flit_idx];
        }

        FlitType flit_type = FlitType::BODY;
        if (idx == 0) {
            vc_t = std::pair<long, long>(0, this->input_module_.getBufferSize(0, 0));
            for (long i = 0; i < this->vc_number_; i++) {
                long t = this->input_module_.getBufferSize(0, i);
                if (vc_t.second > t) {
                    vc_t = std::pair<long, long>(i, t);
                }
            }

            // if the input buffer is empty, set it to be ROUTING_
            if (this->input_module_.getBufferSize(0, vc_t.first) == 0) {
                this->input_module_.setState(0, vc_t.first, VCStateType::ROUTING);
            }

            // if the input buffer has more than predefined flits, then
            // add the flits and sign a flag
            if (this->input_module_.getBufferSize(0, vc_t.first) > 100) {
                this->input_module_.setIBuffFull(true);
            }

            flit_type = packet_size == 1 ? FlitType::SINGLE : FlitType::HEADER;
        } else if (idx == (packet_size - 1)) {
            flit_type = FlitType::TAIL;
        }

        this->power_module_.addBufferWritePwr(0, flit_data);
        Flit flit(flit_id, flit_type, src_addr, des_addr, start_time, std::move(flit_data),
                  packet_id);

        flit.setSendFinTime(start_time + idx);

        if (this->config_.isSyncProtocolEnable()) {
            ProtoStateMachine& trans_it = this->state_.protocols.get(flit.getPacketId());

            // trans_it.packetDelay.push_back(flit.getSendFinTime() - flit.getStartTime());

            if (flit_type == FlitType::SINGLE || flit_type == FlitType::TAIL) {
                this->delay_writer_.writeProtocolInjection(trans_it, flit.getSendFinTime() -
                                                                         flit.getStartTime());
            }
        }

        this->input_module_.addFlit(0, vc_t.first, std::move(flit));
    }
}

void BaseRouter::recvFlit(long phy_idx, long vc_idx, Flit&& flit) {
    if (this->config_.isPacketLoss()) {
        const auto packet_id = flit.getPacketId();
        const bool buffer_full = this->input_module_.getBufferSize(phy_idx, vc_idx) >=
                                 static_cast<std::size_t>(this->inbuffer_size_);
        if (buffer_full) {
            this->state_.markAbandoned(packet_id);
        }
        if (this->state_.isAbandoned(packet_id)) {
            this->state_.events.addMessage(MessEvent(
                this->state_.currentTime() + CREDIT_DELAY_, MessType::CREDIT, this->address_,
                flit.getCreditReturnAddress(), flit.getCreditReturnPort(), vc_idx));
            return;
        }
    }
    this->power_module_.addBufferWritePwr(phy_idx, flit.getData());
    const auto flit_type = flit.getFlitType();
    this->input_module_.addFlit(phy_idx, vc_idx, std::move(flit));

    if (isHeader(flit_type)) {
        if (this->input_module_.getBufferSize(phy_idx, vc_idx) == 1) {
            this->input_module_.setState(phy_idx, vc_idx, VCStateType::ROUTING);
        } else {
            // Do nothing
        }
    } else {
        if (this->input_module_.getState(phy_idx, vc_idx) == VCStateType::INIT) {
            this->input_module_.setState(phy_idx, vc_idx, VCStateType::SW_AB);
        }
    }
}

void BaseRouter::decideRouting() {
    TimeType event_time = this->state_.currentTime();

    for (long each_vc = 0; each_vc < this->vc_number_; each_vc++) {
        if (this->input_module_.getState(0, each_vc) == VCStateType::ROUTING) {
            Flit flit(this->input_module_.getFlit(0, each_vc));
            AddrType des_t = flit.getDesAddr();
            AddrType sor_t = flit.getSrcAddr();
            if (this->address_ == des_t) {
                this->acceptFlit(event_time, flit);
                this->input_module_.removeFlit(0, each_vc);

                VCStateType vcst;
                if (flit.getFlitType() == FlitType::HEADER) {
                    vcst = VCStateType::HOME;
                } else {
                    vcst = (this->input_module_.getBufferSize(0, each_vc) > 0 ? VCStateType::ROUTING
                                                                              : VCStateType::INIT);
                }
                this->input_module_.setState(0, each_vc, vcst);
            } else {
                this->input_module_.clearRouting(0, each_vc);
                this->routingAlg(des_t, sor_t, 0, each_vc);
                this->input_module_.setState(0, each_vc, VCStateType::VC_AB);
            }
        } else if (this->input_module_.getState(0, each_vc) == VCStateType::HOME) {
            if (this->input_module_.getBufferSize(0, each_vc) > 0) {
                Flit flit(this->input_module_.getFlit(0, each_vc));
                Sassert(!isHeader(flit.getFlitType()), "Error: Flit type is not HEADER");
                this->acceptFlit(event_time, flit);
                this->input_module_.removeFlit(0, each_vc);
                if (flit.getFlitType() == FlitType::TAIL) {
                    if (this->input_module_.getBufferSize(0, each_vc) > 0) {
                        this->input_module_.setState(0, each_vc, VCStateType::ROUTING);
                    } else {
                        this->input_module_.setState(0, each_vc, VCStateType::INIT);
                    }
                }
            }
        }
    }

    for (long each_phy = 1; each_phy < this->physic_ports_; each_phy++) {
        for (long each_vc = 0; each_vc < this->vc_number_; each_vc++) {
            if (this->input_module_.getBufferSize(each_phy, each_vc) > 0) {
                Flit flit(this->input_module_.getFlit(each_phy, each_vc));
                AddrType des_t = flit.getDesAddr();
                if (this->address_ == des_t) {
                    this->state_.events.addMessage(MessEvent(
                        event_time + CREDIT_DELAY_, MessType::CREDIT, this->address_,
                        flit.getCreditReturnAddress(), flit.getCreditReturnPort(), each_vc));
                }
            }
            if (this->input_module_.getState(each_phy, each_vc) == VCStateType::ROUTING) {
                Flit flit(this->input_module_.getFlit(each_phy, each_vc));
                Sassert(isHeader(flit.getFlitType()), "Error: Flit type is not HEADER");
                AddrType des_t = flit.getDesAddr();
                AddrType sor_t = flit.getSrcAddr();
                if (this->address_ == des_t) {
                    this->acceptFlit(event_time, flit);
                    this->input_module_.removeFlit(each_phy, each_vc);

                    VCStateType vcst;
                    if (flit.getFlitType() == FlitType::HEADER) {
                        vcst = VCStateType::HOME;
                    } else {
                        vcst = (this->input_module_.getBufferSize(each_phy, each_vc) > 0
                                    ? VCStateType::ROUTING
                                    : VCStateType::INIT);
                    }
                    this->input_module_.setState(each_phy, each_vc, vcst);
                } else {
                    this->input_module_.clearRouting(each_phy, each_vc);
                    this->routingAlg(des_t, sor_t, each_phy, each_vc);
                    this->input_module_.setState(each_phy, each_vc, VCStateType::VC_AB);
                }
            } else if (this->input_module_.getState(each_phy, each_vc) == VCStateType::HOME) {
                if (this->input_module_.getBufferSize(each_phy, each_vc) > 0) {
                    Flit flit(this->input_module_.getFlit(each_phy, each_vc));
                    Sassert(!isHeader(flit.getFlitType()), "Error: Flit type is not HEADER");
                    this->acceptFlit(event_time, flit);
                    this->input_module_.removeFlit(each_phy, each_vc);
                    if (flit.getFlitType() == FlitType::TAIL) {
                        if (this->input_module_.getBufferSize(each_phy, each_vc) > 0) {
                            this->input_module_.setState(each_phy, each_vc, VCStateType::ROUTING);
                        } else {
                            this->input_module_.setState(each_phy, each_vc, VCStateType::INIT);
                        }
                    }
                }
            }
        }
    }
}

void BaseRouter::routingPipeStage(TimeType routing_period) {
    if (routing_period != this->getPipeStageDelay()) {
        return;
    }
    // stage 5 flit traversal
    this->traverseLink();
    // stage 4 flit output buffer
    this->toOutBuffer();
    // stage 3 switch arbitration
    this->arbitrationSW();
    // stage 2, vc arbitration
    this->arbitrationVC();
    // stage 1, routing decision
    this->decideRouting();
}

VCType BaseRouter::selectVC(long phy_idx, long vc_idx) {
    const auto& candidates = this->input_module_.getRouting(phy_idx, vc_idx);
    Sassert(!candidates.empty(), "Error: No available VC");
    available_vcs_.clear();

    for (const auto& vc : candidates) {
        if (this->config_.getVcShare() == VCShareType::SHARE) {
            if (this->output_module_.getUsage(vc.first, vc.second) == VCUsageType::FREE) {
                available_vcs_.push_back(vc);
            }
        } else {
            if (this->output_module_.getCounter(vc.first, vc.second) == this->inbuffer_size_) {
                if (this->output_module_.getUsage(vc.first, vc.second) == VCUsageType::FREE) {
                    available_vcs_.push_back(vc);
                }
            }
        }
    }

    if (available_vcs_.empty()) {
        return VCType(-1, -1);
    }
    if (available_vcs_.size() == 1) {
        return available_vcs_.front();
    }
    return available_vcs_[static_cast<std::size_t>(
        this->state_.random.random_long(0, static_cast<long>(available_vcs_.size())))];
}

void BaseRouter::arbitrationVC() {
    for (auto& requests : vc_requests_) {
        requests.clear();
    }
    AtomType vc_request = 0;
    bool has_requests = false;

    for (long i = 0; i < this->physic_ports_; i++) {
        for (long j = 0; j < this->vc_number_; j++) {
            VCType vc_t;
            if (this->input_module_.getState(i, j) == VCStateType::VC_AB) {
                vc_t = this->selectVC(i, j);
                if ((vc_t.first >= 0) && (vc_t.second >= 0)) {
                    vc_requests_[static_cast<std::size_t>(vc_t.first * vc_number_ + vc_t.second)]
                        .push_back(VCType(i, j));
                    vc_request = vc_request | this->state_.vcMasks[i * vc_number_ + j];
                    has_requests = true;
                }
            }
        }
    }
    if (!has_requests) {
        return;
    }
    for (long i = 1; i < this->physic_ports_; i++) {
        for (long j = 0; j < this->vc_number_; j++) {
            if (this->output_module_.getUsage(i, j) == VCUsageType::FREE) {
                auto& requests = vc_requests_[static_cast<std::size_t>(i * vc_number_ + j)];
                const auto cont_temp = static_cast<long>(requests.size());
                if (cont_temp > 0) {
                    VCType vc_win = requests.front();
                    if (cont_temp > 1) {
                        vc_win = requests[static_cast<std::size_t>(
                            this->state_.random.random_long(0, cont_temp))];
                    }
                    this->input_module_.setState(vc_win.first, vc_win.second, VCStateType::SW_AB);
                    this->input_module_.setCRouting(vc_win.first, vc_win.second, VCType(i, j));
                    this->output_module_.acquireChannel(i, j, vc_win);
                    this->power_module_.addVCArbitPwr(
                        i, j, vc_request, (vc_win.first) * vc_number_ + (vc_win.second));
                }
            }
        }
    }
}

void BaseRouter::arbitrationSW() {
    for (auto& requests : switch_requests_) {
        requests.clear();
    }
    bool has_requests = false;
    for (long i = 0; i < physic_ports_; i++) {
        input_vc_candidates_.clear();
        for (long j = 0; j < vc_number_; j++) {
            if (this->input_module_.getState(i, j) == VCStateType::SW_AB) {
                VCType out_t = this->input_module_.getCRouting(i, j);
                if ((this->output_module_.getCounter(out_t.first, out_t.second) > 0) &&
                    (this->output_module_.getLocalCounter(out_t.first) > 0)) {
                    input_vc_candidates_.push_back(j);
                }
            }
        }
        const auto vc_size_t = static_cast<long>(input_vc_candidates_.size());
        if (vc_size_t > 1) {
            long win_t = this->state_.random.random_long(0, vc_size_t);
            const auto input_vc = input_vc_candidates_[static_cast<std::size_t>(win_t)];
            VCType r_t = this->input_module_.getCRouting(i, input_vc);
            switch_requests_[static_cast<std::size_t>(r_t.first)].push_back(VCType(i, input_vc));
            has_requests = true;
        } else if (vc_size_t == 1) {
            VCType r_t = this->input_module_.getCRouting(i, input_vc_candidates_.front());
            switch_requests_[static_cast<std::size_t>(r_t.first)].push_back(
                VCType(i, input_vc_candidates_.front()));
            has_requests = true;
        }
    }

    if (!has_requests) {
        return;
    }

    for (long i = 0; i < this->physic_ports_; i++) {
        auto& requests = switch_requests_[static_cast<std::size_t>(i)];
        const auto vc_size_t = static_cast<long>(requests.size());
        if (vc_size_t > 0) {
            VCType vc_win = requests.front();
            if (vc_size_t > 1) {
                vc_win = requests[static_cast<std::size_t>(
                    this->state_.random.random_long(0, vc_size_t))];
            }
            this->input_module_.setState(vc_win.first, vc_win.second, VCStateType::SW_TR);
        }
    }
}

void BaseRouter::toOutBuffer() {
    for (long i = 0; i < this->physic_ports_; i++) {
        for (long j = 0; j < this->vc_number_; j++) {
            if (this->input_module_.getState(i, j) == VCStateType::SW_TR) {
                VCType out_t = this->input_module_.getCRouting(i, j);
                this->output_module_.decCounter(out_t.first, out_t.second);

                TimeType event_time = this->state_.currentTime();
                if (i != 0) {
                    const auto& incoming_flit = this->input_module_.getFlit(i, j);
                    this->state_.events.addMessage(
                        MessEvent(event_time + CREDIT_DELAY_, MessType::CREDIT, this->address_,
                                  incoming_flit.getCreditReturnAddress(),
                                  incoming_flit.getCreditReturnPort(), j));
                }

                long in_size_t = this->input_module_.getBufferSize(i, j);
                Sassert(in_size_t >= 1, "Error: Buffer size is less than 1");
                Flit flit_t(std::move(input_module_.getFlit(i, j)));
                this->input_module_.removeFlit(i, j);
                this->power_module_.addBufferReadPwr(i, flit_t.getData());
                this->power_module_.addCrossbarTravPwr(i, out_t.first, flit_t.getData());
                const auto flit_type = flit_t.getFlitType();
                this->output_module_.addFlit(out_t.first, std::move(flit_t));
                if (i == 0) {
                    if (this->input_module_.isIBuffFull() == true) {
                        if (this->input_module_.getBufferSize(0, j) < BUFF_BOUND_) {
                            this->input_module_.setIBuffFull(false);
                            this->recvPacket();
                        }
                    }
                }
                this->output_module_.addAddr(out_t.first, out_t);
                if (isTail(flit_type)) {
                    this->output_module_.releaseChannel(out_t.first, out_t.second);
                }
                if (in_size_t > 1) {
                    if (isTail(flit_type)) {
                        if (this->config_.getVcShare() == VCShareType::MONO) {
                            if (i != 0) {
                                if (in_size_t != 1) {
                                    Logger::info("{}: {}", i, in_size_t);
                                }
                                Sassert(in_size_t == 1, "Error: Buffer size is not 1");
                            }
                        }
                        this->input_module_.setState(i, j, VCStateType::ROUTING);
                    } else {
                        this->input_module_.setState(i, j, VCStateType::SW_AB);
                    }
                } else {
                    this->input_module_.setState(i, j, VCStateType::INIT);
                }
            }
        }
    }
}

void BaseRouter::traverseLink() {
    for (long i = 1; i < this->physic_ports_; i++) {
        this->traverseLink(i);
    }
}

void BaseRouter::traverseLink(long port) {
    TimeType event_time = this->state_.currentTime();
    if (this->output_module_.getOutBufferSize(port) > 0) {
        TimeType delay = this->getWireDelay(port);
        TimeType flit_delay_t = delay + event_time;
        AddrType wire_add_t;
        long wire_pc_t = getWirePc(port);
        this->getNextAddress(wire_add_t, port);
        Flit flit_t(std::move(this->output_module_.getFlit(port)));
        VCType outadd_t = this->output_module_.getAddr(port);
        this->power_module_.addLinkTravPwr(port, flit_t.getData());
        flit_t.setCreditReturn(this->address_, port);

        this->output_module_.removeFlit(port);
        this->output_module_.removeAddr(port);
        this->state_.events.addMessage(MessEvent(flit_delay_t, MessType::WIRE, this->address_,
                                                 wire_add_t, wire_pc_t, outadd_t.second,
                                                 std::move(flit_t)));
    }
}

void BaseRouter::XY_algorithm(const AddrType& des_t, const AddrType&, long s_ph, long s_vc) {
    if (this->vc_number_ < 4) {
        Logger::warn("VC number is less than 4, XY algorithm may not work properly");
    }

    for (std::size_t dimension = des_t.size(); dimension-- > 0;) {
        const long offset = des_t[dimension] - this->address_[dimension];
        if (offset == 0) {
            continue;
        }

        const auto port = static_cast<long>(dimension * 2) + (offset < 0 ? 1 : 2);
        for (long vc = 0; vc < 4 && vc < this->vc_number_; ++vc) {
            this->input_module_.addRouting(s_ph, s_vc, VCType(port, vc));
        }
        return;
    }
}

void BaseRouter::TXY_algorithm(const AddrType& des_t, const AddrType&, long s_ph, long s_vc) {
    Sassert(this->vc_number_ >= 2,
            "Error: VC number is less than 2, TXY algorithm should not be used");

    long xoffset = des_t[0] - address_[0];
    long yoffset = des_t[1] - address_[1];
    bool xdirection = (abs(static_cast<int>(xoffset)) * 2 <= this->ary_size_) ? true : false;
    bool ydirection = (abs(static_cast<int>(yoffset)) * 2 <= this->ary_size_) ? true : false;

    if (xdirection) {
        if (xoffset < 0) {
            this->input_module_.addRouting(s_ph, s_vc, VCType(1, 0));
        } else if (xoffset > 0) {
            this->input_module_.addRouting(s_ph, s_vc, VCType(2, 1));
        } else {
            if (ydirection) {
                if (yoffset < 0) {
                    this->input_module_.addRouting(s_ph, s_vc, VCType(3, 0));
                } else if (yoffset > 0) {
                    this->input_module_.addRouting(s_ph, s_vc, VCType(4, 1));
                }
            } else {
                if (yoffset < 0) {
                    this->input_module_.addRouting(s_ph, s_vc, VCType(4, 0));
                } else if (yoffset > 0) {
                    this->input_module_.addRouting(s_ph, s_vc, VCType(3, 1));
                }
            }
        }
    } else {
        if (xoffset < 0) {
            this->input_module_.addRouting(s_ph, s_vc, VCType(2, 0));
        } else if (xoffset > 0) {
            this->input_module_.addRouting(s_ph, s_vc, VCType(1, 1));
        } else {
            if (ydirection) {
                if (yoffset < 0) {
                    this->input_module_.addRouting(s_ph, s_vc, VCType(3, 0));
                } else if (yoffset > 0) {
                    this->input_module_.addRouting(s_ph, s_vc, VCType(4, 1));
                }
            } else {
                if (yoffset < 0) {
                    this->input_module_.addRouting(s_ph, s_vc, VCType(4, 0));
                } else if (yoffset > 0) {
                    this->input_module_.addRouting(s_ph, s_vc, VCType(3, 1));
                }
            }
        }
    }
}

void BaseRouter::chiplet_routing_alg(const AddrType& des_t, const AddrType&, long s_ph, long s_vc) {
    const long VIRTUAL_CHANNEL_COUNT = 2;
    AddrType delta;
    size_t addr_len = des_t.size();

    delta.reserve(addr_len);

    for (std::size_t i = 0; i < addr_len; ++i) {
        delta.push_back(des_t[i] - this->address_[i]);
    }

    if (delta[0] == 0 && delta[1] == 0) {
        VCType vc;
        if (delta[2] == 0) {
            vc.first = (delta[3] < 0 ? 7 : 8);
        } else {
            vc.first = (delta[2] < 0 ? 5 : 6);
        }
        for (long i = 0; i < VIRTUAL_CHANNEL_COUNT; ++i) {
            vc.second = i;
            this->input_module_.addRouting(s_ph, s_vc, vc);
        }
    } else {
        long vc_first;
        if (this->address_[2] == 0 && this->address_[3] == 0) {
            if (delta[0] == 0) {
                vc_first = (delta[1] < 0 ? 3 : 4);
            } else {
                vc_first = (delta[0] < 0 ? 1 : 2);
            }
            for (long i = 0; i < VIRTUAL_CHANNEL_COUNT; ++i) {
                this->input_module_.addRouting(s_ph, s_vc, VCType(vc_first, i));
            }
        } else {
            if (this->address_[2] > 0) {
                vc_first = 5;
            } else {
                vc_first = 7;
            }
            for (long i = 0; i < VIRTUAL_CHANNEL_COUNT; ++i) {
                this->input_module_.addRouting(s_ph, s_vc, VCType(vc_first, i));
            }
        }
    }
}

void BaseRouter::chiplet_star_topo_routing_alg(const AddrType& des_t, const AddrType&, long s_ph,
                                               long s_vc) {
    const auto address_length = this->address_.size();
    AddrType delta;
    delta.reserve(address_length);
    for (std::size_t index = 0; index < address_length; ++index) {
        delta.push_back(des_t[index] - this->address_[index]);
    }

    long centerXY = this->ary_size_ - 1;
    if (isCentral(this->address_, centerXY)) {
        for (long vc = 0; vc < 2; ++vc) {
            this->input_module_.addRouting(s_ph, s_vc,
                                           VCType(des_t[0] * centerXY + des_t[1] + 1, vc));
        }
    } else if (delta[0] == 0 && delta[1] == 0) {
        long port = 0;
        if (delta[2] == 0) {
            if (delta[3] == 0) {
                return;
            }
            port = (3 << 1) + (delta[3] > 0 ? 2 : 1);
        } else {
            port = (2 << 1) + (delta[2] > 0 ? 2 : 1);
        }
        for (long vc = 0; vc < 2; ++vc) {
            this->input_module_.addRouting(s_ph, s_vc, VCType(port, vc));
        }
    } else {
        if (this->address_[2] == 0 && this->address_[3] == 0) {
            const auto center_port = static_cast<long>(address_length * 2) + 1;
            for (long vc = 0; vc < 2; ++vc) {
                this->input_module_.addRouting(s_ph, s_vc, VCType(center_port, vc));
            }
        } else {
            const long port = (this->address_[2] > 0 ? 5 : 7);
            for (long vc = 0; vc < 2; ++vc) {
                this->input_module_.addRouting(s_ph, s_vc, VCType(port, vc));
            }
        }
    }
}

BaseRouter::BaseRouter(const Config& config, AddrType addr, RouterList& router_list,
                       SimulationState& state, InputTrace& input_trace, DelayWriter& delay_writer,
                       long physical_port_count)
    : router_list_(router_list), address_(std::move(addr)),
      input_module_(physical_port_count > 0 ? physical_port_count : config.getPhysicalPortNumber(),
                    config.getVirtualChannelNumber()),
      output_module_(physical_port_count > 0 ? physical_port_count : config.getPhysicalPortNumber(),
                     config.getVirtualChannelNumber(), config.getInBufferSize(),
                     config.getOutBufferSize()),
      power_module_(physical_port_count > 0 ? physical_port_count : config.getPhysicalPortNumber(),
                    config.getVirtualChannelNumber(), config.getFlitSize(), config.getLinkLength()),
      init_data_(), config_(config), state_(state), input_trace_(input_trace),
      delay_writer_(delay_writer), ary_size_(config.getAryNumber()),
      flit_size_(config.getFlitSize()),
      physic_ports_(physical_port_count > 0 ? physical_port_count : config.getPhysicalPortNumber()),
      vc_number_(config.getVirtualChannelNumber()), inbuffer_size_(config.getInBufferSize()),
      outbuffer_size_(config.getOutBufferSize()), total_delay_(0),
      routing_alg_(config.getRoutingAlg()), curr_algorithm(), local_time_(LOCAL_INPUT_TIME_0),
      packet_counter_(0), available_vcs_(),
      vc_requests_(static_cast<std::size_t>(physic_ports_ * vc_number_)),
      switch_requests_(static_cast<std::size_t>(physic_ports_)), input_vc_candidates_() {
    this->state_.ensureVcMasks(static_cast<std::size_t>(this->physic_ports_ * this->vc_number_));
    this->init_data_.resize(this->flit_size_);
    for (long i = 0; i < this->flit_size_; i++) {
        this->init_data_[i] = this->state_.random.random_u_long_long(0, MAX_64_);
    }
    available_vcs_.reserve(static_cast<std::size_t>(vc_number_));
    input_vc_candidates_.reserve(static_cast<std::size_t>(vc_number_));
    for (auto& requests : vc_requests_) {
        requests.reserve(static_cast<std::size_t>(physic_ports_ * vc_number_));
    }
    for (auto& requests : switch_requests_) {
        requests.reserve(static_cast<std::size_t>(physic_ports_));
    }
    this->setRoutingType();
}

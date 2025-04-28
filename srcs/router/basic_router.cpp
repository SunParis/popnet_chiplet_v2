# include "router/base_router.h"


TimeType BaseRouter::getWireDelay(long port) {
    return this->curr_wireDelayFunc(port);
}
	
TimeType BaseRouter::getWireDelay_mesh(long port) {
    return WIRE_DELAY_;
}
	
TimeType BaseRouter::getWireDelay_chipletMesh(long port) {
    return (port <= 5 ? (port < 3 ? LONG_WIRE_DELAY_X : LONG_WIRE_DELAY_Y) : SHORT_WIRE_DELAY);
}
	
TimeType BaseRouter::getWireDelay_chipletStar(long port) {
    TimeType r = 0;
	long centerXY = this->ary_size_-1;
	if (isCentral(this->address_, centerXY)) {
		long chipletX = (port - 1) / centerXY;
		bool inner = (chipletX == 1 || chipletX==2);
		r = inner ? STAR_TOPO_4_2_INNER : STAR_TOPO_4_2_OUTER;
	}
	else if (((long)this->address_.size() << 1) + 1 == port) {
		r = (this->address_[0] == 1 || this->address_[0]==2) ? STAR_TOPO_4_2_INNER : STAR_TOPO_4_2_OUTER;
	}
	else {
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
	}
	else {
		long p = t >> 1;
		if (p == this->address_.size()) {
            nextAddress = {centerXY, centerXY, 0, 0};
        }
		else {
			nextAddress = this->address_;
			if (t & 1) {
				nextAddress[p]++;
				if (nextAddress[p] == this->ary_size_) {
                    nextAddress[p] = 0;
                }					
			}
			else {
				nextAddress[p]--;
				if (nextAddress[p] == -1) {
                    nextAddress[p] = this->ary_size_ - 1;
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
	}
    else {
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
	long centerXY = this->ary_size_-1;
	if(isCentral(this->address_, centerXY)){
		return centerPort;
	}
	if (port == centerPort) {
        return this->address_[0] * centerXY + this->address_[1] + 1;
    }
	else {
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
    long centerXY = this->ary_size_-1;
	long t = port-1;
	if (isCentral(this->address_, centerXY)) {
		ldiv_t xy = ldiv(t, centerXY);
		from = {xy.quot, xy.rem, 0, 0};
	}
    else {
		long p = t >> 1;
		if (p == this->address_.size()) {
            from = {centerXY, centerXY, 0, 0};
        }
		else {
			from = this->address_;
			if (t & 1) {
				from[p]++;
				if(from[p] == this->ary_size_) {
                    from[p] = 0;
                }
			}
            else {
				from[p]--;
				if (from[p] == -1) {
                    from[p] = this->ary_size_-1;
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
    long centerXY = this->ary_size_-1;
	long centerPort = ((long)this->address_.size() << 1) + 1;
	if (isCentral(this->address_, centerXY)) {
		return centerPort;
	}
	if (centerPort == port) {
        return this->address_[0] * centerXY + this->address_[1] + 1;
    }
    else {
        return port + (port & 1 ? 1 : -1);
    }
}

void BaseRouter::routingAlg(const AddrType& destination, const AddrType& source, long s_ph, long s_vc) {
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
    		this->curr_prevRouterFunc= std::bind(&BaseRouter::getFromRouter_chipletStar, this, PARAM_2);
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
    long idx = 0;
    for (long i = 0; i < addr.size(); i++) {
        idx = idx * this->ary_size_ + addr[i];
    }
    BaseRouter *ret = nullptr;
    try {
        ret = this->router_list_.at(idx);
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
    return *ret;
}

void BaseRouter::updateTrans(TimeType new_time, const Flit& flit) {
    ProtoStateMachine& trans_it = Global::getTrans(flit.getPacketId());

    trans_it.packetDelay.push_back(flit.getSendFinTime() - flit.getStartTime());
    trans_it.packetDelay.push_back(new_time - flit.getStartTime());

    if (trans_it.protoDesc & (SPD_LAUNCH | SPD_BARRIER | SPD_LOCK | SPD_UNLOCK)) { // Launch
        this->updateTransACK(new_time, trans_it);
    }
    else {
        this->updateTransNormal(new_time, trans_it);
    }

    if (trans_it.status == ProtoState::DONE) {
            
        std::ofstream ofs(this->config_.getDelayFname(), std::ios::app);
            
        ofs << (long)trans_it.src_time << ' ';
            
        for (auto& x : trans_it.src_addr)
            ofs << x << ' ';
            
        for (auto& x : trans_it.des_addr)
            ofs << x << ' ';
            
        ofs << trans_it.protoDesc << ' ' << trans_it.packetDelay.size() << ' ';
            
        for (auto& x : trans_it.packetDelay)
            ofs << (long)x << ' ';
            
        ofs << std::endl;

        ofs.close();
    }
}

void BaseRouter::updateTransACK(TimeType new_time, ProtoStateMachine& trans) {
    if (trans.status == ProtoState::DATA_TRANS) {
        // Add new packet.
        SPacket packet(trans.src_addr.size());
        packet.start_time = (new_time > trans.des_time) ? new_time : trans.des_time;
        packet.src_addr = trans.des_addr;
        if (trans.protoDesc & (SPD_BARRIER | SPD_LOCK | SPD_UNLOCK)) {
            for (int i = 0; i < packet.src_addr.size(); i++) {
                packet.src_addr[i] = 0;
            }
        }
        packet.des_addr = trans.src_addr;
        packet.packet_size = 1;
        packet.id = trans.id;
        
        if (Global::inputTrace->isEmpty(this->address_)
            || Global::inputTrace->front().start_time > packet.start_time
        ) {
		    this->setLocalTime(packet.start_time);
	    }
        Global::inputTrace->addTrace(packet);
        Global::messageQueue.updateEVGCycle(packet.start_time);
        
        trans.status = ProtoState::ACK_TRANS;
    }
    else {
        trans.status = ProtoState::DONE;
    }
}

void BaseRouter::updateTransNormal(TimeType new_time, ProtoStateMachine& trans) {
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
		std::ofstream ofs(this->config_.getDelayFname(), std::ios::app);
		Global::incTotalFin();
		TimeType t = accept_time - target_flit.getStartTime();
		this->updateDelay(t);
		ofs << (long)target_flit.getStartTime() << ' ';
		for (auto &x : target_flit.getSrcAddr()) {
            ofs << x << ' ';
        }
		for (auto &x : target_flit.getDesAddr()) {
            ofs << x << ' ';
        }
		ofs << t << std::endl;
        ofs.close();
	}
	else {
		Global::incTotalFin();
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
    
    if (Global::inputTrace->isEmpty(this->address_)) 
        return;
    
    
    if (this->local_time_ == LOCAL_INPUT_TIME_0_0) {
        this->local_time_ = Global::inputTrace->front().start_time;
    }

    TimeType event_time = Global::getCurrTime();

    while (this->input_module_.isIBuffFull() == false
        && this->local_time_ <= (event_time + S_ELPS_))
	{
		if (Global::inputTrace->isEmpty(this->address_)) 
            return;
		
        const SPacket& p = Global::inputTrace->front(this->address_);
		
        this->injctPacket(packet_counter_, p.src_addr, p.des_addr, this->local_time_, p.packet_size, p.id);
		
        packet_counter_++;
		
        Global::inputTrace->popFront(this->address_);
		
        if (Global::inputTrace->isEmpty(this->address_)) {
			this->local_time_ = LOCAL_INPUT_TIME_0;
		}
        else {
			this->local_time_ = Global::inputTrace->front().start_time;
		}
	}
	
	if (FILTERING_BOOL && this->input_module_.isIBuffFull()) {
        std::ostringstream os;
        os << this->address_;
        Logger::info("Jam at time {} in router {}.", Global::getCurrTime(), os.str());
	}
}

void BaseRouter::injctPacket(long flit_id, const AddrType& src_addr,
    const AddrType& des_addr, TimeType start_time,
    long packet_size, TId packet_id)
{
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
        DataType flit_data;
        for (long flit_idx = 0; flit_idx < this->flit_size_; flit_idx++) {
            this->init_data_[flit_idx] = static_cast<AtomType>(
                this->init_data_[flit_idx] * CORR_EFF_ +
                Global::RandomGen.random_u_long_long(0, MAX_64_)
            );
            // Logger::debug("Flit data: {}", this->init_data_[flit_idx]);
			flit_data.push_back(this->init_data_[flit_idx]);
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
        }
        else if (idx == (packet_size - 1)) {
            flit_type = FlitType::TAIL;
		}

        Flit flit(flit_id, flit_type,
            src_addr, des_addr, start_time, flit_data, packet_id);
		
        flit.setSendFinTime(start_time + idx);
		
        this->input_module_.addFlit(0, vc_t.first, flit);
		this->power_module_.addBufferWritePwr(0, flit_data);
    }
}

void BaseRouter::recvFlit(long phy_idx, long vc_idx, Flit& flit) {
    if (this->config_.isPacketLoss()) {
        if (this->input_module_.getBufferSize(phy_idx, vc_idx) > this->inbuffer_size_) {
            Global::AbandonedPackets.insert(flit.getPacketId());
        }
        if (Global::AbandonedPackets.count(flit.getPacketId()) > 0) {
            return;
        }
    }
    this->input_module_.addFlit(phy_idx, vc_idx, flit);
    this->power_module_.addBufferWritePwr(phy_idx, flit.getData());

    if (isHeader(flit.getFlitType())) {
        if (this->input_module_.getBufferSize(phy_idx, vc_idx) == 1) {
            this->input_module_.setState(phy_idx, vc_idx, VCStateType::ROUTING);
        }
        else {
            // Do nothing
        }
    }
    else {
        if (this->input_module_.getState(phy_idx, vc_idx) == VCStateType::INIT) {
            this->input_module_.setState(phy_idx, vc_idx, VCStateType::SW_AB);
        }
    }
}

void BaseRouter::decideRouting() {
    TimeType event_time = Global::getCurrTime();

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
                }
                else {
                    vcst = (this->input_module_.getBufferSize(0, each_vc) > 0 ? VCStateType::ROUTING : VCStateType::INIT);
                }
                this->input_module_.setState(0, each_vc, vcst);
            }
            else {
                this->input_module_.clearRouting(0, each_vc);
                this->routingAlg(des_t, sor_t, 0, each_vc);
                this->input_module_.setState(0, each_vc, VCStateType::VC_AB);
            }
        }
        else if (this->input_module_.getState(0, each_vc) == VCStateType::HOME) {
            if (this->input_module_.getBufferSize(0, each_vc) > 0) {
                Flit flit(this->input_module_.getFlit(0, each_vc));
                Sassert(!isHeader(flit.getFlitType()), "Error: Flit type is not HEADER");
                this->acceptFlit(event_time, flit);
                this->input_module_.removeFlit(0, each_vc);
                if (flit.getFlitType() == FlitType::TAIL) {
                    if (this->input_module_.getBufferSize(0, each_vc) > 0) {
                        this->input_module_.setState(0, each_vc, VCStateType::ROUTING);
                    }
                    else {
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
                    AddrType cre_add_t;
					long cre_pc_t;
                    this->getFromRouter(cre_add_t, each_phy);
                    cre_pc_t = this->getFromPort(each_phy);
                    Global::messageQueue.addMessage(
					    MessEvent(event_time + CREDIT_DELAY_,
                            MessType::CREDIT, this->address_,
                            cre_add_t, cre_pc_t, each_vc
                        )
                    );
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
                    }
                    else {
                        vcst = (this->input_module_.getBufferSize(each_phy, each_vc) > 0 ? VCStateType::ROUTING : VCStateType::INIT);
                    }
                    this->input_module_.setState(each_phy, each_vc, vcst);
                }
                else {
                    this->input_module_.clearRouting(each_phy, each_vc);
                    this->routingAlg(des_t, sor_t, each_phy, each_vc);
                    this->input_module_.setState(each_phy, each_vc, VCStateType::VC_AB);
                }
            }
            else if (this->input_module_.getState(each_phy, each_vc) == VCStateType::HOME) {
                if (this->input_module_.getBufferSize(each_phy, each_vc) > 0) {
                    Flit flit(this->input_module_.getFlit(each_phy, each_vc));
                    Sassert(!isHeader(flit.getFlitType()), "Error: Flit type is not HEADER");
                    this->acceptFlit(event_time, flit);
                    this->input_module_.removeFlit(each_phy, each_vc);
                    if (flit.getFlitType() == FlitType::TAIL) {
                        if (this->input_module_.getBufferSize(each_phy, each_vc) > 0) {
                            this->input_module_.setState(each_phy, each_vc, VCStateType::ROUTING);
                        }
                        else {
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
    std::vector<VCType> vc_can_t = this->input_module_.getRouting(phy_idx, vc_idx);
    Sassert(vc_can_t.size() > 0, "Error: No available VC");
    std::vector<VCType> vc_acq_t;

    for (auto& vc : vc_can_t) {
        if (this->config_.getVcShare() == VCShareType::SHARE) {
            if (this->output_module_.getUsage(vc.first, vc.second) == VCUsageType::FREE) {
                vc_acq_t.push_back(vc);
            }
        }
        else {
            if (this->output_module_.getCounter(vc.first, vc.second) == this->inbuffer_size_) {
                if (this->output_module_.getUsage(vc.first, vc.second) == VCUsageType::FREE) {
                    vc_acq_t.push_back(vc);
                }
            }
        }
    }

    if (vc_acq_t.size() == 0) {
        return VCType(-1, -1);
    }
    else if (vc_acq_t.size() == 1) {
        return vc_acq_t[0];
    }    
    return vc_acq_t[Global::RandomGen.random_long(0, vc_acq_t.size())];
}

void BaseRouter::arbitrationVC() {
    std::map<VCType, std::vector<VCType>> vc_o_i_map;
	AtomType vc_request = 0;

	for (long i = 0; i < this->physic_ports_; i++) {
		for (long j = 0; j < this->vc_number_; j++) {
			VCType vc_t;
			if (this->input_module_.getState(i, j) == VCStateType::VC_AB) {
				vc_t = this->selectVC(i,j);
				if ((vc_t.first >= 0) && (vc_t.second >= 0)) {
					vc_o_i_map[vc_t].push_back(VCType(i, j));
					vc_request = vc_request | Global::VC_MASK[i * vc_number_ + j];
				}
			}
		}
	}
	if (vc_o_i_map.size() == 0) {
		return;
	}
    for (long i= 1; i < this->physic_ports_; i++) {
		for (long j = 0; j < this->vc_number_; j++) {
			if (this->output_module_.getUsage(i, j) == VCUsageType::FREE) {
				long cont_temp = vc_o_i_map[VCType(i, j)].size();
				if (cont_temp > 0) {
					VCType vc_win = vc_o_i_map[VCType(i, j)][0];
					if (cont_temp > 1) {
						vc_win = vc_o_i_map[VCType(i, j)]
                            [Global::RandomGen.random_long(0, cont_temp)];
					}
					this->input_module_.setState(vc_win.first,
                        vc_win.second, VCStateType::SW_AB);
					this->input_module_.setCRouting(vc_win.first,
                        vc_win.second, VCType(i,j));
					this->output_module_.acquireChannel(i, j, vc_win);
					this->power_module_.addVCArbitPwr(i, j, vc_request,
                        (vc_win.first) * vc_number_ + (vc_win.second));
				}
			}
		}
	}
}

void BaseRouter::arbitrationSW() {
    std::map<long, std::vector<VCType>> vc_o_map;
	for (long i = 0; i < physic_ports_; i++) {
		std::vector<long> vc_i_t;
		for (long j = 0; j < vc_number_; j++) {
			if (this->input_module_.getState(i, j) == VCStateType::SW_AB) {
				VCType out_t = this->input_module_.getCRouting(i, j);
				if ((this->output_module_.getCounter(out_t.first, out_t.second) > 0) &&
					(this->output_module_.getLocalCounter(out_t.first) > 0))
                {
					vc_i_t.push_back(j);
				}
			}
		}
		long vc_size_t = vc_i_t.size();
		if (vc_size_t > 1) {
			long win_t = Global::RandomGen.random_long(0, vc_size_t);
			VCType r_t = this->input_module_.getCRouting(i, vc_i_t[win_t]);
			vc_o_map[r_t.first].push_back(VCType(i, vc_i_t[win_t]));
		}
		else if (vc_size_t == 1) {
			VCType r_t = this->input_module_.getCRouting(i, vc_i_t[0]);
			vc_o_map[r_t.first].push_back(VCType(i, vc_i_t[0]));
		}
	}

	if (vc_o_map.size() == 0) {
		return;
	}

	for (long i = 0; i < this->physic_ports_; i++) {
		long vc_size_t = vc_o_map[i].size();
		if (vc_size_t > 0) {
			VCType vc_win = vc_o_map[i][0];
			if (vc_size_t > 1) {
				vc_win = vc_o_map[i][Global::RandomGen.random_long(0, vc_size_t)];
			}
			this->input_module_.setState(vc_win.first, vc_win.second, VCStateType::SW_TR);
			Flit &flit_t = this->input_module_.getFlit(vc_win.first, vc_win.second);
		}
	}
}

void BaseRouter::toOutBuffer() {
    for (long i = 0; i < this->physic_ports_; i++) {
		for (long j = 0; j < this->vc_number_; j++) {
			if (this->input_module_.getState(i, j) == VCStateType::SW_TR) {
				VCType out_t = this->input_module_.getCRouting(i, j);
				this->output_module_.decCounter(out_t.first, out_t.second);

				TimeType event_time = Global::getCurrTime();
				if (i != 0) {
					AddrType cre_add_t;
					long cre_pc_t;
					this->getFromRouter(cre_add_t, i);
					cre_pc_t = this->getFromPort(i);
					Global::messageQueue.addMessage(
						MessEvent(event_time + CREDIT_DELAY_,
							MessType::CREDIT, this->address_,
                            cre_add_t, cre_pc_t, j
                        )
                    );
				}

				long in_size_t = this->input_module_.getBufferSize(i, j);
				Sassert(in_size_t >= 1, "Error: Buffer size is less than 1");
				Flit flit_t(input_module_.getFlit(i, j));
				this->input_module_.removeFlit(i, j);
				this->power_module_.addBufferReadPwr(i, flit_t.getData());
				this->power_module_.addCrossbarTravPwr(i, out_t.first, flit_t.getData());
				this->output_module_.addFlit(out_t.first, flit_t);
				if (i == 0) {
					if (this->input_module_.isIBuffFull() == true) {
						if (this->input_module_.getBufferSize(0, j) < BUFF_BOUND_) {
							this->input_module_.setIBuffFull(false);
							this->recvPacket();
						}
					}
				}
				this->output_module_.addAddr(out_t.first, out_t);
				if (isTail(flit_t.getFlitType())) {
					this->output_module_.releaseChannel(out_t.first, out_t.second);
				}
				if (in_size_t > 1) {
					if (isTail(flit_t.getFlitType())) {
						if (this->config_.getVcShare() == VCShareType::MONO) {
							if (i != 0) {
								if (in_size_t != 1) {
                                    Logger::info("{}: {}", i, in_size_t);
								}
								Sassert(in_size_t == 1, "Error: Buffer size is not 1");
							}
						}
						this->input_module_.setState(i, j, VCStateType::ROUTING);
					}
					else {
						this->input_module_.setState(i, j, VCStateType::SW_AB);
					}
				}
				else {
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
    TimeType event_time = Global::getCurrTime();
	if (this->output_module_.getOutBufferSize(port) > 0) {
		TimeType delay = this->getWireDelay(port);
		TimeType flit_delay_t = delay + event_time;
		AddrType wire_add_t;
		long wire_pc_t = getWirePc(port);
		this->getNextAddress(wire_add_t, port);
		Flit flit_t(this->output_module_.getFlit(port));
		VCType outadd_t = this->output_module_.getAddr(port);
		this->power_module_.addLinkTravPwr(port, flit_t.getData());

		this->output_module_.removeFlit(port);
		this->output_module_.removeAddr(port);
		Global::messageQueue.addMessage(
            MessEvent(flit_delay_t, MessType::WIRE,
                this->address_, wire_add_t,
                wire_pc_t, outadd_t.second, flit_t
            )
        );
	}
}

void BaseRouter::XY_algorithm(const AddrType& des_t, const AddrType& sor_t, long s_ph, long s_vc) {
    long xoffset = des_t[0] - this->address_[0];
	long yoffset = des_t[1] - this->address_[1];
    if (this->vc_number_ < 4) {
        Logger::warn("VC number is less than 4, XY algorithm may not work properly");
    }
	if (yoffset < 0) {
        for (int i = 0; i < 4 && i < this->vc_number_; i++) {
            this->input_module_.addRouting(s_ph, s_vc, VCType(3, i));
        }
	}
    else if (yoffset > 0) {
		for (int i = 0; i < 4 && i < this->vc_number_; i++) {
            this->input_module_.addRouting(s_ph, s_vc, VCType(4, i));
        }
	}
    else {
		if (xoffset < 0) {
			for (int i = 0; i < 4 && i < this->vc_number_; i++) {
                this->input_module_.addRouting(s_ph, s_vc, VCType(1, i));
            }
		}
        else if (xoffset > 0) {
			for (int i = 0; i < 4 && i < this->vc_number_; i++) {
                this->input_module_.addRouting(s_ph, s_vc, VCType(2, i));
            }
		}
	}
}
	
void BaseRouter::TXY_algorithm(const AddrType& des_t, const AddrType& sor_t, long s_ph, long s_vc) {
    Sassert(this->vc_number_ >= 2, "Error: VC number is less than 2, TXY algorithm should not be used");
    
    long xoffset = des_t[0] - address_[0];
	long yoffset = des_t[1] - address_[1];
	bool xdirection = (abs(static_cast<int>(xoffset)) * 2 <= this->ary_size_) ? true: false; 
	bool ydirection = (abs(static_cast<int>(yoffset)) * 2 <= this->ary_size_) ? true: false; 

	if (xdirection) {
		if (xoffset < 0) {
			this->input_module_.addRouting(s_ph, s_vc, VCType(1, 0));
		}
        else if (xoffset > 0) {
			this->input_module_.addRouting(s_ph, s_vc, VCType(2, 1));
		}
        else {
			if (ydirection) {
				if (yoffset < 0) {
					this->input_module_.addRouting(s_ph, s_vc, VCType(3, 0));
				}
                else if(yoffset > 0) {
					this->input_module_.addRouting(s_ph, s_vc, VCType(4, 1));
				}
			}
            else {
				if (yoffset < 0) {
					this->input_module_.addRouting(s_ph, s_vc, VCType(4, 0));
				}
                else if (yoffset > 0) {
					this->input_module_.addRouting(s_ph, s_vc, VCType(3, 1)); 
				}
			}
		}
	}
    else  {
		if (xoffset < 0) {
			this->input_module_.addRouting(s_ph, s_vc, VCType(2, 0));
		}
        else if (xoffset > 0) {
			this->input_module_.addRouting(s_ph, s_vc, VCType(1, 1));
		}
        else {
			if (ydirection) {
				if (yoffset < 0) {
					this->input_module_.addRouting(s_ph, s_vc, VCType(3, 0));
				}
                else if (yoffset > 0) {
					this->input_module_.addRouting(s_ph, s_vc, VCType(4, 1));
				}
			}
            else {
				if (yoffset < 0) {
					this->input_module_.addRouting(s_ph, s_vc, VCType(4, 0));
				}
                else if (yoffset> 0) {
					this->input_module_.addRouting(s_ph, s_vc, VCType(3, 1)); 
				}
			}
		}
	}
}

void BaseRouter::chiplet_routing_alg(const AddrType& des_t,const AddrType& sor_t, long s_ph, long s_vc) {
    const long VIRTUAL_CHANNEL_COUNT = 2;
	AddrType delta;
	size_t addr_len = des_t.size();
	
    delta.reserve(addr_len);
	
    for (long i = 0; i < addr_len; ++i) {
        delta.push_back(des_t[i] - this->address_[i]);
    }
	
    if (delta[0] == 0 && delta[1] == 0) {
		VCType vc;
		if (delta[2] == 0) {
            vc.first = (delta[3] < 0 ? 7 : 8);
        }
		else {
            vc.first = (delta[2] < 0 ? 5 : 6);
        }
		for (long i = 0; i < VIRTUAL_CHANNEL_COUNT; ++i){
			vc.second = i;
			this->input_module_.addRouting(s_ph, s_vc, vc);
		}
	}
    else {
		long vc_first;
		if (this->address_[2] == 0 && this->address_[3]==0) {
			if(delta[0] == 0) {
                vc_first = (delta[1] < 0 ? 3 : 4);
            }
			else {
                vc_first = (delta[0] < 0 ? 1 : 2);
            }
			for (long i = 0; i < VIRTUAL_CHANNEL_COUNT; ++i) {
                this->input_module_.addRouting(s_ph, s_vc, VCType(vc_first, i));
            }
		}
        else {
			if(this->address_[2] > 0) {
                vc_first = 5;
            }
			else {
                vc_first = 7;
            }
			for (long i=0;i<VIRTUAL_CHANNEL_COUNT;++i) {
                this->input_module_.addRouting(s_ph, s_vc, VCType(vc_first, i));
            }
		}
	}
}
    
void BaseRouter::chiplet_star_topo_routing_alg(const AddrType& des_t,const AddrType& src_t, long s_ph, long s_vc) {
    long addrLen = this->address_.size();
	AddrType delta;
	long i, port;
	for (long i = 0;i<addrLen;++i) {
        delta.push_back(des_t[i] - this->address_[i]);
    }
    
	long centerXY = this->ary_size_-1;
	if (isCentral(this->address_, centerXY)) {
		for (long i = 0; i < 2; ++i){
		    this->input_module_.addRouting(s_ph, s_vc, VCType(des_t[0] * centerXY + des_t[1] + 1, i));
	    }
    }
    else if (delta[0] == 0 && delta[1] == 0) {
		if (delta[2] == 0) {
            if (delta[3] != 0) {
                port = (3 << 1) + (delta[3] > 0 ? 2 : 1);
            }
        }
        else {
            port = (2 << 1) + (delta[2] > 0 ? 2 : 1);
        }
        for (long i = 0; i < 2; ++i){
		    this->input_module_.addRouting(s_ph, s_vc, VCType(port, i));
	    }
	}
    else {
		if (this->address_[2] == 0 && this->address_[3] ==0) {
            for (long i = 0; i < 2; ++i){
		        this->input_module_.addRouting(s_ph, s_vc, VCType((addrLen << 1) + 1, i));
	        }
		}
        else {
            port = (this->address_[2] > 0 ? 5 : 7);
			for (long i = 0; i < 2; ++i){
		        this->input_module_.addRouting(s_ph, s_vc, VCType(port, i));
	        }
		}
	}
}

BaseRouter::BaseRouter(const Config& config, AddrType addr, std::vector<BaseRouter *>& router_list)
:   physic_ports_(config.getPhysicalPortNumber()),
    vc_number_(config.getVirtualChannelNumber()),
    inbuffer_size_(config.getInBufferSize()),
    outbuffer_size_(config.getOutBufferSize()),
    address_(addr),
    ary_size_(config.getAryNumber()),
    flit_size_(config.getFlitSize()),
    config_(config),
    input_module_(config.getPhysicalPortNumber(), config.getVirtualChannelNumber()),
    output_module_(config.getPhysicalPortNumber(), config.getVirtualChannelNumber(), config.getInBufferSize(), config.getOutBufferSize()),
    power_module_(config.getPhysicalPortNumber(), config.getVirtualChannelNumber(), config.getFlitSize(), config.getLinkLength()),
    init_data_(),
    total_delay_(0),
    routing_alg_(config.getRoutingAlg()),
    curr_algorithm(0),
    local_time_(LOCAL_INPUT_TIME_0_0),
    packet_counter_(0),
    router_list_(router_list)
{
    this->init_data_.resize(this->flit_size_);
    for (long i = 0; i < this->flit_size_; i++) {
        this->init_data_[i] = Global::RandomGen.random_u_long_long(0, MAX_64_);
    }
    this->setRoutingType();
}

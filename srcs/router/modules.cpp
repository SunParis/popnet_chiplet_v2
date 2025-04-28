# include "router/modules.h"

/**
 * @brief Construct a new Input Modules object
 * @param phy_port_num the number of physical ports
 * @param vc_num the number of virtual channels
 */
InputModules::InputModules(long phy_port_num, long vc_num)
:  ibuff_full_(false)
{    
    this->input_.resize(phy_port_num);
    for (long i = 0; i < phy_port_num; i++) {
        this->input_[i].resize(vc_num);
    }
    
    this->states_.resize(phy_port_num);
    for (long i = 0; i < phy_port_num; i++) {
        this->states_[i].resize(vc_num, VCStateType::INIT);
    }

    this->routing_.resize(phy_port_num);
    for (long i = 0; i < phy_port_num; i++) {
        this->routing_[i].resize(vc_num);
    }

    this->crouting_.resize(phy_port_num);
    for (long i = 0; i < phy_port_num; i++) {
        this->crouting_[i].resize(vc_num, VC_NULL);
    }
}

/**
 * @brief If the input buffer is full
 */
bool InputModules::isIBuffFull() const {
    return this->ibuff_full_;
}

/**
 * @brief set the input buffer is full or not
 */
void InputModules::setIBuffFull(bool is_buffer_full) {
    this->ibuff_full_ = is_buffer_full;
}

/**
 * @brief Get the buffer size
 * @param pc the physical port
 * @param vc the virtual channel
 * @return the buffer size
 */
std::size_t InputModules::getBufferSize(long pc, long vc) const {
    try {
        return this->input_.at(pc).at(vc).size();
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
    return this->input_.at(pc).at(vc).size();
}

/**
 * @brief set the state
 * @param pc the physical port
 * @param vc the virtual channel
 * @param state the new state
 */
void InputModules::setState(long pc, long vc, VCStateType state) {
    try {
        this->states_.at(pc).at(vc) = state;
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief get the state
 * @param pc the physical port
 * @param vc the virtual channel
 * @return the state
 */
VCStateType InputModules::getState(long pc, long vc) const {
    try {
        return this->states_.at(pc).at(vc);
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
    return this->states_.at(pc).at(vc);
}

/**
 * @brief add a flit
 * @param pc the physical port
 * @param vc the virtual channel
 * @param flit the flit
 */
void InputModules::addFlit(long pc, long vc, const Flit& flit) {
    try {
        this->input_.at(pc).at(vc).push_back(flit);
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief Get the flit
 * @param pc the physical port
 * @param vc the virtual channel
 * @return the flit
 */
Flit& InputModules::getFlit(long pc, long vc) {
    try {
        return this->input_.at(pc).at(vc).front();
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
    return this->input_.at(pc).at(vc).front();
}

/**
 * @brief Remove the flit
 * @param pc the physical port
 * @param vc the virtual channel
 * @param flit the flit
 */
void InputModules::removeFlit(long pc, long vc) {
    try {
        this->input_.at(pc).at(vc).pop_front();
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief Get the routing
 * @param pc the physical port
 * @param vc the virtual channel
 * @return the routing
 */
std::vector<VCType>& InputModules::getRouting(long pc, long vc) {
    try {
        return this->routing_.at(pc).at(vc);
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
    return this->routing_.at(pc).at(vc);
}

/**
 * @brief Add the routing
 * @param pc the physical port
 * @param vc the virtual channel
 * @param vc_type the virtual channel type
 */
void InputModules::addRouting(long pc, long vc, VCType vc_type) {
    try {
        this->routing_.at(pc).at(vc).push_back(vc_type);
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief Clear the routing
 * @param pc the physical port
 * @param vc the virtual channel
 */
void InputModules::clearRouting(long pc, long vc) {
    try {
        this->routing_.at(pc).at(vc).clear();
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief Set the crouting
 * @param pc the physical port
 * @param vc the virtual channel
 * @param vc_type the virtual channel type
 */
void InputModules::setCRouting(long pc, long vc, VCType vc_type) {
    try {
        this->crouting_.at(pc).at(vc) = vc_type;
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief Get the CRouting
 * @param pc the physical port
 * @param vc the virtual channel
 */
VCType InputModules::getCRouting(long pc, long vc) {
    try {
        return this->crouting_.at(pc).at(vc);
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
    return this->crouting_.at(pc).at(vc);
}

/**
 * @brief Construct a new Output Modules object
 * @param phy_port_num the number of physical ports
 * @param vc_num the number of virtual channels
 * @param input_buffer_size the size of input buffer
 * @param output_buffer_size the size of output buffer
 */
OutputModules::OutputModules(long phy_port_num, long vc_num, long input_buffer_size, long output_buffer_size)
:   buffer_size_(input_buffer_size)
{    
    this->counter_.resize(phy_port_num);
    for (long i = 0; i < phy_port_num; i++) {
        this->counter_[i].resize(vc_num, input_buffer_size);
    }

    this->localcounter_.resize(phy_port_num, output_buffer_size);
	this->assign_.resize(phy_port_num);
	for (long i = 0; i < phy_port_num; i++) {
		this->assign_[i].resize(vc_num, VC_NULL);
	}
	
    this->usage_.resize(phy_port_num);
	for (long i = 0; i < phy_port_num; i++) {
		this->usage_[i].resize(vc_num, VCUsageType::FREE);
	}
	
    this->outbuffers_.resize(phy_port_num);
	this->flit_state_.resize(phy_port_num);
	this->outadd_.resize(phy_port_num);
}

/**
 * @brief increment the counter
 * @param phy_port the physical port
 * @param vc the virtual channel
 */
void OutputModules::incCounter(long phy_port, long vc) {
    try {
        this->counter_.at(phy_port).at(vc) += 1;
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief decrement the counter
 * @param phy_port the physical port
 * @param vc the virtual channel
 */
void OutputModules::decCounter(long phy_port, long vc) {
    try {
        this->counter_.at(phy_port).at(vc) -= 1;
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief get the counter
 * @param phy_port the physical port
 * @param vc the virtual channel
 * @return the counter
 */
long OutputModules::getCounter(long phy_port, long vc) {
    try {
        return this->counter_.at(phy_port).at(vc);
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
    return this->counter_.at(phy_port).at(vc);
}

/**
 * @brief Get the local counter
 * @param phy_port the physical port
 * @return the local counter
 */
long OutputModules::getLocalCounter(long phy_port) {
    try {
        return this->localcounter_.at(phy_port);
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
    return this->localcounter_.at(phy_port);
}

/**
 * @brief Get the usage
 * @param phy_port the physical port
 * @param vc the virtual channel
 * @return the usage
 */
VCUsageType OutputModules::getUsage(long phy_port, long vc) {
    try {
        return this->usage_.at(phy_port).at(vc);
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
    return this->usage_.at(phy_port).at(vc);
}

/**
 * @brief Acquire the channel
 * @param phy_port the physical port
 * @param vc the virtual channel
 * @param vc_type the vc_type
 */
void OutputModules::acquireChannel(long phy_port, long vc, VCType vc_type) {
    try {
        this->usage_.at(phy_port).at(vc) = VCUsageType::USED;
        this->assign_.at(phy_port).at(vc) = vc_type;
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief Release a channel
 * @param phy_port the physical port
 * @param vc the virtual channel
 */
void OutputModules::releaseChannel(long phy_port, long vc) {
    try {
        this->usage_.at(phy_port).at(vc) = VCUsageType::FREE;
        this->assign_.at(phy_port).at(vc) = VC_NULL;
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief Get the out buffer size
 * @param port the physical port
 * @return the out buffer size
 */
std::size_t OutputModules::getOutBufferSize(long port) {
    try {
        return this->outbuffers_.at(port).size();
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
    return this->outbuffers_.at(port).size();
}

/**
 * @brief Add a flit
 * @param port the physical port
 * @param flit the flit
 */
void OutputModules::addFlit(long port, const Flit& flit) {
    try {
        this->outbuffers_.at(port).push_back(flit);
        this->localcounter_.at(port) -= 1;
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief Get the flit
 * @param port the port
 * @return the flit
 */
Flit& OutputModules::getFlit(long port) {
    try {
        return this->outbuffers_.at(port).front();
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
    return this->outbuffers_.at(port).front();
}

/**
 * @brief Remove the flit
 * @param port the port
 */
void OutputModules::removeFlit(long port) {
    try {
        this->outbuffers_.at(port).pop_front();
        this->localcounter_.at(port) += 1;
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief Add a address
 * @param port the port
 * @param addr the address
 */
void OutputModules::addAddr(long port, VCType& addr) {
    try {
        this->outadd_.at(port).push_back(addr);
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief Get the address
 * @param port the port
 * @return the address
 */
VCType OutputModules::getAddr(long port) {
    try {
        return this->outadd_.at(port).front();
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
    return this->outadd_.at(port).front();
}

/**
 * @brief Remove the address
 * @param port the port
 */
void OutputModules::removeAddr(long port) {
    try {
        this->outadd_.at(port).pop_front();
    } catch (const std::out_of_range& e) {
        Sassert(false, e.what());
    }
}

/**
 * @brief Construct a new Power Modules object
 * @param phy_port_num the number of physical ports
 * @param vc_num the number of virtual ports
 * @param flit_size the size of flit
 * @param link_length the length of link
 */
PowerModules::PowerModules(long phy_port_num, long vc_num, long flit_size, long link_length)
:   flit_size_(flit_size)
{
    FUNC(SIM_router_power_init, &this->router_info_, &this->router_power_);
	this->buffer_write_.resize(phy_port_num);
	this->buffer_read_.resize(phy_port_num);
	this->crossbar_read_.resize(phy_port_num);
	this->crossbar_write_.resize(phy_port_num);
	this->link_traversal_.resize(phy_port_num);
	this->crossbar_input_.resize(phy_port_num, 0);
	for (long i = 0; i < phy_port_num; i++) {
		this->buffer_write_[i].resize(flit_size, 0);
		this->buffer_read_[i].resize(flit_size, 0);
		this->crossbar_read_[i].resize(flit_size, 0);
		this->crossbar_write_[i].resize(flit_size, 0);
		this->link_traversal_[i].resize(flit_size, 0);
	}
	SIM_arbiter_init(&this->arbiter_vc_power_, 1, 1, phy_port_num * vc_num, 0, NULL);
	this->arbiter_vc_req_.resize(phy_port_num);
	this->arbiter_vc_grant_.resize(phy_port_num);
	for (long i = 0; i < phy_port_num; i++) {
		this->arbiter_vc_req_[i].resize(vc_num, 1);
		this->arbiter_vc_grant_[i].resize(vc_num, 1);
	}
	SIM_bus_init(&this->link_power_, GENERIC_BUS, IDENT_ENC, ATOM_WIDTH_, 0, 1, 1, link_length, 0);
}

void PowerModules::addBufferReadPwr(long in_port, DataType& read_d) {
    for (long i = 0; i < this->flit_size_; i++) {
		FUNC(SIM_buf_power_data_read, 
            &this->router_info_.in_buf_info, 
            &this->router_power_.in_buf,
            read_d[i]
        );
		this->buffer_read_[in_port][i] = read_d[i];
	}
}

void PowerModules::addBufferWritePwr(long in_port, DataType& write_d) {
    
	for (long i = 0; i < this->flit_size_; i++) {
		VOLA AtomType ata[4] = {0};
		VOLA AtomType& old_d = ata[0];
		VOLA AtomType& new_d = ata[1];
		VOLA AtomType& old_d2 = ata[2];
		VOLA AtomType& new_d2 = ata[3];
		old_d = this->buffer_write_[in_port][i];
		new_d = write_d[i];
		old_d2 = this->buffer_write_[in_port][i];
		new_d2 = write_d[i];

		FUNC(SIM_buf_power_data_write,
            &this->router_info_.in_buf_info,
			&this->router_power_.in_buf,
            (char *)(&old_d),
            (char *)(&old_d),
            (char *)(&new_d));

		this->buffer_write_[in_port][i] = write_d[i];
	}
}

void PowerModules::addCrossbarTravPwr(long in_port, long out_port, DataType& trav_d) {
    for (long i = 0; i < this->flit_size_; i++) {
		SIM_crossbar_record(&this->router_power_.crossbar, 1,
            trav_d[i], this->crossbar_read_[in_port][i], 1, 1);
		
        SIM_crossbar_record(&this->router_power_.crossbar, 0,
            trav_d[i], this->crossbar_write_[out_port][i],
            this->crossbar_input_[out_port], in_port);

		this->crossbar_read_[in_port][i] = trav_d[i];
		this->crossbar_write_[out_port][i] = trav_d[i];
		this->crossbar_input_[out_port] = in_port;
	}
}

void PowerModules::addVCArbitPwr(long pc, long vc, AtomType req, unsigned long gra) {
    SIM_arbiter_record(&this->arbiter_vc_power_, req, this->arbiter_vc_req_[pc][vc],
        gra, this->arbiter_vc_grant_[pc][vc]);
	this->arbiter_vc_req_[pc][vc] = req;
	this->arbiter_vc_grant_[pc][vc] = gra;
}

void PowerModules::addLinkTravPwr(long in_port, DataType& read_d) {
    for (long i = 0; i < this->flit_size_; i++) {
		AtomType old_d = this->link_traversal_[in_port][i];
		AtomType new_d = read_d[i];
		SIM_bus_record(&this->link_power_, old_d, new_d);
		this->link_traversal_[in_port][i] = read_d[i];
	}
}

double PowerModules::getBufferPower() {
    return SIM_array_power_report(&this->router_info_.in_buf_info,
        &this->router_power_.in_buf);
}

double PowerModules::getLinkPower() {
    return SIM_bus_report(&this->link_power_);
}

double PowerModules::getCrossbarPower() {
    return SIM_crossbar_report(&this->router_power_.crossbar);
}

double PowerModules::getArbiterPower() {
    return SIM_arbiter_report(&this->arbiter_vc_power_);
}
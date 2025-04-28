# include "global_defines/message_define.h"


/**
 * @brief Set the start time of the message event
 * @param event_start The start time of the message event
 */
void MessEvent::setEventStart(TimeType event_start) {
    this->start_time_ = event_start;
}

/**
 * @brief Get the start time of the message event
 * @return The start time of the message event
 */
TimeType MessEvent::getEventStart() const {
    return this->start_time_;
}

/**
 * @brief Get the type of the message event
 * @return The type of the message event
 */
MessType MessEvent::getEventType() const {
    return this->mess_type_;
}


/**
 * @brief Get the source address of the message event
 * @return The source address of the message event
 */
AddrType& MessEvent::getSrc() {
    return this->src_addr_;
}


/**
 * @brief Get the source address of the message event (const version)
 * @return The source address of the message event (const version)
 */
const AddrType& MessEvent::getSrc() const {
    return this->src_addr_;
}


/**
 * @brief Get the destination address of the message event
 * @return The destination address of the message event
 */
AddrType& MessEvent::getDes() {
    return this->des_addr_;
}


/**
 * @brief Get the destination address of the message event (const version)
 * @return The destination address of the message event (const version)
 */
const AddrType& MessEvent::getDes() const {
    return this->des_addr_;
}


/**
 * @brief Get the PC of the message event
 * @return The PC of the message event
 */
long MessEvent::getPC() const {
    return this->pc_;
}


/**
 * @brief Get the VC of the message event
 * @return The VC of the message event
 */
long MessEvent::getVC() const {
    return this->vc_;
}


/**
 * @brief Get the flit of the message event
 * @return The flit of the message event
 */
Flit& MessEvent::getFlit() {
    return this->flit_;
}


/**
 * @brief Get the flit of the message event (const version)
 * @return The flit of the message event (const version)
 */
const Flit& MessEvent::getFlit() const {
    return this->flit_;
}


/**
 * @brief Get the routing period of the message event
 * @return The routing period of the message event
 */
TimeType MessEvent::getRoutingPeriod() const {
    return this->routing_period_;
}

/**
 * @brief Construct a new MessEvent object
 * @param start_time The start time of the message event
 * @param mess_type The type of the message event
 * @param routing_period The routing period of the message event
 */
MessEvent::MessEvent(TimeType start_time, MessType mess_type, TimeType routing_period)
:   start_time_(start_time),
    mess_type_(mess_type),
    routing_period_(routing_period),
    pc_(0),
    vc_(0),
    flit_()
{}


/**
 * @brief Construct a new MessEvent object
 * @param start_time The start time of the message event
 * @param mess_type The type of the message event
 * @param src_addr The source address of the message event
 * @param des_addr The destination address of the message event
 * @param pc The PC of the message event
 * @param vc The VC of the message event
 */
MessEvent::MessEvent(TimeType start_time, MessType mess_type,
    const AddrType& src_addr, const AddrType& des_addr,
    long pc, long vc)
:   start_time_(start_time),
    mess_type_(mess_type),
    src_addr_(src_addr),
    des_addr_(des_addr),
    pc_(pc),
    vc_(vc),
    flit_()
{}


/**
 * @brief Construct a new MessEvent object
 * @param start_time The start time of the message event
 * @param mess_type The type of the message event
 * @param src_addr The source address of the message event
 * @param des_addr The destination address of the message event
 * @param pc The PC of the message event
 * @param vc The VC of the message event
 * @param flit The flit of the message event
 */
MessEvent::MessEvent(TimeType start_time, MessType mess_type,
    const AddrType& src_addr, const AddrType& des_addr,
    long pc, long vc, const Flit& flit)
:   start_time_(start_time),
    mess_type_(mess_type),
    src_addr_(src_addr),
    des_addr_(des_addr),
    pc_(pc),
    vc_(vc),
    flit_(flit)
{}

/**
 * @brief Construct a new MessEvent object
 * @param start_time The start time of the message event
 * @param mess_type The type of the message event
 * @param flit The flit of the message event
 */
MessEvent::MessEvent(TimeType start_time, MessType mess_type, const Flit& flit)
:   start_time_(start_time),
    mess_type_(mess_type),
    pc_(0),
    vc_(0),
    flit_(flit)
{}


/**
 * @brief Construct a new MessEvent object by copying another MessEvent object
 * @param me The MessEvent object to be copied
 */
MessEvent::MessEvent(const MessEvent& me)
:   start_time_(me.getEventStart()),
    mess_type_(me.getEventType()),
    src_addr_(me.getSrc()),
    des_addr_(me.getDes()),
    pc_(me.getPC()),
    vc_(me.getVC()),
    flit_(me.getFlit()),
    routing_period_(me.getRoutingPeriod())
{}


/**
 * @brief operator<< overload for MessEvent
 */
std::ostream& operator<<(std::ostream& os, const MessEvent& me) {
    os << "Start time " << me.getEventStart() << ", ";
    os << "from ";
    os << me.getSrc() << " ";
    os << "to ";
    os << me.getDes() << " ";
    os << "(" << me.getEventType() << ")";
    return os;
}

bool MessEvent::operator<(const MessEvent& me) const {
    return this->start_time_ < me.start_time_;
}

bool MessEvent::operator>(const MessEvent& me) const {
    return this->start_time_ > me.start_time_;
}

void MessQueue::clear() {
    this->m_q_.clear();
    this->size_ = 0;
}

void MessQueue::clear(MessType mess_type) {
    if (this->m_q_.find(mess_type) == this->m_q_.end()) {
        return;
    }
    this->size_ -= this->m_q_[mess_type].size();
    this->m_q_[mess_type] = std::priority_queue<MessEvent, std::vector<MessEvent>, std::greater<MessEvent>>();
}

void MessQueue::addMessage(const MessEvent& event) {
    MessType mess_type = event.getEventType();
    if (this->m_q_.find(mess_type) == this->m_q_.end()) {
        this->m_q_[mess_type] = std::priority_queue<MessEvent, std::vector<MessEvent>, std::greater<MessEvent>>();
    }
    this->m_q_[mess_type].push(event);
    this->size_ += 1;
}

void MessQueue::popFront() {
    const MessEvent& top = this->getTop();
    this->m_q_[top.getEventType()].pop();
    this->size_--;
}

const MessEvent& MessQueue::getTop() const {
    if (this->m_q_.empty() || this->size_ == 0) {
        throw std::runtime_error("Message queue is empty.");
    }
    auto ret = this->m_q_.cbegin();
    for (ret = this->m_q_.cbegin(); ret != this->m_q_.cend(); ++ret) {
        if (ret->second.empty()) {
            continue;
        }
        break;
    }
    for (auto iter = ret; iter != this->m_q_.cend(); ++iter) {
        if (iter->second.empty()) {
            continue;
        }
        if (iter->second.top() < ret->second.top()) {
            ret = iter;
        }
    }
    return ret->second.top();
}

const MessEvent& MessQueue::getTop(MessType mess_type) const {
    auto iter = this->m_q_.find(mess_type);
    if (iter == this->m_q_.end()) {
        throw std::runtime_error("Message queue is empty.");
    }
    return iter->second.top();
}

bool MessQueue::empty() const {
    return this->size_ == 0;
}

bool MessQueue::empty(MessType mess_type) const {
    auto iter = this->m_q_.find(mess_type);
    if (iter == this->m_q_.end()) {
        return true;
    }
    return iter->second.size() == 0;
}

MessQueue::size_type MessQueue::size() const {
    return this->size_;
}

void MessQueue::updateEVGCycle(TimeType new_time) {

    if (this->m_q_.find(MessType::EVG) == this->m_q_.end()) {
        this->m_q_[MessType::EVG] = std::priority_queue<MessEvent, std::vector<MessEvent>, std::greater<MessEvent>>();
        this->addMessage(MessEvent(new_time, MessType::EVG));
        return;
    }
    if (this->m_q_[MessType::EVG].empty()) {
        this->addMessage(MessEvent(new_time, MessType::EVG));
        return;
    }
    if (this->m_q_[MessType::EVG].top().getEventStart() >= new_time) {
        MessEvent new_event(this->m_q_[MessType::EVG].top());
        new_event.setEventStart(new_time);
        this->m_q_[MessType::EVG].pop();
        this->m_q_[MessType::EVG].push(new_event);
        Logger::info("Update EVG cycle to {}.", new_time);
    }
}

MessQueue::MessQueue()
    : m_q_()
{
    this->size_ = 0;
}

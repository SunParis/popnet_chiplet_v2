#include "global_defines/message_define.h"

#include <algorithm>
#include <cmath>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "logger/logger.hpp"

MessEvent::MessEvent(TimeType start_time, MessType mess_type, TimeType routing_period)
    : start_time_(start_time), mess_type_(mess_type), routing_period_(routing_period) {}

MessEvent::MessEvent(TimeType start_time, MessType mess_type, const AddrType& src_addr,
                     const AddrType& des_addr, long pc, long vc)
    : start_time_(start_time), mess_type_(mess_type), src_addr_(src_addr), des_addr_(des_addr),
      pc_(pc), vc_(vc) {}

MessEvent::MessEvent(TimeType start_time, MessType mess_type, const AddrType& src_addr,
                     const AddrType& des_addr, long pc, long vc, const Flit& flit)
    : start_time_(start_time), mess_type_(mess_type), src_addr_(src_addr), des_addr_(des_addr),
      pc_(pc), vc_(vc), flit_(flit) {}

MessEvent::MessEvent(TimeType start_time, MessType mess_type, const AddrType& src_addr,
                     const AddrType& des_addr, long pc, long vc, Flit&& flit)
    : start_time_(start_time), mess_type_(mess_type), src_addr_(src_addr), des_addr_(des_addr),
      pc_(pc), vc_(vc), flit_(std::move(flit)) {}

MessEvent::MessEvent(TimeType start_time, MessType mess_type, const Flit& flit)
    : start_time_(start_time), mess_type_(mess_type), flit_(flit) {}

MessEvent::MessEvent(TimeType start_time, MessType mess_type, Flit&& flit)
    : start_time_(start_time), mess_type_(mess_type), flit_(std::move(flit)) {}

void MessEvent::setEventStart(TimeType event_start) noexcept {
    start_time_ = event_start;
}

TimeType MessEvent::getEventStart() const noexcept {
    return start_time_;
}

MessType MessEvent::getEventType() const noexcept {
    return mess_type_;
}

AddrType& MessEvent::getSrc() noexcept {
    return src_addr_;
}

const AddrType& MessEvent::getSrc() const noexcept {
    return src_addr_;
}

AddrType& MessEvent::getDes() noexcept {
    return des_addr_;
}

const AddrType& MessEvent::getDes() const noexcept {
    return des_addr_;
}

long MessEvent::getPC() const noexcept {
    return pc_;
}

long MessEvent::getVC() const noexcept {
    return vc_;
}

Flit& MessEvent::getFlit() noexcept {
    return flit_;
}

const Flit& MessEvent::getFlit() const noexcept {
    return flit_;
}

TimeType MessEvent::getRoutingPeriod() const noexcept {
    return routing_period_;
}

bool MessEvent::operator<(const MessEvent& other) const noexcept {
    return start_time_ < other.start_time_;
}

bool MessEvent::operator>(const MessEvent& other) const noexcept {
    return start_time_ > other.start_time_;
}

std::ostream& operator<<(std::ostream& output, const MessEvent& event) {
    output << "Start time " << event.getEventStart() << ", from " << event.getSrc() << " to "
           << event.getDes() << " (" << event.getEventType() << ')';
    return output;
}

bool MessQueue::EventLater::operator()(const QueuedEvent& left,
                                       const QueuedEvent& right) const noexcept {
    if (left.event.getEventStart() != right.event.getEventStart()) {
        return left.event.getEventStart() > right.event.getEventStart();
    }
    const auto left_phase = MessQueue::phasePriority(left.event.getEventType());
    const auto right_phase = MessQueue::phasePriority(right.event.getEventType());
    if (left_phase != right_phase) {
        return left_phase > right_phase;
    }
    return left.sequence > right.sequence;
}

std::size_t MessQueue::index(MessType mess_type) noexcept {
    return static_cast<std::size_t>(mess_type);
}

std::size_t MessQueue::phasePriority(MessType mess_type) noexcept {
    switch (mess_type) {
    case MessType::CREDIT:
        return 0;
    case MessType::WIRE:
        return 1;
    case MessType::RECONFIGURATION:
        return 2;
    case MessType::ROUTER:
        return 3;
    case MessType::EVG:
        return 4;
    }
    return event_type_count;
}

void MessQueue::clear() noexcept {
    queues_ = {};
    size_ = 0;
}

void MessQueue::clear(MessType mess_type) noexcept {
    auto& queue = queues_[index(mess_type)];
    size_ -= queue.size();
    queue.clear();
}

void MessQueue::addMessage(const MessEvent& event) {
    auto& queue = queues_[index(event.getEventType())];
    queue.push_back({event, next_sequence_++});
    std::push_heap(queue.begin(), queue.end(), EventLater{});
    ++size_;
}

void MessQueue::addMessage(MessEvent&& event) {
    const auto event_index = index(event.getEventType());
    auto& queue = queues_[event_index];
    queue.push_back({std::move(event), next_sequence_++});
    std::push_heap(queue.begin(), queue.end(), EventLater{});
    ++size_;
}

void MessQueue::popFront() {
    static_cast<void>(popMessage());
}

MessEvent MessQueue::popMessage() {
    const auto queue_index = index(topQueuedEvent().event.getEventType());
    auto& queue = queues_[queue_index];
    std::pop_heap(queue.begin(), queue.end(), EventLater{});
    auto event = std::move(queue.back().event);
    queue.pop_back();
    --size_;
    return event;
}

const MessEvent& MessQueue::getTop() const {
    return topQueuedEvent().event;
}

const MessEvent& MessQueue::getTop(MessType mess_type) const {
    const auto& queue = queues_[index(mess_type)];
    if (queue.empty()) {
        throw std::out_of_range("Message queue is empty for requested event type");
    }
    return queue.front().event;
}

bool MessQueue::empty() const noexcept {
    return size_ == 0;
}

bool MessQueue::empty(MessType mess_type) const noexcept {
    return queues_[index(mess_type)].empty();
}

MessQueue::size_type MessQueue::size() const noexcept {
    return size_;
}

void MessQueue::updateEVGCycle(TimeType new_time) {
    auto& queue = queues_[index(MessType::EVG)];
    if (queue.empty()) {
        addMessage(MessEvent(new_time, MessType::EVG));
        return;
    }
    if (queue.front().event.getEventStart() >= new_time) {
        std::pop_heap(queue.begin(), queue.end(), EventLater{});
        auto event = std::move(queue.back().event);
        queue.pop_back();
        event.setEventStart(new_time);
        queue.push_back({std::move(event), next_sequence_++});
        std::push_heap(queue.begin(), queue.end(), EventLater{});
        Logger::info("Update EVG cycle to {}.", new_time);
    }
}

void MessQueue::advanceRouterCycles(TimeType target_time) {
    auto& queue = queues_[index(MessType::ROUTER)];
    for (auto& queued : queue) {
        auto& event = queued.event;
        const auto period = event.getRoutingPeriod();
        if (event.getEventStart() >= target_time || period <= 0) {
            continue;
        }
        const auto periods = std::ceil((target_time - event.getEventStart()) / period);
        event.setEventStart(event.getEventStart() + periods * period);
    }
    std::make_heap(queue.begin(), queue.end(), EventLater{});
}

const MessQueue::QueuedEvent& MessQueue::topQueuedEvent() const {
    if (empty()) {
        throw std::out_of_range("Message queue is empty");
    }

    const QueuedEvent* earliest = nullptr;
    for (const auto& queue : queues_) {
        if (queue.empty()) {
            continue;
        }
        const auto& candidate = queue.front();
        if (earliest == nullptr || EventLater{}(*earliest, candidate)) {
            earliest = &candidate;
        }
    }
    return *earliest;
}

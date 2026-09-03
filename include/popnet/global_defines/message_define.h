#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <vector>

#include "global_defines/defines.h"
#include "global_defines/packet_defines.h"

class MessEvent {
  public:
    MessEvent(TimeType start_time, MessType mess_type, TimeType routing_period = PIPE_DELAY_);
    MessEvent(TimeType start_time, MessType mess_type, const AddrType& src_addr,
              const AddrType& des_addr, long pc, long vc);
    MessEvent(TimeType start_time, MessType mess_type, const AddrType& src_addr,
              const AddrType& des_addr, long pc, long vc, const Flit& flit);
    MessEvent(TimeType start_time, MessType mess_type, const AddrType& src_addr,
              const AddrType& des_addr, long pc, long vc, Flit&& flit);
    MessEvent(TimeType start_time, MessType mess_type, const Flit& flit);
    MessEvent(TimeType start_time, MessType mess_type, Flit&& flit);

    void setEventStart(TimeType event_start) noexcept;
    TimeType getEventStart() const noexcept;
    MessType getEventType() const noexcept;
    AddrType& getSrc() noexcept;
    const AddrType& getSrc() const noexcept;
    AddrType& getDes() noexcept;
    const AddrType& getDes() const noexcept;
    long getPC() const noexcept;
    long getVC() const noexcept;
    Flit& getFlit() noexcept;
    const Flit& getFlit() const noexcept;
    TimeType getRoutingPeriod() const noexcept;

    bool operator<(const MessEvent& other) const noexcept;
    bool operator>(const MessEvent& other) const noexcept;

  private:
    TimeType start_time_{0};
    MessType mess_type_{MessType::EVG};
    AddrType src_addr_;
    AddrType des_addr_;
    long pc_{0};
    long vc_{0};
    TimeType routing_period_{PIPE_DELAY_};
    Flit flit_;
};

std::ostream& operator<<(std::ostream& output, const MessEvent& event);

class MessQueue {
  public:
    using size_type = std::size_t;

    void clear() noexcept;
    void clear(MessType mess_type) noexcept;
    void addMessage(const MessEvent& event);
    void addMessage(MessEvent&& event);
    void popFront();
    MessEvent popMessage();
    const MessEvent& getTop() const;
    const MessEvent& getTop(MessType mess_type) const;
    bool empty() const noexcept;
    bool empty(MessType mess_type) const noexcept;
    size_type size() const noexcept;
    void updateEVGCycle(TimeType new_time);
    void advanceRouterCycles(TimeType target_time);

  private:
    struct QueuedEvent {
        MessEvent event;
        std::uint64_t sequence;
    };

    struct EventLater {
        bool operator()(const QueuedEvent& left, const QueuedEvent& right) const noexcept;
    };

    using EventQueue = std::vector<QueuedEvent>;

    static constexpr std::size_t event_type_count = 5;

    static std::size_t index(MessType mess_type) noexcept;
    static std::size_t phasePriority(MessType mess_type) noexcept;
    const QueuedEvent& topQueuedEvent() const;

    std::array<EventQueue, event_type_count> queues_;
    size_type size_{0};
    std::uint64_t next_sequence_{0};
};

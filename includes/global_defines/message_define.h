# pragma once

/**
 * @file mess_event.h
 * @brief Definition of the MessEvent class
 */

# ifndef _MESS_EVENT_H_
# define _MESS_EVENT_H_ 1

# include <vector>
# include <queue>
# include <map>
# include <sstream>

# include "global_defines/defines.h"
# include "global_defines/packet_defines.h"
# include "logger/logger.hpp"

class MessEvent {

private:
	
    TimeType start_time_;
	MessType mess_type_;
	AddrType src_addr_;
	AddrType des_addr_;
    long pc_;
    long vc_;
    TimeType routing_period_;
	Flit flit_;

public:

    /**
     * @brief Set the start time of the message event
     * @param event_start The start time of the message event 
     */
    void setEventStart(TimeType event_start);

    /**
     * @brief Get the start time of the message event
     * @return The start time of the message event
     */
    TimeType getEventStart() const;

    /**
     * @brief Get the type of the message event
     * @return The type of the message event
     */
    MessType getEventType() const;

    /**
     * @brief Get the source address of the message event
     * @return The source address of the message event
     */
    AddrType& getSrc();

    /**
     * @brief Get the source address of the message event (const version)
     * @return The source address of the message event (const version)
     */
    const AddrType& getSrc() const;
    
    /**
     * @brief Get the destination address of the message event
     * @return The destination address of the message event
     */
    AddrType& getDes();

    /**
     * @brief Get the destination address of the message event (const version)
     * @return The destination address of the message event (const version)
     */
    const AddrType& getDes() const;

    /**
     * @brief Get the PC of the message event
     * @return The PC of the message event
     */
    long getPC() const;

    /**
     * @brief Get the VC of the message event
     * @return The VC of the message event
     */
    long getVC() const;

    /**
     * @brief Get the flit of the message event
     * @return The flit of the message event
     */
    Flit& getFlit();

    /**
     * @brief Get the flit of the message event (const version)
     * @return The flit of the message event (const version)
     */
    const Flit& getFlit() const;

    /**
     * @brief Get the routing period of the message event
     * @return The routing period of the message event
     */
    TimeType getRoutingPeriod() const;

    /**
     * @brief Construct a new MessEvent object
     * @param start_time The start time of the message event
     * @param mess_type The type of the message event
     * @param new_routing_period The new routing period of the message event
     */
    MessEvent(TimeType start_time, MessType mess_type, TimeType routing_period = PIPE_DELAY_);

    /**
     * @brief Construct a new MessEvent object
     * @param start_time The start time of the message event
     * @param mess_type The type of the message event
     * @param src_addr The source address of the message event
     * @param des_addr The destination address of the message event
     * @param pc The PC of the message event
     * @param vc The VC of the message event
     */
    MessEvent(TimeType start_time, MessType mess_type, const AddrType& src_addr, const AddrType& des_addr, long pc, long vc);

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
    MessEvent(TimeType start_time, MessType mess_type, const AddrType& src_addr, const AddrType& des_addr, long pc, long vc, const Flit& flit);

    /**
     * @brief Construct a new MessEvent object
     * @param start_time The start time of the message event
     * @param mess_type The type of the message event
     * @param flit The flit of the message event
     */
    MessEvent(TimeType start_time, MessType mess_type, const Flit& flit);

    /**
     * @brief Construct a new MessEvent object
     * @param me The MessEvent object to be copied
     */
    MessEvent(const MessEvent& me);

    /**
     * @brief operator<< overload for MessEvent
     */
    friend std::ostream& operator<<(std::ostream& os, const MessEvent& me);

    bool operator<(const MessEvent& me) const;

    bool operator>(const MessEvent& me) const;

};

class MessQueue {

public:

    using size_type = std::vector<MessEvent>::size_type;

private:
    
    size_type size_;

    std::map<MessType, std::priority_queue<MessEvent, std::vector<MessEvent>, std::greater<MessEvent>>> m_q_;

public:

    /**
     * @brief Clear the queue.
     */
    void clear();

    /**
     * @brief Clear the queue, with the specified message type.
     */
    void clear(MessType mess_type);

    /**
     * @brief Add a message to the queue.
     */
    void addMessage(const MessEvent& event);

    /**
     * @brief Remove the top message from the queue.
     */
    void popFront();
    
    /**
     * @brief Get the top message from the queue. (const version)
     */
    const MessEvent& getTop() const;

    /**
     * @brief Get the top message from the queue, with the specified message type. (const version)
     * @param mess_type The message type to be checked
     */
    const MessEvent& getTop(MessType mess_type) const;

    /**
     * @brief Check if the queue is empty.
     */
    bool empty() const;

    /**
     * @brief Check if the queue is empty, with the specified message type.
     */
    bool empty(MessType mess_type) const;

    /**
     * @brief Get the size of the queue.
     */
    size_type size() const;

    /**
     * @brief Update the start time of the samllest EVG event to the new time.
     * @param new_time The new time to be set
     */
    void updateEVGCycle(TimeType new_time);

    MessQueue();

};

# endif
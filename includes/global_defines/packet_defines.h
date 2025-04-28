# pragma once

/**
 * @file packet.h
 * @brief This file contains the definition of the SPacket, ProtoPacket, and Flit classes.
 */

# ifndef _PACKET_DEFINES_H_
# define _PACKET_DEFINES_H_ 1

# include <iostream>

# include "global_defines/defines.h"

/**
 * @brief The type of the Packet
 * @note When `sync_protocol_enable` is false,
 *  traces from the trace file will be read as SPacket.
 */
class SPacket {
public:
	
    TId id;
    
    TimeType start_time;
	
    AddrType src_addr;
    AddrType des_addr;
	
    long packet_size;
	
	/**
     * @brief Construct a new SPacket object
     * @param addr_size the size of the address (the size of the vector)
     */
    SPacket(std::size_t addr_size);

    bool operator<(const SPacket& another) const;

    bool operator>(const SPacket& another) const;

};

/**
 * @brief The type of the Packet
 * @note When `sync_protocol_enable` is true,
 *  traces from the trace file will be read as ProtoPackets.
 */
class ProtoPacket {
public:
    
    TId id;

    TimeType src_time;
    TimeType des_time;
    
    AddrType src_addr;
    AddrType des_addr;
    
    long packet_size;
    long proto_dsc;
    
    /**
     * @brief Construct a new ProtoPacket object
     * @param addr_size the size of the address (the size of the vector)
     */
    ProtoPacket(std::size_t addr_size);

};

/**
 * @brief A data packet used for transmission across the network from sender to receiver.
 * @note The maximum size is preset, and if exceeded, the packet must be fragmented for transmission.
 */
class Flit {

private:
	
    TFlitId flit_id_;
	FlitType flit_type_;
	AddrType src_addr_;
	AddrType des_addr_;
	TimeType start_time_;
	TimeType send_finish_time_;
	TimeType finish_time_;
	DataType data_;
	TPacketId packet_id_;
	
public:
	
    /**
     * @brief Get the flit ID
     * @return the flit ID
     * @note The flit ID is used to identify the flit.
     * @note Noting that the flit ID is not the same as the packet ID.
     */
    TFlitId getFlitID() const;
	
    /**
     * @brief Get the flit type
     * @return the flit type
     */
    FlitType getFlitType() const;
	
    /**
     * @brief Get the source address
     * @return the source address
     */
    AddrType& getSrcAddr();
	
    /**
     * @brief Get the source address (const)
     * @return the source address (const)
     */
    const AddrType& getSrcAddr() const;
	
    /**
     * @brief Get the destination address
     * @return the destination address
     */
    AddrType& getDesAddr();
	
    /**
     * @brief Get the destination address (const)
     * @return the destination address (const)
     */
    const AddrType& getDesAddr() const ;

    /**
     * @brief Get the start time
     * @return the start time
     */
    TimeType getStartTime() const;

	/**
     * @brief Get the send finish time
     * @return the send finish time
     */
    TimeType getSendFinTime() const;

    /**
     * @brief Get the finish time
     * @return the finish time
     */
	TimeType getFinTime() const;

	/**
     * @brief Set the start time of the flit
     * @param new_start_time the start time
     */
    void setStartTime(TimeType new_start_time);
	
    /**
     * @brief Set the send finish time of the flit
     * @param new_send_finish_time the send finish time
     */
    void setSendFinTime(TimeType new_send_finish_time);
	
    /**
     * @brief Set the finish time of the flit
     * @param new_finish_time the finish time
     */
    void setFinTime(TimeType new_finish_time);

	/**
     * @brief Get the data
     * @return the data
     */
    DataType& getData();
	
    /**
     * @brief Get the data (const)
     * @return the data (const)
     */
    const DataType& getData() const;

	/**
     * @brief Get the packet ID
     * @return the packet ID
     * @note The packet ID is used to identify the packet to which the flit belongs.
     * @note Noting that the packet ID is not the same as the flit ID. 
     */
    TPacketId getPacketId() const;

	/**
     * @brief Construct a new Flit object
     * @note Every param is set with its default value.
     * @note The flit type is set to `FlitType::HEADER_`.
     */
    Flit();
	
    /**
     * @brief Construct a new Flit object with the specified parameters
     * @param flit_id the flit ID
     * @param flit_type the flit type
     * @param src_addr the source address
     * @param des_addr the destination address
     * @param start_time the start time
     * @param data the data
     * @param packet_id the packet ID
     * @note The finish time is set to `0`.
     * @note The send finish time is set to `0`.
     */
    Flit(long flit_id_, FlitType flit_type,
        const AddrType& sor_addr, const AddrType& des_addr,
		TimeType start_time, const DataType& data,
        TPacketId packet_id);
	
    /**
     * @brief Construct a new Flit object by copying another Flit object
     * @param flit the flit object
     */
    Flit(const Flit& flit);
    
};

/**
 * @brief operator<< overload for Flit
 */
std::ostream& operator<<(std::ostream& os, const Flit& flit);

/**
 * @brief A function to check if the flit is a header flit
 * @param ft the flit type
 * @return true if `FlitType::HEADER_` or `FlitType::SINGLE`, false otherwise
 */
bool isHeader(const FlitType ft);

/**
 * @brief A function to check if the flit is a tail flit
 * @param ft the flit type
 * @return true if the flit is `FlitType::TAIL_` or `FlitType::SINGLE`, false otherwise
 */
bool isTail(const FlitType ft);

# endif

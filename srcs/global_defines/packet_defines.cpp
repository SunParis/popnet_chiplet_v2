# include "global_defines/packet_defines.h"



/**
 * @brief Construct a new SPacket object
 * @param addr_size the size of the address (the size of the vector)
 */
SPacket::SPacket(std::size_t addr_size) {
	this->start_time = -1;
	this->src_addr.reserve(addr_size);
	this->des_addr.reserve(addr_size);
}

bool SPacket::operator<(const SPacket& another) const {
    return this->start_time < another.start_time;
}

bool SPacket::operator>(const SPacket& another) const {
    return this->start_time > another.start_time;
}
    
/**
 * @brief Construct a new ProtoPacket object
 * @param addr_size the size of the address (the size of the vector)
 */
ProtoPacket::ProtoPacket(std::size_t addr_size) {
    this->src_addr.reserve(addr_size);
    this->des_addr.reserve(addr_size);
}

/**
 * @brief Get the flit ID
 * @return the flit ID
 * @note The flit ID is used to identify the flit.
 * @note Noting that the flit ID is not the same as the packet ID.
 */
TFlitId Flit::getFlitID() const {
    return this->flit_id_;
}

/**
 * @brief Get the flit type
 * @return the flit type
 */
FlitType Flit::getFlitType() const {
    return this->flit_type_;
}

/**
 * @brief Get the source address
 * @return the source address
 */
AddrType& Flit::getSrcAddr(){
   return this->src_addr_;
}

/**
 * @brief Get the source address (const)
 * @return the source address (const)
 */
const AddrType& Flit::getSrcAddr() const{
   return this->src_addr_;
}

/**
 * @brief Get the destination address
 * @return the destination address
 */
AddrType& Flit::getDesAddr(){
   return this->des_addr_;
}

/**
 * @brief Get the destination address (const)
 * @return the destination address (const)
 */
const AddrType& Flit::getDesAddr() const {
    return this->des_addr_;
}

/**
 * @brief Get the start time
 * @return the start time
 */
TimeType Flit::getStartTime() const {
   return this->start_time_;
}

/**
 * @brief Get the send finish time
 * @return the send finish time
 */
TimeType Flit::getSendFinTime() const {
   return this->send_finish_time_;
}

/**
 * @brief Get the finish time
 * @return the finish time
 */
TimeType Flit::getFinTime() const {
   return this->finish_time_;
}

/**
 * @brief Set the start time of the flit
 * @param new_start_time the start time
 */
void Flit::setStartTime(TimeType new_start_time) {
   this->start_time_ = new_start_time;
}

/**
 * @brief Set the send finish time of the flit
 * @param new_send_finish_time the send finish time
 */
void Flit::setSendFinTime(TimeType new_send_finish_time) {
    this->send_finish_time_ = new_send_finish_time;
}

/**
 * @brief Set the finish time of the flit
 * @param new_finish_time the finish time
 */
void Flit::setFinTime(TimeType new_finish_time) {
    this->finish_time_ = new_finish_time;
}

/**
 * @brief Get the data
 * @return the data
 */
DataType& Flit::getData() {
    return this->data_;
}

/**
 * @brief Get the data (const)
 * @return the data (const)
 */
const DataType& Flit::getData() const {
    return this->data_;
}

/**
 * @brief Get the packet ID
 * @return the packet ID
 * @note The packet ID is used to identify the packet to which the flit belongs.
 * @note Noting that the packet ID is not the same as the flit ID. 
 */
TPacketId Flit::getPacketId() const {
	return this->packet_id_;
}

/**
 * @brief Construct a new Flit object
 * @note Every param is set with its default value.
 * @note The flit type is set to `FlitType::HEADER_`.
 */
Flit::Flit()
:   flit_id_(),
    flit_type_(FlitType::HEADER),
    src_addr_(),
    des_addr_(),
    start_time_(),
    send_finish_time_(),
    finish_time_(),
    data_(),
    packet_id_()
{}

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
Flit::Flit(long flit_id, FlitType flit_type,
    const AddrType& src_addr, const AddrType& des_addr,
	TimeType start_time, const DataType& data,
    TPacketId packet_id)
:   flit_id_(flit_id),
    flit_type_(flit_type),
    src_addr_(src_addr),
    des_addr_(des_addr),
    start_time_(start_time),
    send_finish_time_(),
    finish_time_(),
    data_(data),
    packet_id_(packet_id)
{}

/**
 * @brief Construct a new Flit object by copying another Flit object
 * @param flit the flit object
 */
Flit::Flit(const Flit& flit)
:   flit_id_(flit.getFlitID()),
    flit_type_(flit.getFlitType()),
    src_addr_(flit.getSrcAddr()),
    des_addr_(flit.getDesAddr()),
    start_time_(flit.getStartTime()),
    send_finish_time_(flit.getSendFinTime()),
    finish_time_(flit.getFinTime()),
    data_(flit.getData()),
    packet_id_(flit.getPacketId())
{}

/**
 * @brief operator<< overload for Flit
*/
std::ostream& operator<<(std::ostream& os, const Flit& flit) {
    os << flit.getFlitID() << ":: ";
    os << "from " << flit.getSrcAddr() << " to " << flit.getSrcAddr() << " (" << flit.getFlitType() << ")";
    return os;
}

/**
 * @brief A function to check if the flit is a header flit
 * @param ft the flit type
 * @return true if `FlitType::HEADER_` or `FlitType::SINGLE`, false otherwise
 */
bool isHeader(const FlitType ft) {
	return (ft == FlitType::HEADER || ft == FlitType::SINGLE);
}

/**
 * @brief A function to check if the flit is a tail flit
 * @param ft the flit type
 * @return true if the flit is `FlitType::TAIL_` or `FlitType::SINGLE`, false otherwise
 */
bool isTail(const FlitType ft) {
    return (ft == FlitType::TAIL || ft == FlitType::SINGLE);
}


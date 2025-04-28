
# include "global_defines/input_trace.h"


FileSizeType InputTrace::getFileSize(const std::string& trace_file_name) {
    return std::filesystem::file_size(trace_file_name);
}

void InputTrace::readAddress(AddrType& address, std::ifstream& trace_file) {
    long t;
    address.clear();
    for (std::size_t i = 0; i < this->dimension_; i++) {
        trace_file >> t;
        address.push_back(t);
    }
}

InputTrace::InputTrace(const std::string& trace_file_name, bool sync_protocol_enable, std::size_t dimension)
:   trace_file_name_(trace_file_name),
    sync_protocol_enable_(sync_protocol_enable),
    has_read_(0),
    dimension_(dimension),
    input_traces_(),
    router_traces_()
{}

void InputTrace::readTraceFile() {
    std::ifstream trace_file(this->trace_file_name_);
    if (!trace_file.is_open()) {
        throw std::runtime_error(std::string("Failed to open trace file: ") + this->trace_file_name_);
    }
    if (this->has_read_ == this->getFileSize(this->trace_file_name_)) {
        return;
    }
    trace_file.seekg(this->has_read_, std::ios::beg);
    std::size_t count = 0;
    
    if (!this->sync_protocol_enable_) {
        SPacket spacket(this->dimension_);
        while (trace_file >> spacket.start_time) {
            this->readAddress(spacket.src_addr, trace_file);
            this->readAddress(spacket.des_addr, trace_file);
            trace_file >> spacket.packet_size;
            spacket.id = count;
            
            this->addTrace(spacket);
            
            count++;
        }
    }
    else {
        ProtoPacket packet(this->dimension_);
        while (trace_file >> packet.src_time >> packet.des_time) {
            this->readAddress(packet.src_addr, trace_file);
            this->readAddress(packet.des_addr, trace_file);
            trace_file >> packet.packet_size >> packet.proto_dsc;
            packet.id = count;
            SPacket spacket = ___add_trans___(packet);
            
            this->addTrace(spacket);
            
            count++;
        }
    }
    Logger::info("Read packets: {}", count);
    this->has_read_ = this->getFileSize(this->trace_file_name_);
}

void InputTrace::addTrace(const SPacket& packet) {
    this->input_traces_.push(packet);    
    if (this->router_traces_.find(packet.src_addr) == this->router_traces_.end()) {
        this->router_traces_[packet.src_addr] = std::priority_queue<SPacket, std::vector<SPacket>, std::greater<SPacket>>();        
    }
    this->router_traces_[packet.src_addr].push(packet);
}

bool InputTrace::isEmpty() {
    this->readTraceFile();
    return this->input_traces_.empty();
}

bool InputTrace::isEmpty(const AddrType& address) {
    this->readTraceFile();
    if (this->router_traces_.find(address) == this->router_traces_.end()) {
        return true;
    }
    return this->router_traces_[address].empty();
}

void InputTrace::popFront() {
    this->input_traces_.pop();
}

void InputTrace::popFront(const AddrType& address) {
    if (this->router_traces_.find(address) == this->router_traces_.end()) {
        return;
    }
    this->router_traces_[address].pop();
}

const SPacket& InputTrace::front() const {
    return this->input_traces_.top();
}

const SPacket& InputTrace::front(const AddrType& address) const {
    auto it = this->router_traces_.find(address);
    if (it == this->router_traces_.end()) {
        throw std::runtime_error("Address not found in router traces");
    }
    return it->second.top();
}


# pragma once

# ifndef _INPUT_TRACE_H_
# define _INPUT_TRACE_H_ 1

# include <filesystem>
# include <fstream>
# include <queue>
# include <map>

# include "global_defines/packet_defines.h"
# include "global_defines/proto_engine.h"
# include "global_defines/defines.h"
# include "logger/logger.hpp"

using FileSizeType = decltype(std::filesystem::file_size(std::declval<const std::filesystem::path&>()));

class InputTrace {

private:
    
    std::string trace_file_name_;

    FileSizeType has_read_;

    bool sync_protocol_enable_;

    std::map<AddrType, std::priority_queue<SPacket, std::vector<SPacket>, std::greater<SPacket>>> router_traces_;
    std::priority_queue<SPacket, std::vector<SPacket>, std::greater<SPacket>> input_traces_;

    std::size_t dimension_;

    FileSizeType getFileSize(const std::string& trace_file_name);

    void readAddress(AddrType& address, std::ifstream& trace_file);

public:

    InputTrace(const std::string& trace_file_name, bool sync_protocol_enable, std::size_t dimension);

    void readTraceFile();

    void addTrace(const SPacket& packet);

    bool isEmpty();

    bool isEmpty(const AddrType& address);

    void popFront();

    void popFront(const AddrType& address);

    const SPacket& front() const;

    const SPacket& front(const AddrType& address) const;

};

# endif
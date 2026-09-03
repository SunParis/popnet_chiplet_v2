#pragma once

#include <fstream>
#include <string>

#include "global_defines/packet_defines.h"
#include "global_defines/proto_engine.h"

class DelayWriter {
  public:
    explicit DelayWriter(const std::string& path);

    DelayWriter(const DelayWriter&) = delete;
    DelayWriter& operator=(const DelayWriter&) = delete;

    void writePacket(const Flit& flit, TimeType delay);
    void writeProtocolInjection(const ProtoStateMachine& transaction, TimeType delay);
    void writeProtocolCompletion(const ProtoStateMachine& transaction);
    void flush();

  private:
    void writeTransactionPrefix(const ProtoStateMachine& transaction);

    std::ofstream output_;
};

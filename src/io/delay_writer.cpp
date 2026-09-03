#include "io/delay_writer.h"

#include <stdexcept>

DelayWriter::DelayWriter(const std::string& path) : output_(path, std::ios::app) {
    if (!output_.is_open()) {
        throw std::runtime_error("Cannot open delay output file: " + path);
    }
}

void DelayWriter::writePacket(const Flit& flit, TimeType delay) {
    output_ << static_cast<long>(flit.getStartTime()) << ' ';
    for (const auto coordinate : flit.getSrcAddr()) {
        output_ << coordinate << ' ';
    }
    for (const auto coordinate : flit.getDesAddr()) {
        output_ << coordinate << ' ';
    }
    output_ << delay << '\n';
}

void DelayWriter::writeProtocolInjection(const ProtoStateMachine& transaction, TimeType delay) {
    writeTransactionPrefix(transaction);
    output_ << transaction.protoDesc << " 2 " << delay << " -1\n";
}

void DelayWriter::writeProtocolCompletion(const ProtoStateMachine& transaction) {
    writeTransactionPrefix(transaction);
    output_ << transaction.protoDesc << ' ' << transaction.packetDelay.size();
    for (const auto delay : transaction.packetDelay) {
        output_ << ' ' << static_cast<long>(delay);
    }
    output_ << '\n';
}

void DelayWriter::flush() {
    output_.flush();
}

void DelayWriter::writeTransactionPrefix(const ProtoStateMachine& transaction) {
    output_ << static_cast<long>(transaction.src_time) << ' ';
    for (const auto coordinate : transaction.src_addr) {
        output_ << coordinate << ' ';
    }
    for (const auto coordinate : transaction.des_addr) {
        output_ << coordinate << ' ';
    }
}

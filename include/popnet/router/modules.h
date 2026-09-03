#pragma once

#include <cstddef>
#include <deque>
#include <memory>
#include <vector>

#include "global.h"

class InputModules {
  public:
    InputModules(long physical_ports, long virtual_channels);

    bool isIBuffFull() const noexcept;
    void setIBuffFull(bool full) noexcept;
    std::size_t getBufferSize(long port, long vc) const;
    void setState(long port, long vc, VCStateType state);
    VCStateType getState(long port, long vc) const;
    void addFlit(long port, long vc, const Flit& flit);
    void addFlit(long port, long vc, Flit&& flit);
    Flit& getFlit(long port, long vc);
    const Flit& getFlit(long port, long vc) const;
    void removeFlit(long port, long vc);
    std::vector<VCType>& getRouting(long port, long vc);
    const std::vector<VCType>& getRouting(long port, long vc) const;
    void addRouting(long port, long vc, VCType route);
    void clearRouting(long port, long vc);
    void setCRouting(long port, long vc, VCType route);
    VCType getCRouting(long port, long vc) const;

  private:
    std::size_t index(long port, long vc) const;

    long physical_ports_;
    long virtual_channels_;
    std::vector<std::deque<Flit>> input_;
    std::vector<VCStateType> states_;
    std::vector<std::vector<VCType>> routing_;
    std::vector<VCType> selected_routing_;
    bool injection_buffer_full_{false};
};

class OutputModules {
  public:
    OutputModules(long physical_ports, long virtual_channels, long input_buffer_size,
                  long output_buffer_size);

    void incCounter(long port, long vc);
    void decCounter(long port, long vc);
    long getCounter(long port, long vc) const;
    long getLocalCounter(long port) const;
    VCUsageType getUsage(long port, long vc) const;
    void acquireChannel(long port, long vc, VCType input);
    void releaseChannel(long port, long vc);
    std::size_t getOutBufferSize(long port) const;
    void addFlit(long port, const Flit& flit);
    void addFlit(long port, Flit&& flit);
    Flit& getFlit(long port);
    const Flit& getFlit(long port) const;
    void removeFlit(long port);
    void addAddr(long port, const VCType& address);
    VCType getAddr(long port) const;
    void removeAddr(long port);

  private:
    std::size_t channelIndex(long port, long vc) const;
    std::size_t portIndex(long port) const;

    long physical_ports_;
    long virtual_channels_;
    std::vector<long> credits_;
    std::vector<VCUsageType> usage_;
    std::vector<std::deque<Flit>> output_buffers_;
    std::vector<std::deque<VCType>> output_addresses_;
    std::vector<long> local_credits_;
};

class PowerModules {
  public:
    PowerModules(long physical_ports, long virtual_channels, long flit_size, double link_length);
    ~PowerModules();

    PowerModules(const PowerModules&) = delete;
    PowerModules& operator=(const PowerModules&) = delete;

    void addBufferReadPwr(long input_port, const DataType& data);
    void addBufferWritePwr(long input_port, const DataType& data);
    void addCrossbarTravPwr(long input_port, long output_port, const DataType& data);
    void addVCArbitPwr(long port, long vc, AtomType request, unsigned long grant);
    void addLinkTravPwr(long input_port, const DataType& data);

    double getBufferPower();
    double getLinkPower();
    double getCrossbarPower();
    double getArbiterPower();

  private:
    struct OrionState;

    long flit_size_;
    std::unique_ptr<OrionState> orion_;
    std::vector<DataType> buffer_write_;
    std::vector<DataType> buffer_read_;
    std::vector<DataType> crossbar_read_;
    std::vector<DataType> crossbar_write_;
    std::vector<DataType> link_traversal_;
    std::vector<long> crossbar_input_;
    std::vector<std::vector<AtomType>> arbiter_vc_req_;
    std::vector<std::vector<unsigned long>> arbiter_vc_grant_;
};

#pragma once

#include <cstddef>
#include <memory>

class Config;
class SimulationState;

struct SimulationResults {
    std::size_t total_finished{0};
    double average_delay{0};
    double memory_power{0};
    double crossbar_power{0};
    double arbiter_power{0};
    double link_power{0};
    double total_power{0};
};

class Sim {
  public:
    explicit Sim(const Config& config);
    ~Sim();

    Sim(const Sim&) = delete;
    Sim& operator=(const Sim&) = delete;

    void mainProcess();
    SimulationResults results();
    void getResults();

    const SimulationState& state() const noexcept;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

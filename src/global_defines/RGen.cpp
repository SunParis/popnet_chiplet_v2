#include "global_defines/RGen.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

namespace {

std::mt19937_64 makeEngine() {
    std::random_device source;
    std::seed_seq seed{
        source(), source(), source(), source(), source(), source(), source(), source(),
    };
    return std::mt19937_64(seed);
}

} // namespace

RGen::RGen() : engine_(makeEngine()) {}

RGen::RGen(long seed) : engine_(makeEngine()), seed_(seed) {
    initialize_seeded_engine(seed);
}

void RGen::reset_seed(long seed) {
    seed_ = seed;
    initialize_seeded_engine(seed);
}

void RGen::reset_seed() {
    seed_.reset();
    engine_ = makeEngine();
}

double RGen::random_double_01() {
    if (seed_.has_value()) {
        std::int32_t sample = 0;
        if (random_r(&seeded_engine_, &sample) != 0) {
            throw std::runtime_error("Could not generate a seeded random value");
        }
        const auto value = static_cast<double>(sample) / static_cast<double>(RAND_MAX);
        return value < 1.0 ? value : std::nextafter(1.0, 0.0);
    }
    return std::generate_canonical<double, std::numeric_limits<double>::digits>(engine_);
}

void RGen::initialize_seeded_engine(long seed) {
    seeded_state_.fill(0);
    seeded_engine_ = {};
    if (initstate_r(static_cast<unsigned int>(seed), seeded_state_.data(), seeded_state_.size(),
                    &seeded_engine_) != 0) {
        throw std::runtime_error("Could not initialize the seeded random generator");
    }
}

double RGen::random_double(double low, double high) {
    assert(low < high);
    return (high - low) * random_double_01() + low;
}

long RGen::random_long(long low, long high) {
    assert(low < high);
    const auto result = static_cast<long>(static_cast<double>(high - low) * random_double_01() +
                                          static_cast<double>(low));
    return std::min(result, high - 1);
}

unsigned long RGen::random_u_long(unsigned long low, unsigned long high) {
    assert(low < high);
    const auto result = static_cast<unsigned long>(
        static_cast<double>(high - low) * random_double_01() + static_cast<double>(low));
    return std::min(result, high - 1);
}

unsigned long long RGen::random_u_long_long(unsigned long long low, unsigned long long high) {
    assert(low < high);
    const long double scaled = static_cast<long double>(high - low) * random_double_01();
    const auto result = static_cast<unsigned long long>(scaled) + low;
    return std::min(result, high - 1);
}

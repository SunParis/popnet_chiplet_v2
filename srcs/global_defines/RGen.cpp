# include "global_defines/RGen.h"

RGen::RGen(long seed) {
    this->seed_ = std::make_unique<long>(seed);
    srandom(seed);
}

/**
 * @brief Reset the seed of the random number generator.
 */
void RGen::reset_seed(long seed) {
    this->seed_ = std::make_unique<long>(seed);
    srandom(seed);
}

/**
 * @brief Reset the seed of the random number generator.
 */
void RGen::reset_seed() {
    new (&this->rd) std::random_device();
}

/**
 * @brief Get a random double number in [0, 1).
 * @return Random double number in [0, 1).
 */
double RGen::random_double_01() {
    if (this->seed_) {
        return random() * 1.0 / RAND_MAX;
    }
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    return dis(this->rd);
}

/**
 * @brief Get a random double number in [low, high).
 * @param low Lower bound.
 * @param high Upper bound.
 * @return Random double number in [low, high).
 */
double RGen::random_double(double low, double high) {
    assert(low < high);
    return (high - low) * this->random_double_01() + low;
}

/**
 * @brief Get a random long number in [low, high).
 * @param low Lower bound.
 * @param high Upper bound.
 * @return Random long number in [low, high).
 */
long RGen::random_long(long low, long high) {
    assert(low < high);
    return (long) ((high - low) * this->random_double_01() + low);
}

/**
 * @brief Get a random unsigned long number in [low, high).
 * @param low Lower bound.
 * @param high Upper bound.
 * @return Random unsigned long number in [low, high).
 */
unsigned long RGen::random_u_long(unsigned long low, unsigned long high) {
    assert(low < high);
    return (unsigned long) ((high - low) * this->random_double_01() + low);
}

/**
 * @brief Get a random unsigned long long number in [low, high).
 * @param low Lower bound.
 * @param high Upper bound.
 * @return Random unsigned long long number in [low, high).
 */
unsigned long long RGen::random_u_long_long(unsigned long long low, unsigned long long high) {
    assert(low < high);
    return (unsigned long long) ((high - low) * this->random_double_01() + low);
}


# pragma once

/**
 * @file RGen.h
 * @brief Random number generator.
 */

# ifndef _RGEN_H_
# define _RGEM_H_ 1

# include <random>
# include <cassert>
# include <iostream>
# include <memory>

# include "global_defines/defines.h"

/**
 * @brief Random number generator.
 */
class RGen {

private:
    
    std::random_device rd;

    std::unique_ptr<long> seed_;

public:
    
    /**
     * @brief Construct a new RGen object.
     */
    RGen() = default;

    /**
     * @brief Construct a new RGen object with a seed.
     */
    RGen(long seed);

    /**
     * @brief Destroy the RGen object.
     */
    ~RGen() = default;

    /**
     * @brief Reset the seed of the random number generator.
     */
    void reset_seed(long seed);

    /**
     * @brief Reset the seed of the random number generator.
     */
    void reset_seed();

    /**
     * @brief Get a random double number in [0, 1).
     * @return Random double number in [0, 1).
     */
    double random_double_01();

    /**
     * @brief Get a random double number in [low, high).
     * @param low Lower bound.
     * @param high Upper bound.
     * @return Random double number in [low, high).
     */
    double random_double(double low, double high);

    /**
     * @brief Get a random long number in [low, high).
     * @param low Lower bound.
     * @param high Upper bound.
     * @return Random long number in [low, high).
     */
    long random_long(long low, long high);
    
    /**
     * @brief Get a random unsigned long number in [low, high).
     * @param low Lower bound.
     * @param high Upper bound.
     * @return Random unsigned long number in [low, high).
     */
    unsigned long random_u_long(unsigned long low, unsigned long high);

    /**
     * @brief Get a random unsigned long long number in [low, high).
     * @param low Lower bound.
     * @param high Upper bound.
     * @return Random unsigned long long number in [low, high).
     */
    unsigned long long random_u_long_long(unsigned long long low, unsigned long long high);

};

# endif
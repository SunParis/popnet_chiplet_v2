# pragma once

/**
 * @file SStd.h
 * @brief This file contains the definition of the SStd namespace.
 */

# ifndef _NETWORK_STD_H_
# define _NETWORK_STD_H_ 1

# include <iostream>
# include "logger/logger.hpp"

# define Sassert(x, y) SStd::sassert(x, __FILE__, __LINE__, y)

namespace SStd {

    /**
     * @brief Abort, printing position in code, if (! x).
     * @param x Condition to check.
     * @param file File name.
     * @param line Line number.
     * @param message Message to print. Default is "Assertion failed.  Aborting.".
     */
    void sassert(bool x, const char *file, int line, const char *message = nullptr);

};

# endif

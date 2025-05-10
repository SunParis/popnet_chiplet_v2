# pragma once

/**
 * @file logger.hpp
 * @brief A very very very simple logger based on fmt library.
 */

# ifndef _LOGGER_HPP_
# define _LOGGER_HPP_ 1

# define _LOGGER_OUT_DEFAULT_ std::string("stdout")
# define _GREEN_ fg(fmt::rgb(165, 210, 165))
# define _ORANGE_ fg(fmt::rgb(230, 165, 105))
# define _RED_ fg(fmt::rgb(250, 110, 110))
# define THREAD_SAFE // Comment this line to `disable` thread safety

# include <string>
# include <iostream>
# include <fstream>
# include <chrono>
# include <ctime>
# include <iomanip>
# include <sstream>

# define FMT_HEADER_ONLY
# include "fmt/core.h"
# include "fmt/color.h"

# ifdef THREAD_SAFE

# include <mutex>
# include <atomic>
# include <linux/futex.h>
# include <sys/syscall.h>
# include <unistd.h>

namespace Logger {

static int futex_wait(std::atomic_bool* addr, int expected) {
    return syscall(SYS_futex, addr, FUTEX_WAIT, expected, nullptr, nullptr, 0);
}

static int futex_wake(std::atomic_bool* addr, int count) {
    return syscall(SYS_futex, addr, FUTEX_WAKE, count, nullptr, nullptr, 0);
}

/**
 * @brief A futex lock implementation.
 * @note This class uses atomic operations and futex system calls to implement a lock. 
 */
class futex {
    
    /**
     * @brief The state of the futex lock.
     * @note This is an atomic boolean that indicates whether the lock is held (true) or not (false).
     */
    std::atomic_bool state{false};

public:

    /**
     * @brief Default constructor.
     */
    futex() = default;
    
    /**
     * @brief Deleted copy constructor.
     */
    futex(const futex&) = delete;

    /**
     * @brief Default destructor.
     */
    ~futex() = default;
    
    /**
     * @brief Deleted copy assignment operator.
     */
    futex& operator=(const futex&) = delete;
    
    /**
     * @brief Acquire the futex lock.
     * @note This function will block until the lock is acquired.
     */
    void lock() {
        bool expected = false;
        // Try to acquire the lock
        while (!this->state.compare_exchange_weak(expected, true, std::memory_order_acquire)) {
            // The lock is already held, call futex to wait
            futex_wait(&this->state, true);
            expected = false;
        }
    }

    /**
     * @brief Release the futex lock.
     * @note This function will wake one waiting thread.
     */
    void unlock() {
        this->state.store(false, std::memory_order_release);
        // Wake one waiting thread
        futex_wake(&this->state, true);
    }

    /**
     * @brief Try to acquire the futex lock without blocking.
     * @return true if the lock was acquired, false otherwise.
     */
    bool try_lock() {
        bool expected = false;
        // Try to acquire the lock
        return this->state.compare_exchange_strong(expected, true, std::memory_order_acquire);
    }

};

};

# endif

enum class LogLevel: unsigned char {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3
};

namespace Logger {

    /**
     * @brief Get the current time in the format YYYY-MM-DD HH:MM:SS
     * @return The current time as a std::string
     */
    inline std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm localTime{};
        localtime_r(&time_t_now, &localTime);

        std::ostringstream timeStr;
        timeStr << std::put_time(&localTime, "%F %T");
        return timeStr.str();
    }

    inline LogLevel CURR_LOGLEVEL = LogLevel::Info;
    inline std::string LOGGER_OUT = _LOGGER_OUT_DEFAULT_;

# ifdef THREAD_SAFE

    inline Logger::futex LOGGER_LOCK;
    inline Logger::futex LOGLEVEL_LOCK;
    inline Logger::futex LOGGER_OUT_LOCK;

# endif


    /**
     * @brief Set the log level
     * @param level The log level
     */
    inline void setLogLevel(LogLevel level) {
# ifdef THREAD_SAFE
        const std::lock_guard<Logger::futex> lock(Logger::LOGLEVEL_LOCK);
# endif
        Logger::CURR_LOGLEVEL = level;
    }

    /**
     * @brief Get the log level
     * @return The log level
     */
    inline LogLevel getLogLevel() {
# ifdef THREAD_SAFE
        const std::lock_guard<Logger::futex> lock(Logger::LOGLEVEL_LOCK);
# endif
        return Logger::CURR_LOGLEVEL;
    }

    /**
     * @brief Set the logger output
     * @param out The logger output
     * @note If the output is a file, the file will be opened in append mode
     * @note The default output is std::cout
     */
    inline void setLoggerOut(const std::string& out) {
# ifdef THREAD_SAFE
        const std::lock_guard<Logger::futex> lock(Logger::LOGGER_OUT_LOCK);
# endif
        Logger::LOGGER_OUT = out;
    }

    /**
     * @brief Get the logger output
     * @return The logger output
     */
    inline std::string getLoggerOut() {
# ifdef THREAD_SAFE
        const std::lock_guard<Logger::futex> lock(Logger::LOGGER_OUT_LOCK);
# endif
        return Logger::LOGGER_OUT;
    }

    /**
     * @brief Log a message
     * @param level The log level
     * @param fmtstr The format string
     * @param args The arguments
     */
    template<typename... Args>
    inline void log(LogLevel level, fmt::format_string<Args...> fmtstr, Args&&... args) {        
        std::string timeStr = Logger::getCurrentTime();

        if (Logger::getLoggerOut() == "std::cout" || Logger::getLoggerOut() == "stdout") {
# ifdef THREAD_SAFE
            const std::lock_guard<Logger::futex> lock(Logger::LOGGER_LOCK);
# endif
            fmt::print("[{}][", timeStr);
            switch (level) {
                case LogLevel::Info: 
                    fmt::print(_GREEN_, "info ");
                    break;
                case LogLevel::Debug: 
                    fmt::print(_GREEN_, "debug");
                    break;
                case LogLevel::Warn:
                    fmt::print(_ORANGE_, "warn ");
                    break;
                case LogLevel::Error:
                    fmt::print(_RED_, "error");
                    break;
            }

            fmt::print("\033[0m");
            fmt::print("] ");
            fmt::print(fmtstr, std::forward<Args>(args)...);
            fmt::print("\n");
        }
        else {
            std::ofstream logFile(Logger::getLoggerOut(), std::ios::app);
# ifdef THREAD_SAFE
            const std::lock_guard<Logger::futex> lock(Logger::LOGGER_LOCK);
# endif
            if (!logFile.is_open()) {
                fmt::print("[{}][", timeStr);
                fmt::print(_RED_, "error");
                fmt::print("\033[0m");
                fmt::print("] ");
                fmt::print("Could not open log file: {}\n", Logger::getLoggerOut());
                return;
            }            
            std::string out_str = "";
            switch (level) {
                case LogLevel::Info: 
                    out_str += fmt::format("[{}][info ] ", timeStr);
                    break;
                case LogLevel::Debug: 
                    out_str += fmt::format("[{}][debug] ", timeStr);
                    break;
                case LogLevel::Warn:
                    out_str += fmt::format("[{}][warn ] ", timeStr);
                    break;
                case LogLevel::Error:
                    out_str += fmt::format("[{}][error] ", timeStr);
                    break;
            }
            out_str += fmt::format(fmtstr, std::forward<Args>(args)...);
            out_str += "\n";
            logFile << out_str;
            logFile.close();
        }
    }
    
    /**
     * @brief Log a message with the debug level
     */
    template<typename... Args>
    inline void debug(fmt::format_string<Args...> fmtstr, Args&&... args) {
        if (Logger::getLogLevel() > LogLevel::Debug) {
            return;
        }
        Logger::log(LogLevel::Debug, fmtstr, std::forward<Args>(args)...);
    }

    /**
     * @brief Log a message with the info level
     */
    template<typename... Args>
    inline void info(fmt::format_string<Args...> fmtstr, Args&&... args) {
        if (Logger::getLogLevel() > LogLevel::Info) {
            return;
        }
        Logger::log(LogLevel::Info, fmtstr, std::forward<Args>(args)...);
    }

    /**
     * @brief Log a message with the warning level
     */
    template<typename... Args>
    inline void warn(fmt::format_string<Args...> fmtstr, Args&&... args) {
        if (Logger::getLogLevel() > LogLevel::Warn) {
            return;
        }
        Logger::log(LogLevel::Warn, fmtstr, std::forward<Args>(args)...);
    }

    /**
     * @brief Log a message with the error level
     */
    template<typename... Args>
    inline void error(fmt::format_string<Args...> fmtstr, Args&&... args) {
        if (Logger::getLogLevel() > LogLevel::Error) {
            return;
        }
        Logger::log(LogLevel::Error, fmtstr, std::forward<Args>(args)...);
    }

    /**
     * @brief Convert a stream to a string
     * @param args The stream
     * @return The string
     * @note Be careful with this function, it can be slow
     */
    template<typename T>
    inline std::string stream_to_string(const T& args) {
        std::ostringstream ss;
        ss << args;
        return ss.str();
    }
    
};

# endif

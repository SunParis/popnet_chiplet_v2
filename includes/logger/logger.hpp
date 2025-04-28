# pragma once

/**
 * @file logger.hpp
 * @brief A very very very simple logger based on fmt library.
 */

# ifndef _LOGGER_HPP_
# define _LOGGER_HPP_ 1

# define LOGGER_OUT_DEFAULT std::string("stdout")

# include <string>
# include <iostream>
# include <fstream>
# include <chrono>
# include <ctime>
# include <iomanip>
# include <sstream>
# include <any>

# include "global_defines/defines.h"

# include "fmt/core.h"
# include "fmt/color.h"

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
    inline std::string LOGGER_OUT = LOGGER_OUT_DEFAULT;

    /**
     * @brief Set the log level
     * @param level The log level
     */
    inline void setLogLevel(LogLevel level) {
        Logger::CURR_LOGLEVEL = level;
    }

    /**
     * @brief Get the log level
     * @return The log level
     */
    inline LogLevel getLogLevel() {
        return Logger::CURR_LOGLEVEL;
    }

    /**
     * @brief Set the logger output
     * @param out The logger output
     * @note If the output is a file, the file will be opened in append mode
     * @note The default output is std::cout
     */
    inline void setLoggerOut(const std::string& out) {
        Logger::LOGGER_OUT = out;
    }

    /**
     * @brief Get the logger output
     * @return The logger output
     */
    inline std::string getLoggerOut() {
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
        
        if (Logger::LOGGER_OUT == "std::cout" || Logger::LOGGER_OUT == "stdout") {
            
            switch (level) {
                case LogLevel::Info: 
                    fmt::print(fmt::fg(fmt::color::green), "[{}][info ] ", timeStr);
                    break;
                case LogLevel::Debug: 
                    fmt::print(fmt::fg(fmt::color::green), "[{}][debug] ", timeStr);
                    break;
                case LogLevel::Warn:
                    fmt::print(fmt::fg(fmt::color::orange), "[{}][warn ] ", timeStr);
                    break;
                case LogLevel::Error:
                    fmt::print(fmt::fg(fmt::color::red), "[{}][error] ", timeStr);
                    break;
            }

            fmt::print("\033[0m");
            fmt::print(fmtstr, std::forward<Args>(args)...);
            fmt::print("\n");
        }
        else {
            std::ofstream logFile(Logger::LOGGER_OUT, std::ios::app);
            if (!logFile.is_open()) {
                fmt::print(fmt::fg(fmt::color::red), "[{}][error] ", timeStr);
                fmt::print("\033[0m");
                fmt::print("Could not open log file: {}\n", Logger::LOGGER_OUT);
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
        if (Logger::CURR_LOGLEVEL > LogLevel::Debug) {
            return;
        }
        Logger::log(LogLevel::Debug, fmtstr, std::forward<Args>(args)...);
    }

    /**
     * @brief Log a message with the info level
     */
    template<typename... Args>
    inline void info(fmt::format_string<Args...> fmtstr, Args&&... args) {
        if (Logger::CURR_LOGLEVEL > LogLevel::Info) {
            return;
        }
        Logger::log(LogLevel::Info, fmtstr, std::forward<Args>(args)...);
    }

    /**
     * @brief Log a message with the warning level
     */
    template<typename... Args>
    inline void warn(fmt::format_string<Args...> fmtstr, Args&&... args) {
        if (Logger::CURR_LOGLEVEL > LogLevel::Warn) {
            return;
        }
        Logger::log(LogLevel::Warn, fmtstr, std::forward<Args>(args)...);
    }

    /**
     * @brief Log a message with the error level
     */
    template<typename... Args>
    inline void error(fmt::format_string<Args...> fmtstr, Args&&... args) {
        if (Logger::CURR_LOGLEVEL > LogLevel::Error) {
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
        std::stringstream ss;
        ss << args;
        return ss.str();
    }
    
};

# endif
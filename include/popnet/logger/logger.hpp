#pragma once

#include <array>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ostream.h>

enum class LogLevel : unsigned char {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3,
};

namespace Logger {

inline constexpr const char* stdout_sink = "stdout";

inline std::string getCurrentTime() {
    const auto now = std::chrono::system_clock::now();
    const auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time{};
    localtime_r(&now_time, &local_time);
    std::array<char, 20> text{};
    std::strftime(text.data(), text.size(), "%F %T", &local_time);
    return text.data();
}

namespace detail {

struct LoggerState {
    std::mutex mutex;
    LogLevel level{LogLevel::Info};
    std::string output{stdout_sink};
    std::ofstream file;
    std::time_t cached_second{0};
    std::string cached_timestamp;
};

inline LoggerState& state() {
    static LoggerState value;
    return value;
}

inline const char* levelText(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::Debug:
        return "debug";
    case LogLevel::Info:
        return "info ";
    case LogLevel::Warn:
        return "warn ";
    case LogLevel::Error:
        return "error";
    }
    return "error";
}

inline bool isStdout(const std::string& output) noexcept {
    return output == "stdout" || output == "std::cout";
}

inline const std::string& timestamp(LoggerState& state) {
    const auto now = std::chrono::system_clock::now();
    const auto second = std::chrono::system_clock::to_time_t(now);
    if (state.cached_timestamp.empty() || state.cached_second != second) {
        state.cached_second = second;
        std::tm local_time{};
        localtime_r(&second, &local_time);
        std::array<char, 20> text{};
        std::strftime(text.data(), text.size(), "%F %T", &local_time);
        state.cached_timestamp = text.data();
    }
    return state.cached_timestamp;
}

} // namespace detail

inline void setLogLevel(LogLevel level) {
    auto& state = detail::state();
    const std::lock_guard lock(state.mutex);
    state.level = level;
}

inline LogLevel getLogLevel() {
    auto& state = detail::state();
    const std::lock_guard lock(state.mutex);
    return state.level;
}

inline void setLoggerOut(const std::string& output) {
    auto& state = detail::state();
    const std::lock_guard lock(state.mutex);

    if (state.file.is_open()) {
        state.file.flush();
        state.file.close();
    }

    state.output = output;
    if (!detail::isStdout(output)) {
        state.file.open(output, std::ios::app);
        if (!state.file.is_open()) {
            state.output = stdout_sink;
            throw std::runtime_error("Could not open log file: " + output);
        }
    }
}

inline std::string getLoggerOut() {
    auto& state = detail::state();
    const std::lock_guard lock(state.mutex);
    return state.output;
}

inline void flush() {
    auto& state = detail::state();
    const std::lock_guard lock(state.mutex);
    if (state.file.is_open()) {
        state.file.flush();
    } else {
        std::cout.flush();
    }
}

template <typename... Args>
inline void log(LogLevel level, fmt::format_string<Args...> format, Args&&... args) {
    auto& state = detail::state();
    const std::lock_guard lock(state.mutex);
    if (level < state.level) {
        return;
    }

    const auto& timestamp = detail::timestamp(state);
    if (detail::isStdout(state.output)) {
        fmt::print("[{}][", timestamp);
        switch (level) {
        case LogLevel::Debug:
        case LogLevel::Info:
            fmt::print(fmt::fg(fmt::rgb(165, 210, 165)), "{}", detail::levelText(level));
            break;
        case LogLevel::Warn:
            fmt::print(fmt::fg(fmt::rgb(230, 165, 105)), "{}", detail::levelText(level));
            break;
        case LogLevel::Error:
            fmt::print(fmt::fg(fmt::rgb(250, 110, 110)), "{}", detail::levelText(level));
            break;
        }
        fmt::print("\033[0m] ");
        fmt::print(format, std::forward<Args>(args)...);
        fmt::print("\n");
        return;
    }

    state.file << '[' << timestamp << "][" << detail::levelText(level) << "] ";
    fmt::print(state.file, format, std::forward<Args>(args)...);
    state.file << '\n';
    if (level >= LogLevel::Error) {
        state.file.flush();
    }
}

template <typename... Args> inline void debug(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::Debug, format, std::forward<Args>(args)...);
}

template <typename... Args> inline void info(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::Info, format, std::forward<Args>(args)...);
}

template <typename... Args> inline void warn(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::Warn, format, std::forward<Args>(args)...);
}

template <typename... Args> inline void error(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::Error, format, std::forward<Args>(args)...);
}

template <typename T> inline std::string stream_to_string(const T& value) {
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

} // namespace Logger

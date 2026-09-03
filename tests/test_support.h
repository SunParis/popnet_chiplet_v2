#pragma once

#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace popnet_test {

class Failure : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

inline std::string location(const char* file, int line) {
    return std::string(file) + ':' + std::to_string(line);
}

inline void check(bool condition, const char* expression, const char* file, int line) {
    if (!condition) {
        throw Failure(location(file, line) + ": check failed: " + expression);
    }
}

template <typename Left, typename Right>
void checkEqual(const Left& left, const Right& right, const char* left_text, const char* right_text,
                const char* file, int line) {
    if (!(left == right)) {
        throw Failure(location(file, line) + ": expected " + left_text + " == " + right_text);
    }
}

inline void checkNear(double left, double right, double tolerance, const char* file, int line) {
    if (std::abs(left - right) > tolerance) {
        std::ostringstream message;
        message << location(file, line) << ": expected " << left << " ~= " << right << " within "
                << tolerance;
        throw Failure(message.str());
    }
}

template <typename Exception, typename Function>
void expectThrows(Function&& function, const char* file, int line) {
    try {
        std::forward<Function>(function)();
    } catch (const Exception&) {
        return;
    } catch (const std::exception& error) {
        throw Failure(location(file, line) + ": unexpected exception: " + error.what());
    }
    throw Failure(location(file, line) + ": expected exception was not thrown");
}

class TempDirectory {
  public:
    explicit TempDirectory(std::string_view label) {
        static std::size_t sequence = 0;
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
                ("popnet-" + std::string(label) + '-' + std::to_string(timestamp) + '-' +
                 std::to_string(sequence++));
        std::filesystem::create_directories(path_);
    }

    TempDirectory(const TempDirectory&) = delete;
    TempDirectory& operator=(const TempDirectory&) = delete;

    ~TempDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path_, error);
    }

    const std::filesystem::path& path() const noexcept {
        return path_;
    }

  private:
    std::filesystem::path path_;
};

class CurrentPathGuard {
  public:
    explicit CurrentPathGuard(const std::filesystem::path& path)
        : previous_(std::filesystem::current_path()) {
        std::filesystem::current_path(path);
    }

    CurrentPathGuard(const CurrentPathGuard&) = delete;
    CurrentPathGuard& operator=(const CurrentPathGuard&) = delete;

    ~CurrentPathGuard() {
        std::error_code error;
        std::filesystem::current_path(previous_, error);
    }

  private:
    std::filesystem::path previous_;
};

class Arguments {
  public:
    Arguments(std::initializer_list<std::string> values) : values_(values) {
        rebuild();
    }

    explicit Arguments(std::vector<std::string> values) : values_(std::move(values)) {
        rebuild();
    }

    int argc() const noexcept {
        return static_cast<int>(pointers_.size());
    }

    char* const* argv() noexcept {
        return pointers_.data();
    }

  private:
    void rebuild() {
        pointers_.reserve(values_.size());
        for (auto& value : values_) {
            pointers_.push_back(value.data());
        }
    }

    std::vector<std::string> values_;
    std::vector<char*> pointers_;
};

inline void writeFile(const std::filesystem::path& path, std::string_view contents) {
    std::ofstream output(path);
    if (!output) {
        throw Failure("Could not create test file: " + path.string());
    }
    output << contents;
}

inline std::string readFile(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw Failure("Could not read test file: " + path.string());
    }
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

inline std::size_t lineCount(std::string_view text) {
    std::size_t count = 0;
    for (const char value : text) {
        if (value == '\n') {
            ++count;
        }
    }
    return count;
}

using TestCase = std::pair<std::string_view, std::function<void()>>;

inline int run(const std::vector<TestCase>& tests) {
    std::size_t passed = 0;
    for (const auto& [name, test] : tests) {
        try {
            test();
            ++passed;
        } catch (const std::exception& error) {
            std::cerr << "[FAIL] " << name << ": " << error.what() << '\n';
        }
    }
    std::cout << passed << '/' << tests.size() << " tests passed\n";
    return passed == tests.size() ? 0 : 1;
}

} // namespace popnet_test

#define POPNET_CHECK(expression)                                                                   \
    ::popnet_test::check(static_cast<bool>(expression), #expression, __FILE__, __LINE__)
#define POPNET_CHECK_EQ(left, right)                                                               \
    ::popnet_test::checkEqual((left), (right), #left, #right, __FILE__, __LINE__)
#define POPNET_CHECK_NEAR(left, right, tolerance)                                                  \
    ::popnet_test::checkNear((left), (right), (tolerance), __FILE__, __LINE__)
#define POPNET_EXPECT_THROW(exception, expression)                                                 \
    ::popnet_test::expectThrows<exception>([&] { expression; }, __FILE__, __LINE__)

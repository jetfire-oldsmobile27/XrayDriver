#pragma once

#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>

namespace jetfire27::Engine::Logging {

class Logger {
public:
    static Logger& GetInstance();
    void Initialize(const std::string& logDirectory);

    template <typename... Args>
    void Info(fmt::format_string<Args...> fmt, Args&&... args) {
        logger_->info(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Error(fmt::format_string<Args...> fmt, Args&&... args) {
        logger_->error(fmt, std::forward<Args>(args)...);
    }

private:
    Logger() = default;
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace jetfire27::Engine::Logging
#include "logger.h"
#include <filesystem>

namespace jetfire27::Engine::Logging {

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

void Logger::Initialize(const std::string& logDirectory) {
    std::filesystem::create_directories(logDirectory);
    std::string logPath = logDirectory + "/jet_service.log";
    logger_ = spdlog::daily_logger_mt("daily_logger", logPath, 0, 0);
    logger_->set_level(spdlog::level::info);
    logger_->flush_on(spdlog::level::info);
}

} // namespace jetfire27::Engine::Logging
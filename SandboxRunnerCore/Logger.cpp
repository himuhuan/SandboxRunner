#include "Logger.h"

#include <new>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::unique_ptr<Logger> Logger::_instance = nullptr;
std::mutex Logger::_instanceMutex;

Logger::Logger(std::shared_ptr<spdlog::logger> logger)
    : _logger(std::move(logger))
{
}

spdlog::level::level_enum Logger::ToSpdlogLevel(LoggerLevel level)
{
    switch (level)
    {
    case LoggerLevel::Debug:
        return spdlog::level::debug;
    case LoggerLevel::Info:
        return spdlog::level::info;
    case LoggerLevel::Warning:
        return spdlog::level::warn;
    case LoggerLevel::Error:
        return spdlog::level::err;
    default:
        return spdlog::level::info;
    }
}

const char *Logger::ToLevelString(LoggerLevel level)
{
    switch (level)
    {
    case LoggerLevel::Debug:
        return "DEBUG";
    case LoggerLevel::Info:
        return "INFO";
    case LoggerLevel::Warning:
        return "WARN";
    case LoggerLevel::Error:
        return "ERROR";
    default:
        return "INFO";
    }
}

void Logger::FallbackWrite(LoggerLevel level, std::string_view message)
{
    fmt::print(stderr, "[logger-fallback][{}] {}\n", ToLevelString(level), message);
}

void Logger::Release()
{
    std::lock_guard<std::mutex> lock(_instanceMutex);
    if (_instance != nullptr)
        _instance.reset();
}

bool Logger::Initialize(const char *logName, const char *logFileName, LoggerLevel level)
{
    std::lock_guard<std::mutex> lock(_instanceMutex);
    if (_instance != nullptr)
        return true;

    const std::string loggerName = (logName != nullptr && *logName != '\0') ? logName : "sandbox";
    std::shared_ptr<spdlog::logger> logger;
    bool downgradedToStderr = false;

    auto configureLogger = [level](const std::shared_ptr<spdlog::logger> &targetLogger) {
        targetLogger->set_level(Logger::ToSpdlogLevel(level));
        targetLogger->flush_on(spdlog::level::warn);
        targetLogger->set_error_handler([](const std::string &msg) {
            fmt::print(stderr, "[logger-error] {}\n", msg);
        });
    };

    try
    {
        if (logFileName != nullptr && *logFileName != '\0')
        {
            auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileName, true);
            logger    = std::make_shared<spdlog::logger>(loggerName, std::move(sink));
        }
        else
        {
            auto sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
            logger    = std::make_shared<spdlog::logger>(loggerName, std::move(sink));
        }
    }
    catch (const std::exception &)
    {
        downgradedToStderr = true;
    }

    if (logger == nullptr)
    {
        try
        {
            auto sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
            logger    = std::make_shared<spdlog::logger>(loggerName, std::move(sink));
        }
        catch (const std::exception &)
        {
            return false;
        }
    }

    configureLogger(logger);

    _instance = std::unique_ptr<Logger>(new (std::nothrow) Logger(std::move(logger)));
    if (_instance == nullptr)
        return false;

    if (downgradedToStderr && logFileName != nullptr && *logFileName != '\0')
        _instance->_logger->warn("Failed to open log file '{}', switched to stderr sink", logFileName);

    return true;
}

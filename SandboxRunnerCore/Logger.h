#pragma once

#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <fmt/format.h>
#include <fmt/std.h>
#include <spdlog/spdlog.h>

class Logger
{
public:
    // Keep the existing logging entrypoints while delegating sink/formatting to spdlog.
    enum class LoggerLevel
    {
        Debug,
        Info,
        Warning,
        Error
    };

    template <typename... Arg> static void Debug(fmt::format_string<Arg...> message, Arg &&...args)
    {
        Log(LoggerLevel::Debug, message, std::forward<Arg>(args)...);
    }

    template <typename... Arg> static void Info(fmt::format_string<Arg...> message, Arg &&...args)
    {
        Log(LoggerLevel::Info, message, std::forward<Arg>(args)...);
    }

    template <typename... Arg> static void Warning(fmt::format_string<Arg...> message, Arg &&...args)
    {
        Log(LoggerLevel::Warning, message, std::forward<Arg>(args)...);
    }

    template <typename... Arg> static void Error(fmt::format_string<Arg...> message, Arg &&...args)
    {
        Log(LoggerLevel::Error, message, std::forward<Arg>(args)...);
    }

    static Logger *GetInstance()
    {
        std::lock_guard<std::mutex> lock(_instanceMutex);
        if (_instance == nullptr)
            throw std::runtime_error("Logger not initialized");
        return _instance.get();
    }

    static void Release();
    static bool Initialize(const char *logName, const char *logFileName, LoggerLevel level);

private:
    std::shared_ptr<spdlog::logger> _logger;
    static std::unique_ptr<Logger> _instance;
    static std::mutex _instanceMutex;

    explicit Logger(std::shared_ptr<spdlog::logger> logger);
    static spdlog::level::level_enum ToSpdlogLevel(LoggerLevel level);
    static const char *ToLevelString(LoggerLevel level);
    static void FallbackWrite(LoggerLevel level, std::string_view message);

public:
    ~Logger() = default;

    template <typename... Arg> static void Log(LoggerLevel level, fmt::format_string<Arg...> message, Arg &&...args)
    {
        std::string logMessage;
        try
        {
            logMessage = fmt::format(message, std::forward<Arg>(args)...);
        }
        catch (const std::exception &ex)
        {
            FallbackWrite(LoggerLevel::Error, fmt::format("Log format failed: {}", ex.what()));
            return;
        }

        std::shared_ptr<spdlog::logger> logger;
        {
            std::lock_guard<std::mutex> lock(_instanceMutex);
            if (_instance != nullptr)
                logger = _instance->_logger;
        }

        if (logger == nullptr)
        {
            FallbackWrite(level, logMessage);
            return;
        }

        try
        {
            logger->log(ToSpdlogLevel(level), logMessage);
        }
        catch (const std::exception &ex)
        {
            FallbackWrite(LoggerLevel::Error, fmt::format("Log sink failed: {}", ex.what()));
            FallbackWrite(level, logMessage);
        }
    }
};

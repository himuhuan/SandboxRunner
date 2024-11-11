#pragma once

#include <cstdio>
#include <string>

#include <fmt/std.h>
#include <fmt/chrono.h>

#if __linux__
/* Linux fork require lock logger file */
#include <sys/file.h>
#include <unistd.h>
#endif

class Logger
{
public:
    enum class LoggerLevel
    {
        Debug,
        Info,
        Warning,
        Error
    };

#ifdef DEBUG
    const LoggerLevel Level = LoggerLevel::Debug;
#else
    const LoggerLevel Level = LoggerLevel::Info;
#endif

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
        if (_instance == nullptr)
            throw std::runtime_error("Logger not initialized");
        return _instance;
    }

    static void Release()
    {
        if (_instance != nullptr)
        {
            fclose(_instance->_logFile);
            delete _instance;
            _instance = nullptr;
        }
    }

    static bool Initialize(const char *logName, const char *logFileName, LoggerLevel level)
    {
        if (_instance == nullptr)
        {
            _instance = new (std::nothrow) Logger(logName, level);
            if (_instance == nullptr)
                return false;
            _instance->_logFile = (logFileName == nullptr) ? stderr : fopen(logFileName, "a");
            if (_instance->_logFile == nullptr)
                return false;
        }
        return true;
    }

private:
    const char *_loggerName;
    FILE *_logFile;
    static Logger *_instance;

    Logger(const char *name, LoggerLevel level);
    ~Logger();

    template <typename... Arg> static void Log(LoggerLevel level, fmt::format_string<Arg...> message, Arg &&...args)
    {
        if (_instance->Level >= level)
            return;
        const char *levelStr = nullptr;
        switch (level)
        {
        case LoggerLevel::Debug:
            levelStr = "[DEBUG]";
            break;
        case LoggerLevel::Info:
            levelStr = "[INFO]";
            break;
        case LoggerLevel::Warning:
            levelStr = "[WARNING]";
            break;
        case LoggerLevel::Error:
            levelStr = "[ERROR]";
            break;
        }

        std::time_t currentTime = std::time(nullptr);
        std::string logPrefix = fmt::format("[{0:%Y-%m-%d %H:%M:%S}][{1}]{2} ", fmt::localtime(currentTime),
                                            _instance->_loggerName, levelStr);
        std::string logMessage = fmt::format(message, std::forward<Arg>(args)...);
        if (GetInstance()->_logFile == nullptr)
        {
            fmt::print(stderr, "{0}{1}\n", logPrefix, logMessage);
            return;
        }
#if __linux
        int logFd = fileno(GetInstance()->_logFile);
        std::string logStr = logPrefix + logMessage + "\n";
        if (flock(logFd, LOCK_EX) == 0)
        {
            if (write(logFd, logStr.c_str(), logStr.size() * sizeof(char)) < 0)
                fmt::print(stderr, "FATAL: write log failed: {0}\n", logMessage);
            flock(logFd, LOCK_UN);
        }
        else
        {
            fmt::print(stderr, "FATAL: flock failed: {0}\n", logMessage);
        }
#endif
    }
};
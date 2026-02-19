#include "Logger.h"

std::unique_ptr<Logger> Logger::_instance = nullptr;

Logger::Logger(const char *name, LoggerLevel level, FILE *logFile)
    : Level(level), _loggerName(name), _logFile(logFile)
{
}
#include "Logger.h"

Logger *Logger::_instance = nullptr;

Logger::~Logger()
{
    Release();
}

Logger::Logger(const char *name, LoggerLevel level) 
    : _logFile(nullptr), _loggerName(name), Level(level)
{
}
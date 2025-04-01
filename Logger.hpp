#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/time.h>

#define LOG_DEBUG 0

#define COLOR_RED "\001\033[31m\002"
#define COLOR_GREEN "\001\033[32m\002"
#define COLOR_YELLOW "\001\033[33m\002"
#define COLOR_CYAN "\001\033[36m\002"
#define COLOR_WHITE "\001\033[48;5;7m\002"
#define COLOR_UNDERLINE   "\001\033[4m\002"
#define RESET_COLOR "\001\033[0m\002"

enum LogLevel {
    INFO,
    WARNING,
    DEBUG,
    ERROR,
    FATAL,
};

class Logger {
  private:
    Logger() {};
    Logger(const Logger&) {};
    Logger& operator=(const Logger&) { return *this; };
    ~Logger() {};

    static std::string   formatMessage(const std::string&);
    static std::ostream& logMessage(LogLevel level, const std::string&);

  public:
    static std::ostream& logInfo(const std::string&);
    static std::ostream& logWarning(const std::string&);
    static std::ostream& logDebug(const std::string&);
    static std::ostream& logFatal(const std::string&);
    static std::ostream& logError(const std::string&);
};

#endif

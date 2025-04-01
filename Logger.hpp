#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <sys/time.h>

#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_UNDERLINE "\033[4m"
#define RESET_COLOR "\033[0m"

#define LOG_DEBUG 0

enum LogLevel {
    FATAL,
    ERROR,
    WARNING,
    INFO,
    DEBUG
};

class Logger {
public:
    static std::ostream &logMessage(LogLevel level, const std::string &message);
    static std::ostream &logInfo(const std::string &message);
    static std::ostream &logWarning(const std::string &message);
    static std::ostream &logDebug(const std::string &message);
    static std::ostream &logError(const std::string &message);
    static std::ostream &logFatal(const std::string &message);

private:
    static void highlightUrls(std::string &message, const std::string &protocol);
    static std::map<LogLevel, std::string> createLevelColors();
    static std::string formatMessage(const std::string &inputMessage);
    static std::string getCurrentDateTime();
};

#endif

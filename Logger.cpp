#include "Logger.hpp"

static std::string getCurrentDateTime() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    time_t seconds = currentTime.tv_sec;
    struct tm *localTime = localtime(&seconds);

    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);

    std::stringstream timeStream;
    timeStream << buffer << '.' << (currentTime.tv_usec / 100);
    return timeStream.str();
}

void Logger::highlightUrls(std::string &message, const std::string &protocol) {
    size_t position = 0;
    while ((position = message.find(protocol, position)) != std::string::npos) {
        size_t endPosition = message.find_first_of(" \t\n", position);
        if (endPosition == std::string::npos) {
            endPosition = message.length();
        }

        std::string urlSubstring = message.substr(position, endPosition - position);
        std::string highlightedUrl =
            std::string(COLOR_CYAN) + COLOR_UNDERLINE + urlSubstring + RESET_COLOR;
        message.replace(position, urlSubstring.length(), highlightedUrl);
        position += highlightedUrl.length();
    }
}

std::string Logger::formatMessage(const std::string &inputMessage) {
    std::string outputMessage = inputMessage;
    highlightUrls(outputMessage, "http://");
    highlightUrls(outputMessage, "https://");
    return outputMessage;
}

std::ostream &Logger::logMessage(LogLevel level, const std::string &message) {
    static const std::map<LogLevel, std::string> levelColors = createLevelColors();

    std::ostream &stream = (level <= ERROR) ? std::cerr : std::cout;

    stream << "[" << getCurrentDateTime() << "] ";

    std::map<LogLevel, std::string>::const_iterator it = levelColors.find(level);
    if (it != levelColors.end()) {
        stream << it->second;
    } else {
        stream << "[UNKNOWN]";
    }

    stream << RESET_COLOR << " " << formatMessage(message) << std::endl;
    return stream;
}

std::ostream &Logger::logInfo(const std::string &message) {
    return logMessage(INFO, message);
}

std::ostream &Logger::logWarning(const std::string &message) {
    return logMessage(WARNING, message);
}

std::ostream &Logger::logDebug(const std::string &message) {
    return LOG_DEBUG ? logMessage(DEBUG, message) : std::cerr;
}

std::ostream &Logger::logError(const std::string &message) {
    return logMessage(ERROR, message);
}

std::ostream &Logger::logFatal(const std::string &message) {
    return logMessage(FATAL, message);
}

std::map<LogLevel, std::string> Logger::createLevelColors() {
    std::map<LogLevel, std::string> levelColors;
    levelColors[FATAL] = COLOR_RED COLOR_WHITE "[FATAL]";
    levelColors[ERROR] = COLOR_RED "[ERROR]";
    levelColors[WARNING] = COLOR_YELLOW "[WARNING]";
    levelColors[INFO] = COLOR_CYAN "[INFO]";
    levelColors[DEBUG] = COLOR_GREEN "[DEBUG]";
    return levelColors;
}

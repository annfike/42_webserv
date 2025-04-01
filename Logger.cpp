#include "./Logger.hpp"

static std::string getCurrentDateTime() {
    char formattedDateTime[30];
    struct timeval currentTime;
    gettimeofday(&currentTime, nullptr);
    std::stringstream timeStream;
    time_t seconds = currentTime.tv_sec;
    struct tm *localTime = localtime(&seconds);
    strftime(formattedDateTime, sizeof(formattedDateTime), "%Y-%m-%d %H:%M:%S", localTime);
    timeStream << formattedDateTime << "." << currentTime.tv_usec / 100;
    return (timeStream.str());
}

std::string Logger::formatMessage(const std::string& inputMessage) {
    std::string outputMessage = inputMessage;

    std::size_t currentPosition = 0;
    while ((currentPosition = outputMessage.find("http://", currentPosition)) != std::string::npos) {
        std::size_t endPosition = outputMessage.find(" \t\n", currentPosition);
        if (endPosition == std::string::npos) {
            endPosition = outputMessage.length();
        }
        std::string urlSubstring = outputMessage.substr(currentPosition, endPosition - currentPosition);
        std::string highlightedUrl = std::string(COLOR_CYAN) + COLOR_UNDERLINE + urlSubstring + RESET_COLOR;
        outputMessage.replace(currentPosition, urlSubstring.length(), highlightedUrl);
        currentPosition += highlightedUrl.length();
    }

    currentPosition = 0;
    while ((currentPosition = outputMessage.find("https://", currentPosition)) != std::string::npos) {
        std::size_t endPosition = outputMessage.find(" \t\n", currentPosition);
        if (endPosition == std::string::npos) {
            endPosition = outputMessage.length();
        }
        std::string urlSubstring = outputMessage.substr(currentPosition, endPosition - currentPosition);
        std::string highlightedUrl = std::string(COLOR_CYAN) + COLOR_UNDERLINE + urlSubstring + RESET_COLOR;
        outputMessage.replace(currentPosition, urlSubstring.length(), highlightedUrl);
        currentPosition += highlightedUrl.length();
    }
    return outputMessage;
}

std::ostream& Logger::logMessage(LogLevel level, const std::string& message) {
    std::ostream& stream = level <= 2 ? std::cerr : std::cout;
    stream << "[" << getCurrentDateTime() << "] ";
    switch(level) {
        case FATAL:
            stream << COLOR_RED << COLOR_WHITE << "[FATAL]";
            break;
        case ERROR:
            stream << COLOR_RED << "[ERROR]";
            break;
        case WARNING:
            stream << COLOR_YELLOW << "[WARNING]";
            break;
        case INFO:
            stream << COLOR_CYAN << "[INFO]";
            break;
        case DEBUG:
            stream << COLOR_GREEN << "[DEBUG]";
            break;
    }
    stream << RESET_COLOR << " " << formatMessage(message);
    return stream;
}

std::ostream& Logger::logInfo(const std::string& message) {
    return Logger::logMessage(INFO, message);
}

std::ostream& Logger::logWarning(const std::string& message) {
    return Logger::logMessage(WARNING, message);
}

std::ostream& Logger::logDebug(const std::string& message) {
    if (LOG_DEBUG)
        return Logger::logMessage(DEBUG, message);
    else
        return std::cerr;
}

std::ostream& Logger::logError(const std::string& message) {
    return Logger::logMessage(ERROR, message);
}

std::ostream& Logger::logFatal(const std::string& message) {
    return Logger::logMessage(FATAL, message);
}

static std::ostream stream(0);

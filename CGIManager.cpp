#include "CGIManager.hpp"

CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

CgiHandler::CgiHandler(const CgiHandler& obj)
{
    this->cgiPath = obj.cgiPath;
}

CgiHandler& CgiHandler::operator=(const CgiHandler& obj) {
    if (this != &obj) {
        this->cgiPath = obj.cgiPath;
    }
    return (*this);
}

void CgiHandler::executeCgiProcess(short& error_code)
{
    if (this->cgi_args_str[0].empty() || this->cgi_args_str[1].empty()) {
        error_code = 500;
        return;
    }

    if (pipe(cgi_input_pipe) < 0) {
        Logger::logError("pipe() failed");
        error_code = 500;
        return;
    }

    if (pipe(cgi_output_pipe) < 0) {
        Logger::logError("pipe() failed");
        close(cgi_input_pipe[0]);
        close(cgi_input_pipe[1]);
        error_code = 500;
        return;
    }

    pid_t cgi_pid = fork();
    
    if (cgi_pid == 0) {
        Logger::logInfo("Fork successful.");
        dup2(cgi_input_pipe[0], STDIN_FILENO);
        dup2(cgi_output_pipe[1], STDOUT_FILENO);

        close(cgi_input_pipe[0]);
        close(cgi_input_pipe[1]);
        close(cgi_output_pipe[0]);
        close(cgi_output_pipe[1]);

        char * cgi_args[3];
        cgi_args[0] = (char*)cgi_args_str[0].c_str();
        cgi_args[1] = (char*)cgi_args_str[1].c_str();
        cgi_args[2] = (char*)cgi_args_str[2].c_str();
        cgi_args[3] = NULL;

        char* cgi_envs[1];
        cgi_envs[0] = NULL;

        int status_exit = execve(cgi_args[0], cgi_args, cgi_envs);

        perror("execve failed");
        exit(status_exit);
    }
    else if (cgi_pid > 0) {
        close(cgi_input_pipe[0]);
        close(cgi_input_pipe[1]);
        close(cgi_output_pipe[1]);

        int status;
        waitpid(cgi_pid, &status, 0);
        if (WIFEXITED(status)) {
            error_code = WEXITSTATUS(status);
        } else {
            error_code = 500;
        }
    } else {
        error_code = 500;
    }
}

int CgiHandler::findSubstringPosition(const std::string& inputString, const std::string& delimiter) 
{
    if (inputString.empty())
        return -1;

    size_t position = inputString.find(delimiter);
    return (position != std::string::npos) ? static_cast<int>(position) : -1;
}

std::string CgiHandler::readCgiOutput() {
    char        buffer[128];
    std::string result = "";
    ssize_t     bytesRead;

    // Logger::logInfo("Reading from pipe...");

    // Чтение из pipe
    while ((bytesRead = read(cgi_output_pipe[0], buffer, sizeof(buffer))) > 0) {
        result.append(buffer, bytesRead);
    }

    if (bytesRead == -1) {
        perror("Error reading from pipe");
    }

    // Logger::logInfo("CGI Output: " + result);
    return result;
}

Response CgiHandler::exec(HttpRequestParser request, std::string cgiPath) {

    this->cgi_args_str[0] = "/usr/bin/python3";
	this->cgi_args_str[1] = cgiPath;
    this->cgi_args_str[2] = request.query;

    if (cgi_args_str[0].empty()) 
        return Response(Response::ERROR, 500, "CGI Execution Error4", "", cgiPath, "");
    
    short error_code = 0;
    executeCgiProcess(error_code);

    if (error_code == 0) {
        std::string cgi_output = readCgiOutput();
        if (cgi_output.empty())
            Logger::logWarning("CGI Output is empty!");
        return Response(Response::CGI, 200, "CGI Execution", "", cgiPath, cgi_output);
    }
    else
        return Response(Response::ERROR, 500, "CGI Execution Error3", "", cgiPath, "");
}

bool CgiHandler::isCGIExtension(const std::string& localPath) 
{
    const std::string pyExtension = ".py";

    size_t      queryPos         = localPath.find('?');
    std::string pathWithoutQuery = localPath;

    if (queryPos != std::string::npos)
        pathWithoutQuery = localPath.substr(0, queryPos);

    size_t dotPos = pathWithoutQuery.rfind('.');
    if (dotPos == std::string::npos)
        return false;

    std::string extension = pathWithoutQuery.substr(dotPos);
    if (extension == pyExtension)
        return true;

    return false;
}

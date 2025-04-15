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

bool CgiHandler::executeCgiProcess(Connection& conn, short& error_code)
{
    if (pipe(cgi_input_pipe) < 0 || pipe(cgi_output_pipe) < 0) {
        error_code = 500;
        return false;
    }

    pid_t pid = fork();
    if (pid == 0) {
        dup2(cgi_input_pipe[0], STDIN_FILENO);
        dup2(cgi_output_pipe[1], STDOUT_FILENO);

        close(cgi_input_pipe[0]);
        close(cgi_input_pipe[1]);
        close(cgi_output_pipe[0]);
        close(cgi_output_pipe[1]);

        char *args[] = {
            (char*)cgi_args_str[0].c_str(),
            (char*)cgi_args_str[1].c_str(),
            (char*)cgi_args_str[2].c_str(),
            NULL
        };

        char *envp[] = { NULL };

        execve(args[0], args, envp);
        exit(1);
    } else if (pid > 0) {
        close(cgi_input_pipe[0]);
        close(cgi_input_pipe[1]);
        close(cgi_output_pipe[1]);

        conn.cgi_output_fd = cgi_output_pipe[0];
        conn.cgi_pid = pid;
        conn.cgi_ready = false;

        conn.poll.fd = conn.cgi_output_fd;
        conn.poll.events = POLLIN;

        return true;
    } else {
        error_code = 500;
        return false;
    }
}

Response CgiHandler::exec(HttpRequestParser request, std::string cgiPath, Connection& conn)
{
    this->cgi_args_str[0] = "/usr/bin/python3";
	this->cgi_args_str[1] = cgiPath;
	this->cgi_args_str[2] = request.query;

    short error_code = 0;

    if (!executeCgiProcess(conn, error_code)) {
        std::cerr << "Error executing CGI process for " << cgiPath << std::endl;
        return Response(Response::ERROR, 500, "CGI Execution Error", "", cgiPath, "");
    }

    std::cerr << "Waiting for CGI process to complete..." << std::endl;

    std::string cgi_output = conn.cgi_output;

    if (cgi_output.empty()) {
        std::cerr << "CGI process completed but no output returned." << std::endl;
        return Response(Response::ERROR, 500, "No Output from CGI", "", cgiPath, "");
    }
    return Response(Response::CGI, 200, "CGI Execution Complete", cgi_output, cgiPath, "");
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

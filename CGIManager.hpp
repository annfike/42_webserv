#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>

#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "ServerConfig.hpp"

// Предварительное объявление класса Response (это позволяет компилятору знать о типе Response)
class Response;

#include "HttpResponse.hpp"

class CgiHandler {
  private:
    std::map<std::string, std::string> cgi_env_variables;

    const char *                       cgi_envs[30];
    const char *                       cgi_args[3];
    int                                status_exit;
    std::string                        cgi_path;
    pid_t                              cgi_pid;

  public:
    int cgi_input_pipe[2];
    int cgi_output_pipe[2];

    CgiHandler();
    CgiHandler(std::string path);
    ~CgiHandler();
    CgiHandler(CgiHandler const& obj);
    CgiHandler& operator=(CgiHandler const& rhs);

    void prepareCgiExecutionEnv(HttpRequestParser& request, std::string cgiPath, std::string& root);
    void setupCgiEnvironment(HttpRequestParser& request, std::string& cgiPath);
    void executeCgiProcess(short& error_code);
    void executeCgiProcessForPost(const std::string& body, short& error_code);
    void setCgiPath(const std::string& cgi_path);
    void setCgiPid(pid_t cgi_pid);
    Response exec(HttpRequestParser request, std::string cgiPath, std::string root);
    Response execPost(HttpRequestParser request, std::string cgiPath, std::string root);
    std::string readCgiOutput();

    const pid_t& getCgiPid() const;
    const std::string& getCgiPath() const;

    int findSubstringPosition(const std::string& inputString, const std::string& delimiter);

    std::string extractPathInfoFromExtension(std::string& path, std::vector<std::string> extensions);
    std::string urlDecode(std::string& path);
    bool isCGIExtension(const std::string& localPath);
    void freeCgiEnv();
    void freeCgiArgs();
};

#endif

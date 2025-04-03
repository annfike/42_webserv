#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "ServerConfig.hpp"
#include "HttpResponse.hpp"

class CgiHandler {
  private:
    std::map<std::string, std::string> cgi_env_variables;

    char**                             cgi_envs;
    char**                             cgi_args;
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

    void prepareCgiExecutionEnv(HttpRequestParser& request, const ServerConfig::Location& location);
    void setupCgiEnvironment(HttpRequestParser& request, const ServerConfig::Location& location);
    void executeCgiProcess(short& error_code);
    void setCgiPath(const std::string& cgi_path);
    void setCgiPid(pid_t cgi_pid);
    short exec(const ServerConfig::Location& location, HttpRequestParser request);
    std::string readCgiOutput();

    const pid_t& getCgiPid() const;
    const std::string& getCgiPath() const;

    int findSubstringPosition(const std::string& inputString, const std::string& delimiter);

    std::string extractPathInfoFromExtension(std::string& path, std::vector<std::string> extensions);
    std::string urlDecode(std::string& path);
    bool isCGIExtension(const std::string& localPath);
};

#endif

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
#include "HttpRequest.hpp"

class CgiHandler {
  private:
    std::string                        cgiPath;
    std::string                        cgi_args_str[3];

  public:
    int cgi_input_pipe[2];
    int cgi_output_pipe[2];

    CgiHandler();
    ~CgiHandler();
    CgiHandler(CgiHandler const& obj);
    CgiHandler& operator=(CgiHandler const& rhs);

    void executeCgiProcess(short& error_code);
    void executeCgiProcessForPost(const std::string& body, short& error_code);
    Response exec(HttpRequestParser request, std::string cgiPathroot);
    std::string readCgiOutput();

    const pid_t& getCgiPid() const;
    const std::string& getCgiPath() const;

    int findSubstringPosition(const std::string& inputString, const std::string& delimiter);

    std::string extractPathInfoFromExtension(std::string& path, std::vector<std::string> extensions);
    std::string urlDecode(std::string& path);
    bool isCGIExtension(const std::string& localPath);
};

#endif

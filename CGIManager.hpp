#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#define CGI_ERROR "CGI execution failed"

class CgiHandler {
  private:
    std::vector<std::string>           envVariables;
    std::vector<char*>                 envPointers;
    std::string                        requestMethod;
    std::string                        cgiInterpreter;
    std::string                        scriptExtension;
    std::string                        scriptPath;
    std::map<std::string, std::string> responseHeaders;
    std::string                        responseBody;

    void setPostEnvironmentVariables(std::map<std::string, std::string>& headers,
                                     std::string& query, std::string& body);
    void setGetEnvironmentVariables(std::map<std::string, std::string>& headers,
                                    std::string&                        query);
    void parseCgiOutput(const std::string& output);
    void executeCgiScript(bool isPost, const std::string& requestBody);
    bool hasShebang(const std::string& filePath);

  public:
    CgiHandler(const std::string& method, const std::string& interpreter,
               const std::string& extension, const std::string& script);

    void run(std::map<std::string, std::string>& headers, std::string& query,
             std::string& requestBody);

    const std::map<std::string, std::string>& getResponseHeaders() const;
    const std::string&                        getResponseBody() const;
    bool                                      isValidCgi();
};

#endif

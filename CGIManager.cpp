#include "CGIManager.hpp"

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

    void setPostEnvironmentVariables(std::map<std::string, std::string>& headers, std::string& query, std::string& body) {
        envVariables.clear();
        envPointers.clear();

        std::string contentType =
            (headers.count("Content-Type")) ? headers["Content-Type"] : "text/plain";
        std::string cookies = (headers.count("Cookie")) ? headers["Cookie"] : "";

        envVariables = {"GATEWAY_INTERFACE=CGI/1.1",
                        "REQUEST_METHOD=" + requestMethod,
                        "SCRIPT_FILENAME=" + scriptPath,
                        "SERVER_PROTOCOL=HTTP/1.1",
                        "SERVER_SOFTWARE=Placeholder",
                        "REDIRECT_STATUS=200",
                        "CONTENT_LENGTH=" + std::to_string(body.size()),
                        "CONTENT_TYPE=" + contentType,
                        "QUERY_STRING=" + query};

        if (!cookies.empty()) {
            envVariables.push_back("HTTP_COOKIE=" + cookies);
        }

        for (size_t i = 0; i < envVariables.size(); i++) {
            envPointers.push_back(const_cast<char*>(envVariables[i].c_str()));
        }
        envPointers.push_back(NULL);
    }

    void setGetEnvironmentVariables(std::map<std::string, std::string>& headers, std::string& query) {
        envVariables.clear();
        envPointers.clear();

        std::string cookies = (headers.count("Cookie")) ? headers["Cookie"] : "";

        envVariables = {"GATEWAY_INTERFACE=CGI/1.1",     "REQUEST_METHOD=" + requestMethod,
                        "SCRIPT_FILENAME=" + scriptPath, "SERVER_PROTOCOL=HTTP/1.1",
                        "SERVER_SOFTWARE=Placeholder",   "REDIRECT_STATUS=200",
                        "QUERY_STRING=" + query};

        if (!cookies.empty()) {
            envVariables.push_back("HTTP_COOKIE=" + cookies);
        }

        for (size_t i = 0; i < envVariables.size(); i++) {
            envPointers.push_back(const_cast<char*>(envVariables[i].c_str()));
        }
        envPointers.push_back(NULL);
    }

    void parseCgiOutput(const std::string& output) {
        size_t headerEnd = output.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            headerEnd = output.find("\n\n");
        }

        std::string headersPart = output.substr(0, headerEnd);
        responseBody            = output.substr(headerEnd + 2);

        std::istringstream headerStream(headersPart);
        std::string        line;
        while (std::getline(headerStream, line)) {
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string key      = line.substr(0, colonPos);
                std::string value    = line.substr(colonPos + 1);
                responseHeaders[key] = value;
            }
        }
    }

    void executeCgiScript(bool isPost, const std::string& requestBody) {
        int inputPipe[2], outputPipe[2];
        if (pipe(outputPipe) == -1 || (isPost && pipe(inputPipe) == -1)) {
            throw std::runtime_error("Failed to create pipes.");
        }
        pid_t pid = fork();
        if (pid == -1) {
            throw std::runtime_error("Failed to fork process.");
        } else if (pid == 0) {
            if (isPost) {
                close(inputPipe[1]);
                dup2(inputPipe[0], STDIN_FILENO);
                close(inputPipe[0]);
            }

            close(outputPipe[0]);
            dup2(outputPipe[1], STDOUT_FILENO);
            close(outputPipe[1]);

            char* args[] = {const_cast<char*>(cgiInterpreter.c_str()),
                            const_cast<char*>(scriptPath.c_str()), NULL};
            execve(cgiInterpreter.c_str(), args, envPointers.data());
            exit(EXIT_FAILURE);

        } else {
            if (isPost) {
                close(inputPipe[0]);
                write(inputPipe[1], requestBody.c_str(), requestBody.size());
                close(inputPipe[1]);
            }

            close(outputPipe[1]);
            char        buffer[1024];
            std::string output;
            ssize_t     bytesRead;
            while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
                output.append(buffer, bytesRead);
            }
            close(outputPipe[0]);

            int status;
            waitpid(pid, &status, 0);
            if (status != 0) {
                throw std::runtime_error(CGI_ERROR);
            }

            parseCgiOutput(output);
        }
    }

    bool hasShebang(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open())
            return false;

        std::string line;
        while (std::getline(file, line) && line.empty())
            continue;

        return (line.substr(0, 2) == "#!" && line.substr(2) == cgiInterpreter);
    }

  public:
    CgiHandler(const std::string& method, const std::string& interpreter, const std::string& extension, const std::string& script)
        : requestMethod(method), cgiInterpreter(interpreter), scriptExtension(extension), scriptPath(script) {}

    void run(std::map<std::string, std::string>& headers, std::string& query, std::string& requestBody) {
        if (requestMethod != "GET" && requestMethod != "POST") {
            throw std::runtime_error("Invalid method for CGI.");
        }

        if (requestMethod == "GET") {
            setGetEnvironmentVariables(headers, query);
            executeCgiScript(false, requestBody);
        } else {
            setPostEnvironmentVariables(headers, query, requestBody);
            executeCgiScript(true, requestBody);
        }
    }

    const std::map<std::string, std::string>& getResponseHeaders() const { return responseHeaders; }
    const std::string& getResponseBody() const { return responseBody; }

    bool isValidCgi() {
        size_t      dotPos    = scriptPath.find_last_of('.');
        std::string extension = (dotPos != std::string::npos) ? scriptPath.substr(dotPos + 1) : "";
        return (extension == scriptExtension || hasShebang(scriptPath));
    }
};

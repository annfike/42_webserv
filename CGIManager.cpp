#include "CGIManager.hpp"

CgiHandler::CgiHandler()
{
    this->cgi_pid     = -1;
    this->cgi_path    = "";
    this->status_exit = 0;
}

CgiHandler::CgiHandler(std::string path)
{
    this->cgi_pid     = -1;
    this->cgi_path    = path;
    this->status_exit = 0;
}

CgiHandler::~CgiHandler()
{}

CgiHandler::CgiHandler(const CgiHandler& obj)
{
    this->cgi_env_variables     = obj.cgi_env_variables;
    this->cgi_path              = obj.cgi_path;
    this->cgi_pid               = obj.cgi_pid;
    this->status_exit           = obj.status_exit;
}

CgiHandler& CgiHandler::operator=(const CgiHandler& obj)
{
    if (this != &obj)
    {
        this->cgi_env_variables = obj.cgi_env_variables;
        this->cgi_path          = obj.cgi_path;
        this->cgi_pid           = obj.cgi_pid;
        this->status_exit       = obj.status_exit;
    }
    return (*this);
}

// getters //
void CgiHandler::setCgiPid(pid_t cgi_pid)
{
    this->cgi_pid = cgi_pid;
}

void CgiHandler::setCgiPath(const std::string& cgi_path)
{
    this->cgi_path = cgi_path;
}

// setters //
const pid_t& CgiHandler::getCgiPid() const
{
    return (this->cgi_pid);
}

const std::string& CgiHandler::getCgiPath() const
{
    return (this->cgi_path);
}

// envs //
void CgiHandler::setupCgiEnvironment(HttpRequestParser& request, const ServerConfig::Location& location)
{
    std::string cgi_executable_path = std::string(location.cgiPath);

    if (request.getMethod() == "POST") {
        std::stringstream out;
        out << request.getBody().length();
        this->cgi_env_variables["CONTENT_LENGTH"] = out.str();
        this->cgi_env_variables["CONTENT_TYPE"]   = request.getHeader("content-type");
    }

    this->cgi_env_variables["GATEWAY_INTERFACE"] = std::string("CGI/1.1");
    this->cgi_env_variables["SCRIPT_NAME"]       = cgi_executable_path;
    this->cgi_env_variables["SCRIPT_FILENAME"]   = cgi_executable_path;
    this->cgi_env_variables["PATH_INFO"]         = cgi_executable_path;
    this->cgi_env_variables["PATH_TRANSLATED"]   = cgi_executable_path;
    this->cgi_env_variables["REQUEST_URI"]       = cgi_executable_path;
    this->cgi_env_variables["SERVER_NAME"]       = request.getHeader("host");
    this->cgi_env_variables["SERVER_PORT"]       = "8006";
    this->cgi_env_variables["REQUEST_METHOD"]    = request.getMethod();
    this->cgi_env_variables["SERVER_PROTOCOL"]   = "HTTP/1.1";
    this->cgi_env_variables["REDIRECT_STATUS"]   = "200";

    // // Перебираем заголовки HTTP-запроса и добавляем их в переменные окружения в формате CGI (например, HTTP_USER_AGENT, HTTP_COOKIE).
    std::map<std::string, std::string> request_headers = request.getHeaders();
    for (std::map<std::string, std::string>::iterator iterator = request_headers.begin();
        iterator != request_headers.end(); ++iterator) {
        std::string name = iterator->first;
        for (size_t i = 0; i < name.length(); ++i) {
            name[i] = std::toupper(name[i]);
        }
        std::string key = "HTTP_" + name;
        cgi_env_variables[key] = iterator->second;
    }

    // // вывожу все cgi_env_variables для дебага
    // for (std::map<std::string, std::string>::iterator it = cgi_env_variables.begin();
    //     it != cgi_env_variables.end(); ++it) {
    //     std::cout << it->first << " = " << it->second << std::endl;
    // }

    // Конвертируем std::map в массив строк в формате ключ=значение, который требует execve().
    // Создаем массив аргументов для CGI-программы
    this->cgi_envs = (char **)calloc(sizeof(char *), this->cgi_env_variables.size() + 1);
	std::map<std::string, std::string>::const_iterator it = this->cgi_env_variables.begin();
	for (int i = 0; it != this->cgi_env_variables.end(); it++, i++)
	{
		std::string tmp = it->first + "=" + it->second;
		this->cgi_envs[i] = strdup(tmp.c_str());
	}
	this->cgi_args = (char **)malloc(sizeof(char *) * 3);
	this->cgi_args[0] = strdup(cgi_executable_path.c_str());
	this->cgi_args[1] = strdup(cgi_executable_path.c_str());
	this->cgi_args[2] = NULL;

    // // Вывод содержимого cgi_envs, если это char**
    // for (int i = 0; this->cgi_envs[i] != NULL; ++i) {
    //     std::cout << "cgi_envs[" << i << "] = " << this->cgi_envs[i] << std::endl;
    // }
    // // Вывод содержимого cgi_args
    // for (int i = 0; this->cgi_args[i] != NULL; ++i) {
    //     std::cout << "cgi_args[" << i << "] = " << this->cgi_args[i] << std::endl;
    // }
}

void CgiHandler::prepareCgiExecutionEnv(HttpRequestParser& request, const ServerConfig::Location& location)
{
    int         position;
    std::string scriptExtension;

    scriptExtension = location.cgiPath.substr(location.cgiPath.find("."));

    this->cgi_env_variables["AUTH_TYPE"]         = "Basic";
    this->cgi_env_variables["CONTENT_LENGTH"]    = request.getHeader("content-length");
    this->cgi_env_variables["CONTENT_TYPE"]      = request.getHeader("content-type");
    this->cgi_env_variables["GATEWAY_INTERFACE"] = "CGI/1.1";

    position = findSubstringPosition(location.cgiPath, "cgi-bin/");
    this->cgi_env_variables["SCRIPT_NAME"]       = location.cgiPath;
    this->cgi_env_variables["SCRIPT_FILENAME"]   = ((position < 0 || (size_t) (position + 8) > location.cgiPath.size()) ? "" : location.cgiPath.substr(position + 8, location.cgiPath.size()));
    this->cgi_env_variables["PATH_INFO"]         = extractPathInfoFromExtension(request.getPath(), location.cgi_extension);
    this->cgi_env_variables["PATH_TRANSLATED"]   = location.root + (this->cgi_env_variables["PATH_INFO"] == "" ? "/" : this->cgi_env_variables["PATH_INFO"]);
    this->cgi_env_variables["QUERY_STRING"]      = urlDecode(request.getQuery());
    this->cgi_env_variables["REMOTE_ADDR"]       = request.getHeader("host");

    position = findSubstringPosition(request.getHeader("host"), ":");
    this->cgi_env_variables["SERVER_NAME"]       = (position > 0 ? request.getHeader("host").substr(0, position) : "");
    this->cgi_env_variables["SERVER_PORT"]       = (position > 0 ? request.getHeader("host").substr(position + 1, request.getHeader("host").size()) : "");
    this->cgi_env_variables["REQUEST_METHOD"]    = request.getMethod();
    this->cgi_env_variables["HTTP_COOKIE"]       = request.getHeader("cookie");
    this->cgi_env_variables["DOCUMENT_ROOT"]     = location.root;
    this->cgi_env_variables["REQUEST_URI"]       = request.getPath() + request.getQuery();
    this->cgi_env_variables["SERVER_PROTOCOL"]   = "HTTP/1.1";
    this->cgi_env_variables["REDIRECT_STATUS"]   = "200";

    // Создаем массив переменных окружения для CGI-процесс
    this->cgi_envs = (char **)calloc(sizeof(char *), this->cgi_env_variables.size() + 1);
	std::map<std::string, std::string>::const_iterator it = this->cgi_env_variables.begin();
	for (int i = 0; it != this->cgi_env_variables.end(); it++, i++)
	{
		std::string tmp = it->first + "=" + it->second;
		this->cgi_envs[i] = strdup(tmp.c_str());
	}

	this->cgi_args = (char **)malloc(sizeof(char *) * 3);
	this->cgi_args[0] = strdup(location.cgiPath.c_str());
	this->cgi_args[1] = strdup(location.cgiPath.c_str());
	this->cgi_args[2] = NULL;
}

void CgiHandler::executeCgiProcess(short& error_code)
{
    Logger::logInfo("executeCgiProcess() is running...");
    if (this->cgi_args[0] == NULL || this->cgi_args[1] == NULL) {
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
    this->cgi_pid = fork();
    if (this->cgi_pid == 0)
    {
        Logger::logInfo("Fork successful.");
        dup2(cgi_input_pipe[0], STDIN_FILENO);
        dup2(cgi_output_pipe[1], STDOUT_FILENO);
        close(cgi_input_pipe[0]);
        close(cgi_input_pipe[1]);
        close(cgi_output_pipe[0]);
        close(cgi_output_pipe[1]);
        this->status_exit = execve(this->cgi_args[0], this->cgi_args, this->cgi_envs);
        exit(this->status_exit);
    } else if (this->cgi_pid > 0)
    {} else {
        Logger::logError("Fork failed");
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

unsigned int fromHexToDecimal(const std::string& hexString)
{
	unsigned int decimalValue;
	std::stringstream stream;
	stream << hexString;
	stream >> std::hex >> decimalValue;
	return (decimalValue);
}

template <typename T>
std::string toString(const T value)
{
    std::stringstream stream;
    stream << value;
    return stream.str();
}

std::string CgiHandler::urlDecode(std::string& encodedUrl)
{
    size_t percentPos = encodedUrl.find("%");
    while (percentPos != std::string::npos) {
        if (encodedUrl.length() < percentPos + 2)
            break;
        char decodedChar = fromHexToDecimal(encodedUrl.substr(percentPos + 1, 2));
        encodedUrl.replace(percentPos, 3, toString(decodedChar));
        percentPos = encodedUrl.find("%");
    }
    return (encodedUrl);
}

std::string CgiHandler::extractPathInfoFromExtension(std::string& path, std::vector<std::string> extensions)
{
    std::string extractedPathInfo;
    size_t      extensionPos, queryPos;

    for (std::vector<std::string>::iterator extensionIter = extensions.begin(); extensionIter != extensions.end(); extensionIter++)
    {
        extensionPos = path.find(*extensionIter);
        if (extensionPos != std::string::npos)
            break;
    }
    if (extensionPos == std::string::npos)
        return "";
    if (extensionPos + 3 >= path.size())
        return "";
    extractedPathInfo = path.substr(extensionPos + 3, path.size());
    if (!extractedPathInfo[0] || extractedPathInfo[0] != '/')
        return "";
    queryPos = extractedPathInfo.find("?");
    return (queryPos == std::string::npos ? extractedPathInfo : extractedPathInfo.substr(0, queryPos));
}

short CgiHandler::exec(const ServerConfig::Location& location, HttpRequestParser request)
{
    Logger::logInfo("CGI execution is running...");
    setupCgiEnvironment(request, location);
    prepareCgiExecutionEnv(request, location);
    if (cgi_args[0] == NULL) {
        Logger::logError("CGI execution failed: Invalid arguments");
        return 500;
    }
    short error_code = 0;
    executeCgiProcess(error_code);
    return error_code;
}

bool CgiHandler::isCGIExtension(const std::string& localPath) {
    const std::string pyExtension = ".py";
    size_t dotPos = localPath.rfind('.');
    if (dotPos == std::string::npos) {
        return false;
    }
    std::string extension = localPath.substr(dotPos);
    if (extension == pyExtension) {
        return true;
    }
    return false;
}

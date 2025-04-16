#pragma once

#include <poll.h>
#include <string>
#include "ServerConfig.hpp"
#include <fstream>
#include "HttpRequest.hpp"

class Connection
{
	private:
		
	public:
		Connection() : isSocket(false),
			ip(""), port(0), headerSent(false),
			keepAlive(false), responseHeader(""),
			transferred(0), closed(false) {}
		Connection(const Connection& other)
			: configs(other.configs), poll(other.poll), isSocket(other.isSocket),
			  ip(other.ip), port(other.port), headerSent(other.headerSent),
			  keepAlive(other.keepAlive), responseHeader(other.responseHeader),
			  transferred(other.transferred), closed(other.closed), 
			  requestBuffer(other.requestBuffer), cgiPoll(other.cgiPoll) {}

		Connection& operator=(const Connection& other) {
			if (this != &other) {
				configs = other.configs;
				poll = other.poll;
				cgiPoll = other.cgiPoll;
				isSocket = other.isSocket;
				ip = other.ip;
				port = other.port;
				headerSent = other.headerSent;
				keepAlive = other.keepAlive;
				responseHeader = other.responseHeader;
				transferred = other.transferred;
				closed = other.closed;
				requestBuffer = other.requestBuffer;
			}
			return *this;
		}
		std::vector<ServerConfig> configs;
		struct pollfd poll;
		bool isSocket;
		std::string ip;
		int port;
		bool headerSent;
		bool keepAlive;
		std::string responseHeader;
		std::size_t transferred;
		bool closed;
		std::vector<char> requestBuffer;
		
		std::string cgiOutput;
		struct pollfd cgiPoll;
		HttpRequestParser request;

		const ServerConfig&  getConfig(const std::string& serverName);
};
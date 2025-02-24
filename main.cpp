#include "Server.hpp"

int main(int argc, char **argv) 
{
	std::string config = "simple.conf";
	if (argc == 2)
		config = argv[1];
	signal(SIGPIPE, SIG_IGN);
	Server server(config);
    server.loop();
}
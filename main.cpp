#include "Server.hpp"

int main(int argc, char **argv) {
	std::string config = "simple.conf";
	if (argc > 1)
		config = argv[1];
	Server server(config);
    server.loop();
}
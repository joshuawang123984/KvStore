#include "server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>

void Server::start(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd,
             reinterpret_cast<sockaddr *>(&address),
             sizeof(address)) == -1)
    {
        close(server_fd);
        throw std::runtime_error("Failed to bind socket");
    }

    std::cout << "Server bound to port " << port << "\n";

    close(server_fd);
}
#include "server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>

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

    if (listen(server_fd, 10) == -1)
    {
        close(server_fd);
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "server listening on port " << port << "\n";

    sockaddr_in client_address{};
    socklen_t client_address_size = sizeof(client_address);

    int client_fd = accept(
        server_fd,
        reinterpret_cast<sockaddr *>(&client_address),
        &client_address_size);

    if (client_fd == -1)
    {
        close(server_fd);
        throw std::runtime_error("Failed to accept client");
    }

    std::cout << "client connected!\n";

    char buffer[1024];

    int bytes_received = recv(
        client_fd,
        buffer,
        sizeof(buffer) - 1,
        0);

    if (bytes_received == -1)
    {
        close(client_fd);
        close(server_fd);
        throw std::runtime_error("Failed to receive data");
    }

    buffer[bytes_received] = '\0';

    std::cout << "received: " << buffer << "\n";

    const char *response = "from server!\n";

    int bytes_sent = send(
        client_fd,
        response,
        strlen(response),
        0);

    if (bytes_sent == -1)
    {
        close(client_fd);
        close(server_fd);
        throw std::runtime_error("Failed to send data");
    }

    close(client_fd);
    close(server_fd);
}
#include "server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>

std::string Server::parseGet(std::string command)
{
    std::string variable = command;
    if (store.get(variable).has_value())
    {
        return store.get(variable).value();
    }

    return "Variable not found.";
}
std::string Server::parseSet(std::string command)
{
    size_t variableEnd = command.find_first_of(" \t\n\r");
    std::string variable = command.substr(0, variableEnd);

    std::string leftover = command.substr(variableEnd);
    size_t valueStart = leftover.find_first_not_of(" \t\n\r");

    std::string value = leftover.substr(valueStart);

    store.set(variable, value);
    return variable + " set with the value of: " + value;
}
std::string Server::parseDelete(std::string command)
{
    std::string variable = command;
    if (store.remove(variable))
    {
        return "Variable successfully deleted.";
    }

    return "Variable not found.";
}

std::string Server::parseCommand(std::string command)
{
    // clears preceding white space
    size_t first = command.find_first_not_of(" \t\n\r");
    if (first != std::string::npos)
    {
        command.erase(0, first);
    }
    else
    {
        return "Cannot process empty command.";
    }

    size_t last = command.find_last_not_of(" \t\n\r");

    if (last != std::string::npos)
    {
        command.erase(last + 1);
    }

    if (command.length() < 3)
    {
        return "Invalid command";
    }

    std::string func = command.substr(0, 3);
    for (char &c : func)
        c = std::tolower(static_cast<unsigned char>(c));

    size_t variableStart = command.substr(3).find_first_not_of(" \t\n\r");

    if (func == "get")
    {
        return parseGet(command.substr(3 + variableStart));
    }
    else if (func == "set")
    {
        return parseSet(command.substr(3 + variableStart));
    }

    if (command.length() < 6)
    {
        return "Invalid command";
    }

    func = command.substr(0, 6);
    for (char &c : func)
        c = std::tolower(static_cast<unsigned char>(c));
    variableStart = command.substr(6).find_first_not_of(" \t\n\r");

    if (func == "delete")
    {
        return parseDelete(command.substr(6 + variableStart));
    }

    return "Invalid command.";
}

void Server::start(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        close(server_fd);
        throw std::runtime_error("Failed to set socket options");
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
    std::string pending;

    while (true)
    {
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

        if (bytes_received == 0)
        {
            break;
        }

        pending.append(buffer, bytes_received);
        size_t newLine;

        while ((newLine = pending.find('\n')) != std::string::npos)
        {
            std::string command = pending.substr(0, newLine);
            pending.erase(0, newLine + 1);

            if (!command.empty() && command.back() == '\r')
            {
                command.pop_back();
            }

            if (command.find_first_not_of(" \t\r") == std::string::npos)
            {
                continue;
            }

            std::string response = parseCommand(command) + "\n";

            int bytes_sent = send(
                client_fd,
                response.c_str(),
                response.size(),
                0);

            if (bytes_sent == -1)
            {
                close(client_fd);
                close(server_fd);
                throw std::runtime_error("Failed to send data");
            }
        }
    }
    close(client_fd);
    close(server_fd);
}
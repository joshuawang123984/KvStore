#include "server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <thread>
#include <arpa/inet.h>

Server::Server(int port, bool isPrimary, const std::vector<int> &replicaPorts) : port(port), isPrimary(isPrimary)
{
    for (int replicaPort : replicaPorts)
    {
        Replica temp = {};
        temp.fd = -1;
        temp.port = replicaPort;
        replicas.push_back(temp);
    }
}

std::string Server::parseGet(std::string command)
{
    std::string variable = command;
    auto res = store.get(variable);
    if (res.has_value())
    {
        return res.value();
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

    bool success = store.set(variable, value);

    if (success && isPrimary)
    {
        for (Replica &replica : replicas)
        {
            std::lock_guard<std::mutex> lock(replicaMutex);

            if (replica.fd == -1)
            {
                connectToReplica(replica);
            }

            if (replica.fd != -1)
            {
                std::string replicationCommand = "SET " + variable + " " + value + "\n";

                int bytesSent = send(
                    replica.fd,
                    replicationCommand.c_str(),
                    replicationCommand.size(),
                    0);

                if (bytesSent == -1)
                {
                    std::cerr << "Replica connection lost (Continuing with primray).\n";
                    close(replica.fd);
                    replica.fd = -1;

                    connectToReplica(replica);
                }
            }
        }
    }

    return variable + " set with the value of: " + value;
}
std::string Server::parseDelete(std::string command)
{
    std::string variable = command;
    if (store.remove(variable))
    {
        if (isPrimary)
        {
            for (Replica &replica : replicas)
            {
                std::lock_guard<std::mutex> lock(replicaMutex);

                if (replica.fd == -1)
                {
                    connectToReplica(replica);
                }

                if (replica.fd != -1)
                {
                    std::string replicationCommand = "DELETE " + variable + "\n";

                    int bytesSent = send(
                        replica.fd,
                        replicationCommand.c_str(),
                        replicationCommand.size(),
                        0);

                    if (bytesSent == -1)
                    {
                        std::cerr << "Replica connection lost (Continuing with primray).\n";
                        close(replica.fd);
                        replica.fd = -1;

                        connectToReplica(replica);
                    }
                }
            }
        }
        return "Variable successfully deleted.";
    }
    return "Variable not found.";
}

std::string Server::parseCommand(std::string command)
{
    std::string func = command.substr(
        0,
        command.find_first_of(" \t\n\r"));

    for (char &c : func)
        c = std::tolower(static_cast<unsigned char>(c));

    size_t argumentStart =
        command.find_first_not_of(" \t\n\r", func.size());

    if (func == "get")
    {
        return parseGet(command.substr(argumentStart));
    }

    if (func == "set")
    {
        return parseSet(command.substr(argumentStart));
    }

    if (func == "delete")
    {
        return parseDelete(command.substr(argumentStart));
    }

    if (func == "sync")
    {
        store.clear();
        return "";
    }

    return "Invalid command.";
}

void Server::handleClient(int client_fd)
{
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
            return;
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
                return;
            }
        }
    }
    close(client_fd);
}

void Server::start()
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

    if (isPrimary)
    {
        for (Replica &replica : replicas)
        {
            connectToReplica(replica);
        }
    }

    while (true)
    {
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

        std::thread client_thread(
            &Server::handleClient,
            this,
            client_fd);

        client_thread.detach();
    }

    close(server_fd);
}

bool Server::connectToReplica(Replica &replica)
{
    if (replica.fd != -1)
    {
        return true;
    }
    replica.fd = socket(AF_INET, SOCK_STREAM, 0);

    if (replica.fd == -1)
    {
        std::cerr << "Failed to create replica socket.\n";
        return false;
    }

    sockaddr_in replica_address{};

    replica_address.sin_family = AF_INET;
    replica_address.sin_port = htons(replica.port);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &replica_address.sin_addr);

    if (connect(
            replica.fd,
            reinterpret_cast<sockaddr *>(&replica_address),
            sizeof(replica_address)) == -1)
    {
        std::cerr << "Failed to reconnect to replica.\n";

        close(replica.fd);
        replica.fd = -1;

        return false;
    }

    std::cout << "Connected to replica on port "
              << replica.port << "\n";

    if (!resyncReplica(replica))
    {
        close(replica.fd);
        replica.fd = -1;

        return false;
    }

    return true;
}

bool Server::resyncReplica(Replica &replica)
{
    std::vector<std::pair<std::string, std::string>> snapshot =
        store.snapshot();

    std::string syncCommand = "SYNC\n";

    if (send(
            replica.fd,
            syncCommand.c_str(),
            syncCommand.size(),
            0) == -1)
    {
        std::cerr << "Failed to send sync command.\n";
        return false;
    }

    for (const auto &[key, value] : snapshot)
    {
        std::string command =
            "SET " + key + " " + value + "\n";

        if (send(
                replica.fd,
                command.c_str(),
                command.size(),
                0) == -1)
        {
            std::cerr << "Failed to send replica snapshot.\n";
            return false;
        }
    }

    std::cout << "replica resynchronized.\n";

    return true;
}
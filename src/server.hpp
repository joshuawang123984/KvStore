#pragma once
#include "store.hpp"

class Server
{
public:
    Server(int port, bool isPrimary, int replicaPort = -1);
    void start();

private:
    std::string parseCommand(std::string command);
    std::string parseGet(std::string command);
    std::string parseSet(std::string command);
    std::string parseDelete(std::string command);

    void handleClient(int client_fd);

    KVStore store;
    int port;

    bool isPrimary;
    // might change in future to handle multiple replicaPorts
    int replicaPort;
    int replica_fd = -1;
};
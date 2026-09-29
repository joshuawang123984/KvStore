#pragma once
#include "store.hpp"
#include <vector>

struct Replica
{
    int fd = -1;
    int port;
};

class Server
{
public:
    Server(int port, bool isPrimary);
    void start();

private:
    std::string parseCommand(std::string command);
    std::string parseGet(std::string command);
    std::string parseSet(std::string command);
    std::string parseDelete(std::string command);

    void handleClient(int client_fd);

    bool connectToReplica(Replica &replica);
    bool resyncReplica(Replica &replica);

    KVStore store;
    int port;

    bool isPrimary;
    std::vector<Replica> replicas;

    std::mutex replicaMutex;
};
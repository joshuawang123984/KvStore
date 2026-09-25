#pragma once
#include "store.hpp"

class Server
{
public:
    Server(int port);
    void start();

private:
    std::string parseCommand(std::string command);
    std::string parseGet(std::string command);
    std::string parseSet(std::string command);
    std::string parseDelete(std::string command);

    void handleClient(int client_fd);

    KVStore store;
    int port;
};
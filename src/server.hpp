#pragma once
#include "store.hpp"

class Server
{
public:
    void start(int port);

private:
    std::string parseCommand(std::string command);
    std::string parseGet(std::string command);
    std::string parseSet(std::string command);
    std::string parseDelete(std::string command);

    void handleClient(int client_fd);

    KVStore store;
};
#include "server.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage:\n";
        std::cerr << "  ./server <port> primary <replica port>\n";
        std::cerr << "  ./server <port> replica\n";
        return 1;
    }

    int port = std::stoi(argv[1]);
    std::string role = argv[2];

    if (role == "primary")
    {
        if (argc != 4)
        {
            std::cerr << "Primary requires a replica port\n";
            return 1;
        }

        int replicaPort = std::stoi(argv[3]);
        Server server(port, true, replicaPort);
        server.start();
    }

    else if (role == "replica")
    {
        Server server(port, false);
        server.start();
    }

    else
    {
        std::cerr << "Role must be 'primary' or 'replica'\n";
        return 1;
    }

    return 0;
}
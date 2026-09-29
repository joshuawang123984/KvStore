#include "server.hpp"
#include <iostream>
#include <csignal>

/*
TODO:
    How to push replica structs into servers replicas vector?
    MAKE sure sets and deletes dont do anything unless server is primary. ->
    Move logic into the if primary check rather than actually doing something then checking
    Also change main to accompany this no port constructor. or maybe accept and directly push back to vector?
*/

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);

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

        std::vector<int> replicaPorts;
        for (int i = 3; i < argc; ++i)
        {
            replicaPorts.push_back(std::stoi(argv[i]));
        }

        Server server(port, true, replicaPorts);
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
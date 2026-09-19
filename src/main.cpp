#include "server.hpp"

int main()
{
    Server server;
    server.start(6379);

    return 0;
}
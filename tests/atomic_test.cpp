#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>

void runClient(int client_id)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        std::cerr << "Failed to create socket\n";
        return;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(6379);

    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        std::cerr << "Invalid server address\n";
        close(sock);
        return;
    }

    if (connect(
            sock,
            reinterpret_cast<sockaddr *>(&server_address),
            sizeof(server_address)) == -1)
    {
        std::cerr << "Failed to connect\n";
        close(sock);
        return;
    }

    for (int i = 0; i < 100; i++)
    {
        std::string key =
            "client" + std::to_string(client_id) +
            "_" + std::to_string(i);

        std::string value = "value";

        std::string command =
            "SET " + key + " " + value + "\n";

        send(sock, command.c_str(), command.size(), 0);

        char buffer[1024];

        int bytes_received = recv(
            sock,
            buffer,
            sizeof(buffer) - 1,
            0);

        if (bytes_received <= 0)
        {
            std::cerr << "Client " << client_id
                      << " failed to receive response\n";
            break;
        }

        buffer[bytes_received] = '\0';

        std::string response(buffer);

        std::string expected =
            key + " set with the value of: " + value + "\n";

        if (response != expected)
        {
            std::cerr << "Client " << client_id
                      << " received unexpected response:\n"
                      << "Expect: " << expected
                      << "Received: " << response << "\n";
            break;
        }

        std::string getCommand = "GET " + key + "\n";

        send(sock, getCommand.c_str(), getCommand.size(), 0);

        bytes_received = recv(
            sock,
            buffer,
            sizeof(buffer) - 1,
            0);

        if (bytes_received <= 0)
        {
            std::cerr << "Failed to receive get response\n";
            break;
        }

        buffer[bytes_received] = '\0';

        response = std::string(buffer);

        if (response != value + "\n")
        {
            std::cerr << "GET failed for " << key
                      << "\nExpect: " << value
                      << "\nReceived: " << response;
            break;
        }
    }

    close(sock);
}

int main()
{
    std::vector<std::thread> clients;

    for (int i = 0; i < 20; i++)
    {
        clients.emplace_back(runClient, i);
    }

    for (auto &client : clients)
    {
        client.join();
    }

    std::cout << "test complete\n";
}
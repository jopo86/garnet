#include <iostream>

#include <Garnet.h>

using namespace Garnet;

std::vector<Address> clientAddresses = {};

void receive(void* buffer, int bufferSize, int actualSize, Address clientAddr)
{
    if (std::find(clientAddresses.begin(), clientAddresses.end(), clientAddr) == clientAddresses.end())
    {
        clientAddresses.push_back(clientAddr);
    }

    if (strcmp((const char*)buffer, "!quit") == 0)
    {
        std::cout << "Client (" << clientAddr.host << ":" << clientAddr.port << ") left the chat.\n";
        clientAddresses.erase(std::remove(clientAddresses.begin(), clientAddresses.end(), clientAddr), clientAddresses.end());
        delete buffer;
        return;
    }

    std::string msg = "Client (" + clientAddr.host + ":" + std::to_string(clientAddr.port) + "): " + std::string((const char*)buffer);
    std::cout << msg << "\n";
    ServerUDP& server = *((ServerUDP*)GetUserPtr());
    for (const Address& addr : clientAddresses)
    {
        if (clientAddr == addr) continue;
        server.send((void*)msg.c_str(), strlen(msg.c_str()), addr);
    }

    delete buffer;
}

int main()
{
    std::cout << "SERVER\n\n";

    Garnet::Init(true);
    ServerUDP server(Address{
        .host = "127.0.0.1",
        .port = 55555
    });
    SetUserPtr(&server);

    server.setReceiveCallback(receive);
    server.open();

    char buffer[256] = "Server: ";
    while (server.isOpen())
    {

        std::cin.getline(buffer + 8, sizeof(buffer) - 8);

        for (Address addr : clientAddresses)
        {
            server.send(buffer, sizeof(buffer), addr);
        }

        if (strcmp(buffer, "Server: !quit") == 0) break;
    }

    server.close();
    Garnet::Terminate();

    return 0;
}

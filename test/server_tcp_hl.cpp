#include <iostream>

#include <Garnet.h>

using namespace Garnet;

void receive(void* buffer, int bufferSize, int actualSize, Address clientAddr)
{
    std::string msg = "Client (" + clientAddr.IP + ":" + std::to_string(clientAddr.port) + "): " + std::string((const char*)buffer);
    std::cout << msg << "\n";
    ServerTCP& server = *((ServerTCP*)GetUserPtr());
    for (const Address& addr : server.getClientAddresses())
    {
        if (clientAddr == addr) continue;
        server.send(addr, (void*)msg.c_str(), msg.length());
    }
    delete buffer;
}

int main()
{
    std::cout << "SERVER\n\n";

    Garnet::Init(true);
    ServerTCP server(Address{
        .IP = "127.0.0.1",
        .port = 55555
    });
    SetUserPtr(&server);

    server.setReceiveCallback(receive);
    server.open(1);

    char buffer[256] = "Server: ";
    while (server.isOpen())
    {
        std::cin.getline(buffer + 8, sizeof(buffer) - 8);

        if (strcmp(buffer, "Server: !quit") == 0) break;

        for (const Address& addr : server.getClientAddresses())
        {
            server.send(addr, buffer, sizeof(buffer));
        }
    }

    server.close();
    Garnet::Terminate();

    return 0;
}
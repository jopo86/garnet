#include <iostream>

#include <Garnet.h>

using namespace Garnet;

void receive(void* buffer, int bufferSize, int actualSize, Address clientAddr)
{
    std::string msg = "Client (" + clientAddr.IP + ":" + std::to_string(clientAddr.port) + "): " + std::string((const char*)buffer);
    std::cout << msg << "\n";
    ServerUDP& server = *((ServerUDP*)GetUserPtr());
    server.send((void*)"Server received message", 24, clientAddr);
    delete buffer;
}

int main()
{
    std::cout << "SERVER\n\n";

    Garnet::Init(true);
    ServerUDP server(Address{
        .IP = "127.0.0.1",
        .port = 55555
    });
    SetUserPtr(&server);

    server.setReceiveCallback(receive);
    server.open();

    char buffer[256] = "Server: ";
    std::cin.get();

    server.close();
    Garnet::Terminate();

    return 0;
}

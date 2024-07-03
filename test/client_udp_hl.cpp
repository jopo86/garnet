#include <iostream>

#include <Garnet.h>

using namespace Garnet;

void receive(void* buffer, int bufferSize, int actualSize, Address serverAddr)
{
    std::cout << (const char*)buffer << "\n";
    delete buffer;
}

int main()
{
    std::cout << "CLIENT\n\n";

    Garnet::Init(true);
    ClientUDP client;
    client.start();
    SetUserPtr(&client);
    client.setReceiveCallback(receive);

    char buffer[256] = "";
    while (client.isRunning())
    {
        std::cin.getline(buffer, sizeof(buffer));

        if (strcmp(buffer, "!quit") == 0) break;
        client.send(buffer, sizeof(buffer), Address { .IP = "127.0.0.1", .port = 55555 } );
    }

    client.stop();
    Garnet::Terminate();

    return 0;
}

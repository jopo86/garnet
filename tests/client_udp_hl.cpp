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
    ClientUDP client('c');
    SetUserPtr(&client);
    client.setReceiveCallback(receive);

    char buffer[256] = "";
    while (client.isConnected())
    {
        std::cin.getline(buffer, sizeof(buffer));

        if (strcmp(buffer, "!quit") == 0) break;
        client.send(buffer, sizeof(buffer), Address { .host = "127.0.0.1", .port = 55555 } );
    }

    client.disconnect();
    Garnet::Terminate();

    return 0;
}

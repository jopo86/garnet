#include <iostream>

#include <Garnet.h>

using namespace Garnet;

void receive(void* buffer, int bufferSize, int actualSize, Address serverAddr)
{
    if (strcmp((char*)buffer, "Server: !quit") == 0)
    {
        std::cout << "Server disconnected.\n";
        delete buffer;
        exit(0);
    }
    std::cout << std::string((char*)buffer, bufferSize) << "\n";
    delete buffer;
}

int main()
{
    std::cout << "CLIENT (server won't see you until you send a message)\n\n";

    Garnet::Init(true);
    ClientUDP client('c');
    SetUserPtr(&client);
    client.setReceiveCallback(receive);

    char buffer[256] = "";
    while (client.isConnected())
    {
        std::cin.getline(buffer, sizeof(buffer));

        client.send(buffer, sizeof(buffer), Address{ .host = "127.0.0.1", .port = 55555 } );
        if (strcmp(buffer, "!quit") == 0) break;
    }

    client.disconnect();
    Garnet::Terminate();

    return 0;
}

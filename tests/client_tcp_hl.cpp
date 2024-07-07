#include <iostream>

#include <Garnet.h>

using namespace Garnet;

void receive(void* data, int size, int actualSize)
{
    std::cout << std::string((char*)data, size) << "\n";
    delete data;
}

int main()
{
    std::cout << "CLIENT\n\n";

    Garnet::Init(true);
    ClientTCP client('c');
    client.connect(Address{
        .host = "127.0.0.1",
        .port = 55555
    });
    SetUserPtr(&client);
    client.setReceiveCallback(receive);

    char buffer[256] = "";
    while (client.isConnected())
    {
        std::cin.getline(buffer, sizeof(buffer));

        if (strcmp(buffer, "!quit") == 0) break;
        client.send(buffer, sizeof(buffer));
    }

    client.disconnect();
    Garnet::Terminate();

    return 0;
}

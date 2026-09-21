#include <iostream>

#include <garnet.h>

using namespace garnet;

void receive(void* data, int size, int actual_size)
{
    if (strcmp((char*)data, "Server: !quit") == 0)
    {
        std::cout << "Server disconnected.\n";
        delete[] (char*)data;
        exit(0);
    }
    std::cout << std::string((char*)data, actual_size) << "\n";
    delete[] (char*)data;
}

int main()
{
    std::cout << "CLIENT\n\n";

    garnet::init(true);
    ClientTcp client('c');
    client.connect(Address{
        .host = "127.0.0.1",
        .port = 55555
    });
    set_user_ptr(&client);
    client.set_receive_callback(receive);

    char buffer[256] = "";
    while (client.is_connected())
    {
        std::cin.getline(buffer, sizeof(buffer));

        if (strcmp(buffer, "!quit") == 0) break;
        client.send(buffer, sizeof(buffer));
    }

    client.disconnect();
    garnet::terminate();

    return 0;
}

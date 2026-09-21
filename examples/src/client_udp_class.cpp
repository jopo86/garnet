#include <iostream>

#include <garnet.h>

using namespace garnet;

void receive(void* buffer, int buffer_size, int actual_size, Address server_addr)
{
    if (strcmp((char*)buffer, "Server: !quit") == 0)
    {
        std::cout << "Server disconnected.\n";
        delete[] (char*)buffer;
        exit(0);
    }
    std::cout << std::string((char*)buffer, actual_size) << "\n";
    delete[] (char*)buffer;
}

int main()
{
    std::cout << "CLIENT (server won't see you until you send a message)\n\n";

    garnet::init(true);
    ClientUdp client('c');
    set_user_ptr(&client);
    client.set_receive_callback(receive);

    char buffer[256] = "";
    while (client.is_connected())
    {
        std::cin.getline(buffer, sizeof(buffer));

        client.send(buffer, sizeof(buffer), Address{ .host = "127.0.0.1", .port = 55555 } );
        if (strcmp(buffer, "!quit") == 0) break;
    }

    client.disconnect();
    garnet::terminate();

    return 0;
}

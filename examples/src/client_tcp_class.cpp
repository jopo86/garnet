#include <iostream>

#include <garnet/garnet.hpp>

using namespace garnet;

void receive(void* data, int size, int actual_size)
{
    std::string msg((char*)data, actual_size < size ? actual_size : size);
    if (msg == "Server: !quit")
    {
        std::cout << "Server disconnected.\n";
        delete[] (char*)data;
        exit(0);
    }
    std::cout << msg << "\n";
    delete[] (char*)data;
}

void server_disconnected()
{
    std::cout << "Lost connection to server." << std::endl;
    exit(0);
}

int main()
{
    std::cout << "CLIENT\n\n";

    garnet::init(true);
    ClientTcp client;
    client.connect(Address{
        .host = "127.0.0.1",
        .port = 55555
    });
    set_user_ptr(&client);
    client.set_receive_callback(receive);
    client.set_disconnect_callback(server_disconnected);

    char buffer[256] = "";
    while (client.is_connected())
    {
        std::cin.getline(buffer, sizeof(buffer));

        if (strcmp(buffer, "!quit") == 0) break;
        client.send(buffer, (int)strlen(buffer));
    }

    client.disconnect();
    garnet::terminate();

    return 0;
}

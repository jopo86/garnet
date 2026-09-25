#include <iostream>

#include <garnet/garnet.hpp>

using namespace garnet;

void receive(void* data, int size, int actual_size, Address client_addr)
{
    std::string msg = "Client (" + client_addr.host + ":" + std::to_string(client_addr.port) + "): " + std::string((char*)data, actual_size < size ? actual_size : size);
    std::cout << msg << "\n";
    ServerTcp& server = *((ServerTcp*)get_user_ptr());
    for (const Address& addr : server.get_client_addresses())
    {
        if (client_addr == addr) continue;
        server.send((void*)msg.c_str(), (int)strlen(msg.c_str()), addr);
    }
    delete[] (char*)data;
}

void client_connected(Address client_addr)
{
    std::string msg = "Client (" + client_addr.host + ":" + std::to_string(client_addr.port) + ") connected.";
    std::cout << msg << "\n";
    ServerTcp& server = *((ServerTcp*)get_user_ptr());
    for (const Address& addr : server.get_client_addresses())
    {
        if (client_addr == addr) continue;
        server.send((void*)msg.c_str(), (int)strlen(msg.c_str()), addr);
    }
}

void client_disconnected(Address client_addr)
{
    std::string msg = "Client (" + client_addr.host + ":" + std::to_string(client_addr.port) + ") disconnected.";
    std::cout << msg << "\n";
    ServerTcp& server = *((ServerTcp*)get_user_ptr());
    for (const Address& addr : server.get_client_addresses())
    {
        if (client_addr == addr) continue;
        server.send((void*)msg.c_str(), (int)strlen(msg.c_str()), addr);
    }
}

int main()
{
    std::cout << "SERVER\n\n";

    garnet::init(true);
    ServerTcp server(Address{
        .host = "127.0.0.1",
        .port = 55555
    });
    set_user_ptr(&server);

    server.set_receive_callback(receive);
    server.set_client_connect_callback(client_connected);
    server.set_client_disconnect_callback(client_disconnected);
    server.open();

    char buffer[256] = "Server: ";
    while (server.is_open())
    {
        std::cin.getline(buffer + 8, sizeof(buffer) - 8);

        for (const Address& addr : server.get_client_addresses())
        {
            server.send(buffer, (int)strlen(buffer), addr);
        }

        if (strcmp(buffer, "Server: !quit") == 0) break;
    }

    server.close();
    garnet::terminate();

    return 0;
}

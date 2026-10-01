#include <iostream>
#include <algorithm>

#include <garnet/garnet.hpp>

using namespace gnet;

std::vector<Address> client_addresses = {};

void receive(void* buffer, int buffer_size, int actual_size, Address client_addr)
{
    if (std::find(client_addresses.begin(), client_addresses.end(), client_addr) == client_addresses.end())
    {
        client_addresses.push_back(client_addr);
    }

    std::string text((const char*)buffer, actual_size < buffer_size ? actual_size : buffer_size);
    if (text == "!quit")
    {
        std::cout << "Client (" << client_addr.host << ":" << client_addr.port << ") left the chat.\n";
        client_addresses.erase(std::remove(client_addresses.begin(), client_addresses.end(), client_addr), client_addresses.end());
        delete[] (char*)buffer;
        return;
    }

    std::string msg = "Client (" + client_addr.host + ":" + std::to_string(client_addr.port) + "): " + text;
    std::cout << msg << "\n";
    ServerUdp& server = *((ServerUdp*)get_user_ptr());
    for (const Address& addr : client_addresses)
    {
        if (client_addr == addr) continue;
        server.send((void*)msg.c_str(), (int)strlen(msg.c_str()), addr);
    }

    delete[] (char*)buffer;
}

int main()
{
    std::cout << "SERVER\n\n";

    gnet::init(true);
    ServerUdp server(Address{
        .host = "127.0.0.1",
        .port = 55555
    });
    set_user_ptr(&server);

    server.set_receive_callback(receive);
    server.open();

    char buffer[256] = "Server: ";
    while (server.is_open())
    {

        std::cin.getline(buffer + 8, sizeof(buffer) - 8);

        for (Address addr : client_addresses)
        {
            server.send(buffer, (int)strlen(buffer), addr);
        }

        if (strcmp(buffer, "Server: !quit") == 0) break;
    }

    server.close();
    gnet::terminate();

    return 0;
}

#include <iostream>
#include <algorithm>
#include <mutex>

#include <garnet/garnet.hpp>

using namespace gnet;

int main()
{
    std::cout << "SERVER\n\n";

    gnet::init(true);
    ServerUdp server(Address{
        .host = "127.0.0.1",
        .port = 55555
    });

    // UDP has no connections, so the server learns about clients from the messages they send.
    // The receive callback runs on a background thread while main() also reads the list, so it needs a lock.
    std::vector<Address> client_addresses;
    std::mutex client_addresses_mtx;

    // the lambda captures these locals by reference, which is safe because
    // server.close() below stops the receive thread before they go out of scope
    server.set_receive_callback([&](void* buffer, int buffer_size, int actual_size, Address client_addr)
    {
        std::string text((const char*)buffer, actual_size < buffer_size ? actual_size : buffer_size);
        delete[] (char*)buffer;

        std::lock_guard lock(client_addresses_mtx);
        if (std::find(client_addresses.begin(), client_addresses.end(), client_addr) == client_addresses.end())
        {
            client_addresses.push_back(client_addr);
        }

        if (text == "!quit")
        {
            std::cout << "Client (" << client_addr.host << ":" << client_addr.port << ") left the chat.\n";
            client_addresses.erase(std::remove(client_addresses.begin(), client_addresses.end(), client_addr), client_addresses.end());
            return;
        }

        std::string msg = "Client (" + client_addr.host + ":" + std::to_string(client_addr.port) + "): " + text;
        std::cout << msg << "\n";
        for (const Address& addr : client_addresses)
        {
            if (client_addr == addr) continue;
            server.send((void*)msg.c_str(), (int)msg.size(), addr);
        }
    });
    server.open();

    char buffer[256] = "Server: ";
    while (server.is_open())
    {
        std::cin.getline(buffer + 8, sizeof(buffer) - 8);

        {
            std::lock_guard lock(client_addresses_mtx);
            for (const Address& addr : client_addresses)
            {
                server.send(buffer, (int)strlen(buffer), addr);
            }
        }

        if (strcmp(buffer, "Server: !quit") == 0) break;
    }

    server.close();
    gnet::terminate();

    return 0;
}

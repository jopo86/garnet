#include <iostream>

#include <garnet/garnet.hpp>

using namespace gnet;

int main()
{
    std::cout << "SERVER\n\n";

    gnet::init(true);
    ServerTcp server(Address{
        .host = "127.0.0.1",
        .port = 55555
    });

    // sends msg to every client except the one it came from
    auto broadcast = [&server](const std::string& msg, Address except)
    {
        for (const Address& addr : server.get_client_addresses())
        {
            if (addr == except) continue;
            server.send((void*)msg.c_str(), (int)msg.size(), addr);
        }
    };

    // the lambdas capture `server` and `broadcast` by reference, which is safe because
    // server.close() below stops the callback threads before either goes out of scope
    server.set_receive_callback([&](void* data, int size, int actual_size, Address client_addr)
    {
        std::string msg = "Client (" + client_addr.host + ":" + std::to_string(client_addr.port) + "): " + std::string((char*)data, actual_size < size ? actual_size : size);
        std::cout << msg << "\n";
        broadcast(msg, client_addr);
        delete[] (char*)data;
    });

    server.set_client_connect_callback([&](Address client_addr)
    {
        std::string msg = "Client (" + client_addr.host + ":" + std::to_string(client_addr.port) + ") connected.";
        std::cout << msg << "\n";
        broadcast(msg, client_addr);
    });

    server.set_client_disconnect_callback([&](Address client_addr)
    {
        std::string msg = "Client (" + client_addr.host + ":" + std::to_string(client_addr.port) + ") disconnected.";
        std::cout << msg << "\n";
        broadcast(msg, client_addr);
    });

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
    gnet::terminate();

    return 0;
}

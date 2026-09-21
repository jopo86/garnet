#include <iostream>

#include <garnet.h>

using namespace garnet;

int main()
{
    std::cout << "SERVER\n\n";

    garnet::init(true);
    Socket server_socket(Protocol::Udp);
    server_socket.bind(Address{
        .host = "127.0.0.1",
        .port = 55555
    });

    std::cout << "CHAT STARTED ----- enter '!quit' to exit\n\n";
    char buffer[256];
    Address client_addr;
    while (true)
    {
        Address recv_addr;
        bool received = server_socket.receive_from(buffer, sizeof(buffer), &recv_addr);
        if (received)
        {
            client_addr = recv_addr;
            if (strcmp(buffer, "!quit") == 0)
            {
                std::cout << "Client left the chat.\n";
                break;
            }
            else std::cout << "Client (" << client_addr.host << ":" << client_addr.port << "): " << buffer << "\n";
        }
        else continue;

        std::cout << "Server: ";
        std::cin.getline(buffer, sizeof(buffer));

        if (!server_socket.send_to(buffer, sizeof(buffer), client_addr)) std::cout << "MESSAGE NOT SENT\n";
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Server left the chat.\n";
			break;
		}
    }

    server_socket.close();
    garnet::terminate();
    return 0;
}

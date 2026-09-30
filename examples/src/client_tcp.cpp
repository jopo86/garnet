#include <iostream>

#include <garnet/garnet.hpp>

using namespace gnet;

int main()
{
    std::cout << "CLIENT\n\n";

    gnet::init(true);
    Socket client_socket(Protocol::Tcp);
    std::cout << "Connecting to server...\n";
    client_socket.connect(Address{
        .host = "127.0.0.1",
        .port = 55555
    });

    std::cout << "CHAT STARTED ----- enter '!quit' to exit\n\n";
    char buffer[256];
    while (true)
    {
        std::cout << "Client: ";
        std::cin.getline(buffer, sizeof(buffer));

        if (!client_socket.send(buffer, (int)strlen(buffer) + 1)) std::cout << "MESSAGE NOT SENT\n";
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Client left the chat.\n";
			break;
		}

        int received = client_socket.receive(buffer, sizeof(buffer) - 1);
        if (received <= 0)
        {
            std::cout << "Server disconnected.\n";
            break;
        }
        buffer[received] = '\0';
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Server left the chat.\n";
			break;
		}
        else std::cout << "Server: " << buffer << "\n";
    }

    client_socket.close();
    gnet::terminate();
    return 0;
}

#include <iostream>

#include <garnet.h>

using namespace garnet;

int main()
{
    std::cout << "SERVER\n\n";

    garnet::init(true);
    Socket server_socket(Protocol::Tcp);
    server_socket.bind(Address{
        .host = "127.0.0.1",
        .port = 55555
    });
    server_socket.listen(5);
    std::cout << "Listening for connection...\n";
    Socket accept_socket = server_socket.accept();
    std::cout << "Connected with client (IP: " << accept_socket.get_address().host << ", port " << accept_socket.get_address().port << ")\n\n";

    std::cout << "CHAT STARTED ----- enter '!quit' to exit\n\n";
    char buffer[256];
    while (true)
    {
        int received = accept_socket.receive(buffer, sizeof(buffer) - 1);
        if (received <= 0)
        {
            std::cout << "Client disconnected.\n";
            break;
        }
        buffer[received] = '\0';
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Client left the chat.\n";
			break;
		}
        else std::cout << "Client: " << buffer << "\n";

        std::cout << "Server: ";
        std::cin.getline(buffer, sizeof(buffer));

        if (!accept_socket.send(buffer, strlen(buffer) + 1)) std::cout << "MESSAGE NOT SENT\n";
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Server left the chat.\n";
			break;
		}
    }

    server_socket.close();
    accept_socket.close();
    garnet::terminate();
    return 0;
}

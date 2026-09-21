#include <iostream>

#include <garnet.h>

using namespace garnet;

int main()
{
    std::cout << "CLIENT\n\n";

    garnet::init(true);
    Socket client_socket(Protocol::Udp);

    Address server_addr{ .host = "127.0.0.1", .port = 55555 };

    std::cout << "CHAT STARTED ----- enter '!quit' to exit\n\n";
    char buffer[256];
    while (true)
    {
        std::cout << "Client: ";
        std::cin.getline(buffer, sizeof(buffer));

        if (!client_socket.send_to(buffer, sizeof(buffer), server_addr)) std::cout << "MESSAGE NOT SENT\n";
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Client left the chat.\n";
			break;
		}

        bool received = false;
        bool should_break = false;
        while (!received)
        {
            received = client_socket.receive_from(buffer, sizeof(buffer), nullptr);
            if (received)
            {
                if (strcmp(buffer, "!quit") == 0)
                {
                    std::cout << "Server left the chat.\n";
                    should_break = true;
                    break;
                }
                else std::cout << "Server: " << buffer << "\n";
            }
        }
        if (should_break) break;
    }

    client_socket.close();
    garnet::terminate();
    return 0;
}

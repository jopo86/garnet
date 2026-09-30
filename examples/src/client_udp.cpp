#include <iostream>

#include <garnet/garnet.hpp>

using namespace gnet;

int main()
{
    std::cout << "CLIENT\n\n";

    gnet::init(true);
    Socket client_socket(Protocol::Udp);

    Address server_addr{ .host = "127.0.0.1", .port = 55555 };

    std::cout << "CHAT STARTED ----- enter '!quit' to exit\n\n";
    char buffer[256];
    while (true)
    {
        std::cout << "Client: ";
        std::cin.getline(buffer, sizeof(buffer));

        if (!client_socket.send_to(buffer, (int)strlen(buffer) + 1, server_addr)) std::cout << "MESSAGE NOT SENT\n";
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Client left the chat.\n";
			break;
		}

        bool received = false;
        bool should_break = false;
        while (!received)
        {
            int num_bytes = client_socket.receive_from(buffer, sizeof(buffer) - 1, nullptr);
            received = num_bytes >= 0;
            if (received)
            {
                buffer[num_bytes] = '\0';
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
    gnet::terminate();
    return 0;
}

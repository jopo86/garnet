#include <iostream>

#include <Garnet.h>

using namespace Garnet;

int main()
{
    std::cout << "CLIENT\n\n";

    Garnet::Init(true);
    Socket clientSocket(Protocol::UDP);
    float start = time(nullptr);

    Address serverAddr{ .IP = "127.0.0.1", .port = 55555 };

    std::cout << "CHAT STARTED ----- enter '!quit' to exit\n\n";
    char buffer[256];
    while (true)
    {
        std::cout << "Client: ";
        std::cin.getline(buffer, sizeof(buffer));

        if (!clientSocket.sendTo(buffer, sizeof(buffer), serverAddr)) std::cout << "MESSAGE NOT SENT\n";
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Client left the chat.\n";
			break;
		}

        bool received = false;
        bool shouldBreak = false;
        while (!received)
        {
            received = clientSocket.receiveFrom(buffer, sizeof(buffer), nullptr);
            if (received)
            {
                if (strcmp(buffer, "!quit") == 0)
                {
                    std::cout << "Server left the chat.\n";
                    shouldBreak = true;
                    break;
                }
                else std::cout << "Server: " << buffer << "\n";
            }
        }
        if (shouldBreak) break;
    }

    clientSocket.close();
    Garnet::Terminate();
    return 0;
}

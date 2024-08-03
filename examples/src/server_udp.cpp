#include <iostream>

#include <Garnet.h>

using namespace Garnet;

int main()
{
    std::cout << "SERVER\n\n";

    Garnet::Init(true);
    Socket serverSocket(Protocol::UDP);
    serverSocket.bind(Address{
        .host = "127.0.0.1",
        .port = 55555
    });

    std::cout << "CHAT STARTED ----- enter '!quit' to exit\n\n";
    char buffer[256];
    Address clientAddr;
    while (true)
    {
        Address recvAddr;
        bool received = serverSocket.receiveFrom(buffer, sizeof(buffer), &recvAddr);
        if (received)
        {
            clientAddr = recvAddr;
            if (strcmp(buffer, "!quit") == 0)
            {
                std::cout << "Client left the chat.\n";
                break;
            }
            else std::cout << "Client (" << clientAddr.host << ":" << clientAddr.port << "): " << buffer << "\n";
        }
        else continue;

        std::cout << "Server: ";
        std::cin.getline(buffer, sizeof(buffer));

        if (!serverSocket.sendTo(buffer, sizeof(buffer), clientAddr)) std::cout << "MESSAGE NOT SENT\n";
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Server left the chat.\n";
			break;
		}
    }

    serverSocket.close();
    Garnet::Terminate();
    return 0;
}

#include <iostream>

#include <Garnet.h>

using namespace Garnet;

int main()
{
    std::cout << "CLIENT\n\n";

    Garnet::Init(true);
    Socket clientSocket(Protocol::TCP);
    std::cout << "Connecting to server...\n";
    clientSocket.connect(Address{
        .IP = "josh-posner.com", // these should be the same as server address
        .port = 8080
    });

    std::cout << "CHAT STARTED ----- enter '!quit' to exit\n\n";
    char buffer[256];
    while (true)
    {
        std::cout << "Client: ";
        std::cin.getline(buffer, sizeof(buffer));

        if (!clientSocket.send(buffer, sizeof(buffer))) std::cout << "MESSAGE NOT SENT\n";
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Client left the chat.\n";
			break;
		}

        clientSocket.receive(buffer, sizeof(buffer));
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Server left the chat.\n";
			break;
		}
        else std::cout << "Server: " << buffer << "\n";
    }

    clientSocket.close();
    Garnet::Terminate();
    return 0;
}

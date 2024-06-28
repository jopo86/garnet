#include <iostream>

#include <Garnet.h>

using namespace Garnet;

int main()
{
    std::cout << "CLIENT\n\n";

    int port = 55555;

    Garnet::Init(true);
    Socket clientSocket(Protocol::TCP);
    float start = time(nullptr);
    std::cout << "Connecting to server...\n";
    clientSocket.connect(Address{
        .IP = "127.0.0.1", // these should be the same as server address
        .port = 55555
    });
    float end = time(nullptr);
    std::cout << "Connected to server in " + std::to_string(end - start) + "s\n\n";

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

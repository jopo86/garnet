#include <iostream>

#include <Garnet.h>

using namespace Garnet;

SOCKADDR_IN addr_gtow(Garnet::Address addr);
Garnet::Address addr_wtog(SOCKADDR_IN addr);

int main()
{
    std::cout << "SERVER\n\n";

    Garnet::Init(true);
    Socket serverSocket(Protocol::TCP);
    serverSocket.bind(Address{
        .IP = "127.0.0.1",
        .port = 55555
    });
    serverSocket.listen(1);
    std::cout << "Listening for connection...\n";
    Socket acceptSocket = serverSocket.accept();
    std::cout << "Connected with a client\n\n";

    std::cout << "CHAT STARTED ----- enter '!quit' to exit\n\n";
    char buffer[256];
    while (true)
    {
        acceptSocket.receive(buffer, sizeof(buffer));
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Client left the chat.\n";
			break;
		}
        else std::cout << "Client: " << buffer << "\n";

        std::cout << "Server: ";
        std::cin.getline(buffer, sizeof(buffer));

        if (!acceptSocket.send(buffer, sizeof(buffer))) std::cout << "MESSAGE NOT SENT\n";
        if (strcmp(buffer, "!quit") == 0)
		{
			std::cout << "Server left the chat.\n";
			break;
		}
    }

    serverSocket.close();
    acceptSocket.close();
    Garnet::Terminate();
    return 0;
}

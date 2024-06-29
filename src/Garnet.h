#include <string>
#include <unordered_map>
#include <thread>

#include <winsock2.h>
#include <ws2tcpip.h>

#define GNET_VERSION_MAJOR  0
#define GNET_VERSION_MINOR  0
#define GNET_VERSION_PATCH  1

#define GNET_DEV            true
#define GNET_ALPHA          false
#define GNET_BETA           false
#define GNET_STABLE         false

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define GNET_OS_WINDOWS
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#define GNET_OS_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
#define GNET_OS_MAC
#else
#define GNET_OS_UNKNOWN
#endif

typedef unsigned short ushort;

namespace Garnet
{
    int GetVersionMajor();
    int GetVersionMinor();
    int GetVersionPatch();
    std::string GetVersionString();

    bool Init(bool printErrors = false);
    void Terminate();

    const std::string& GetLastError();

    enum class Protocol
    {
        Null, TCP, UDP
    };

    struct Address
    {
        std::string IP;
        ushort port;
    };

    class Socket
    {
    public:
        Socket();
        Socket(Protocol protocol, bool* success = nullptr);

        void bind(Address serverAddress, bool* success = nullptr);
        void listen(int maxClients, bool* success = nullptr);
        Socket accept(bool* success = nullptr);
        void connect(Address serverAddress, bool* success = nullptr);

        int send(void* data, int size, bool* success = nullptr);
        int receive(void* buffer, int bufferSize);
        int sendTo(void* data, int size, Address to, bool* success = nullptr);
        int receiveFrom(void* buffer, int bufferSize, Address* from, bool* success = nullptr);

        void close();

        const Address& getAddress() const;
        const Protocol& getProtocol() const;
        bool isOpen() const;

    private:
        Address m_addr;
        Protocol m_proto;

        SOCKET m_wsSocket;
        SOCKADDR_IN m_wsAddr;
        int m_wsAddrSize;

        bool m_open;
    };

    class ServerTCP
    {
    public:
        ServerTCP();
        ServerTCP(Address serverAddress, bool* success = nullptr);

        void open(int maxClients, int bufferSize = 256, bool* success = nullptr);
        void send(int clientIndex, bool* success = nullptr);
        void send(Address clientAddress, bool* success = nullptr);
        void close(bool* success = nullptr);
        
        bool isOpen() const;

    private:
        Address m_addr;
        Socket m_socket;

        std::atomic<int> m_bufSize;

        std::atomic<int> m_nClients;
        std::unordered_map<int, Socket> m_clientIdxs;
        std::unordered_map<Address, Socket> m_clients;

        std::atomic<bool> m_open;

        void accept(); // accept() and add to maps while true until error (from closure)
        void receive(Socket& acceptedSocket, int clientIdx); // receive() and callback while true until error (from closure)
        std::thread m_accepting;
        std::vector<std::thread> m_receivings;


        void (*m_pReceiveCallbackIdx)(void* buffer, int bufferSize, int actualSize, int clientIndex);
        void (*m_pReceiveCallbackAddr)(void* buffer, int bufferSize, int actualSize, Address from);
    };

    class ServerUDP
    {
    public:

    private:

    };

    class ClientTCP
    {
    public:
        ClientTCP();
        
        void connect(Address serverAddress);


    private:

    };

    class ClientUDP
    {
    public:

    private:

    };
};

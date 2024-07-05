#include <functional>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <list>

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

    void SetUserPtr(void* ptr);
    void* GetUserPtr();


    enum class Protocol
    {
        Null, TCP, UDP
    };

    struct Address
    {
        std::string IP;
        ushort port;

        void operator=(const Address& other);
        bool operator==(const Address& other) const;
    };

    std::string HostnameToIP(const std::string& hostname, bool* success = nullptr);
};

namespace std
{
    template <>
    struct hash<Garnet::Address>
    {
        size_t operator()(const Garnet::Address& addr) const 
        {
            return hash<string>()(addr.IP) ^ (hash<int>()(addr.port) << 1);
        }
    };
};

namespace Garnet
{
    class Socket
    {
    public:
        Socket();
        Socket(Protocol protocol, bool* success = nullptr);

        void bind(Address serverAddress, bool* success = nullptr);
        void listen(int backlog, bool* success = nullptr);
        Socket accept(bool* success = nullptr);
        void connect(Address serverAddress, bool* success = nullptr);

        int send(void* data, int size, bool* success = nullptr);
        int receive(void* buffer, int bufferSize, bool* success = nullptr);
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

        void open(int backlog = 10, bool* success = nullptr);
        void send(void* data, int size, Address clientAddress, bool* success = nullptr);
        void close(bool* success = nullptr);
        
        bool isOpen() const;
        int getBufferSize() const;
        int getNumClients() const;
        const Socket& getClientAcceptedSocket(Address clientAddress);
        const std::list<Address>& getClientAddresses();
        const std::unordered_map<Address, Socket>& getClientMap();

        void setBufferSize(int size); // size of receiving buffer in bytes, default 256
        void setReceiveCallback(void (*callback)(void* buffer, int bufferSize, int actualSize, Address fromClientAddress));
        void setClientConnectCallback(void (*callback)(Address clientAddress));
        void setClientDisconnectCallback(void (*callback)(Address clientAddress));

    private:
        Address m_addr;
        Socket m_socket;

        std::atomic<int> m_bufSize;
        std::atomic<int> m_nClients;

        std::list<Address> m_clientAddrs;
        std::unordered_map<Address, Socket> m_clientMap;
        std::mutex m_clientAddrsMtx;
        std::mutex m_clientMapMtx;

        std::atomic<bool> m_open;

        void accept(); // accept() and add to maps while true until error (from closure)
        void receive(Socket acceptedSocket); // receive() and callback while true until error (from closure)
        std::thread m_accepting;
        std::vector<std::thread> m_receivings;

        // USER IS RESPONSIBLE FOR DELETING BUFFER
        void (*m_pReceiveCallback)(void* buffer, int bufferSize, int actualSize, Address fromAddr);
        void (*m_pClientConnectCallback)(Address clientAddr);
        void (*m_pClientDisconnectCallback)(Address clientAddr);
    };

    class ServerUDP
    {
    public:
        ServerUDP();
        ServerUDP(Address serverAddress, bool* success = nullptr);

        void open(bool* success = nullptr);
        void send(void* data, int size, Address clientAddress, bool* success = nullptr);
        void close(bool* success = nullptr);

        bool isOpen() const;
        int getBufferSize() const;

        void setBufferSize(int size);
        void setReceiveCallback(void (*callback)(void* buffer, int bufferSize, int actualSize, Address fromClientAddress));

    private:
        Address m_addr;
        Socket m_socket;

        std::atomic<int> m_bufSize;
        std::atomic<bool> m_open;

        void receive();
        std::thread m_receiving;

        void (*m_pReceiveCallback)(void* buffer, int bufferSize, int actualSize, Address fromAddr);
    };

    class ClientTCP
    {
    public:
        ClientTCP(bool* success = nullptr);
        
        void connect(Address serverAddress, bool* success = nullptr);
        void send(void* data, int size, bool* success = nullptr);
        void disconnect(bool* success = nullptr);

        bool isConnected() const;
        int getBufferSize() const;

        void setBufferSize(int size); // size of receiving buffer in bytes, 256 by default
        void setReceiveCallback(void (*callback)(void* buffer, int bufferSize, int actualSize));

    private:
        Socket m_socket;

        std::atomic<int> m_bufSize;
        std::atomic<bool> m_connected;

        void receive(); // receive() and callback while true until error (from server or client closure)
        std::thread m_receiving;

        void (*m_pReceiveCallback)(void* buffer, int bufferSize, int actualSize);
    };

    class ClientUDP
    {
    public:
        ClientUDP(bool* success = nullptr);

        void start(bool* success = nullptr);
        void send(void* data, int size, Address serverAddress, bool* success = nullptr);
        void stop(bool* success = nullptr);

        bool isRunning() const;
        int getBufferSize() const;

        void setBufferSize(int size);
        void setReceiveCallback(void (*callback)(void* buffer, int bufferSize, int actualSize, Address fromServerAddress));

    private:
        Socket m_socket;

        std::atomic<int> m_bufSize;
        std::atomic<bool> m_running;

        void receive();
        std::thread m_receiving;

        void (*m_pReceiveCallback)(void* buffer, int bufferSize, int actualSize, Address fromAddr);
    };
};

#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>

#define GNET_VERSION_MAJOR  0
#define GNET_VERSION_MINOR  0
#define GNET_VERSION_PATCH  0

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
        unsigned short port;
    };

    class Socket
    {
    public:
        Socket();
        Socket(Protocol protocol, bool* result = nullptr);

        bool bind(Address serverAddress);
        bool listen(int maxClients);
        Socket accept(bool* result = nullptr);
        bool connect(Address serverAddress);

        bool send(void* data, int size);
        void receive(void* buffer, int bufferSize);
        bool sendTo(void* data, int size, Address to);
        void receiveFrom(void* buffer, int bufferSize, Address* from);

        void close();

        const Address& getAddress() const;
        const Protocol& getProtocol() const;
        bool isOpen() const;

    private:
        Address m_addr;
        Protocol m_prot;

        SOCKET m_wsSocket;
        SOCKADDR_IN m_wsAddr;
        int m_wsAddrSize;

        bool m_open;
    };
};

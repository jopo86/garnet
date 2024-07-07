#include "Garnet.h"

#include <iostream>
#include <vector>

#ifdef GNET_OS_WINDOWS
    bool wsaInitialized = false;
    WSADATA wsaData;
#endif

std::string err;
bool printErrors = false;
void* userPtr = nullptr;

#ifdef GNET_OS_WINDOWS

    SOCKADDR_IN addr_gtob(Garnet::Address addr)
    {
        SOCKADDR_IN bAddr;
        bAddr.sin_family = AF_INET;
        inet_pton(AF_INET, addr.host.c_str(), &bAddr.sin_addr.s_addr);
        bAddr.sin_port = htons(addr.port);
        return bAddr;
    }

    Garnet::Address addr_btog(SOCKADDR_IN addr)
    {
        Garnet::Address gAddr;
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr.s_addr, buf, sizeof(buf));
        gAddr.host = std::string(buf);
        gAddr.port = ntohs(addr.sin_port); 
        return gAddr;
    }

#elif defined(GNET_OS_UNIX)

    sockaddr_in addr_gtob(Garnet::Address addr) 
    {
        sockaddr_in bAddr;
        memset(&bAddr, 0, sizeof(bAddr)); // Ensure struct is zeroed out
        bAddr.sin_family = AF_INET;
        inet_pton(AF_INET, addr.host.c_str(), &bAddr.sin_addr.s_addr);
        bAddr.sin_port = htons(addr.port);
        return bAddr;
    }

    Garnet::Address addr_btog(sockaddr_in addr) 
    {
        Garnet::Address gAddr;
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr.s_addr, buf, sizeof(buf));
        gAddr.host = std::string(buf);
        gAddr.port = ntohs(addr.sin_port);
        return gAddr;
    }

#endif

void Garnet::Address::operator=(const Address& other)
{
    host = other.host;
    port = other.port;
}

bool Garnet::Address::operator==(const Address& other) const
{
    return (host == other.host && port == other.port);
}

int Garnet::GetVersionMajor()
{
    return GNET_VERSION_MAJOR;
}

int Garnet::GetVersionMinor()
{
    return GNET_VERSION_MINOR;
}

int Garnet::GetVersionPatch()
{
    return GNET_VERSION_PATCH;
}

std::string Garnet::GetVersionString()
{
    std::string v = std::to_string(GNET_VERSION_MAJOR) + "." + std::string(GNET_VERSION_MINOR) + "." + std::to_string(GNET_VERSION_PATCH);
    if (!GNET_STABLE)
    {
        if (GNET_DEV) v += "-dev";
        else if (GNET_ALPHA) v += "-alpha";
        else if (GNET_BETA) v += "-beta";
    }
    return v;
}

#ifdef GNET_OS_WINDOWS

    bool Garnet::Init(bool _printErrors)
    {
        printErrors = _printErrors;

        if (wsaInitialized)
        {
            err = "Initialization failed: already initialized";
            if (printErrors) std::cout << err << "\n";
            return false;
        }

        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            err = "Initialization failed: Winsock DLL not found";
            if (printErrors) std::cout << err << "\n";
            return false;
        }

        wsaInitialized = true;
        return true;
    }

    void Garnet::Terminate()
    {
        WSACleanup();
    }

#elif defined(GNET_OS_UNIX)

    bool Garnet::Init(bool _printErrors)
    {
        printErrors = _printErrors;
        return true;
    }

    void Garnet::Terminate() {}

#endif

const std::string& Garnet::GetLastError()
{
    return err;
}

void Garnet::SetUserPtr(void* ptr)
{
    userPtr = ptr;
}

void* Garnet::GetUserPtr()
{
    return userPtr;
}

std::string Garnet::HostnameToIP(const std::string& hostname, bool* success)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  // Use IPv4
    hints.ai_socktype = SOCK_STREAM;  // Use TCP

    if (getaddrinfo(hostname.c_str(), nullptr, &hints, &res) != 0)
    {
        err = "Failed to resolve hostname: '" + hostname + "'";
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return "";
    }

    char ipStr[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(((struct sockaddr_in*)res->ai_addr)->sin_addr), ipStr, sizeof(ipStr)) == nullptr)
    {
        freeaddrinfo(res);
        err = "Failed to convert address to string";
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return "";
    }

    std::string ipAddr(ipStr);
    freeaddrinfo(res);
    if (success != nullptr) *success = true;
    return ipAddr;
}

#ifdef GNET_OS_WINDOWS

    Garnet::Socket::Socket()
    {
        m_addr.host = "";
        m_addr.port = 0;
        m_proto = Protocol::Null;
        m_bSocket = INVALID_SOCKET;
        m_bAddr.sin_family = AF_INET;
        m_bAddrSize = sizeof(m_bAddr);
        m_open = false;
    }

    Garnet::Socket::Socket(Protocol proto, bool* success)
    {
        m_addr.host = "";
        m_addr.port = 0;
        m_proto = proto;
        m_bSocket = INVALID_SOCKET;
        m_bAddr.sin_family = AF_INET;
        m_bAddrSize = sizeof(m_bAddr);

        if (m_proto == Protocol::Null)
        {
            err = "Socket creation failed: protocol cannot be null";
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return;
        }
        else if (m_proto == Protocol::TCP)
        {
            m_bSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            u_long mode = 0;
            ioctlsocket(m_bSocket, FIONBIO, &mode);
            if (m_bSocket == INVALID_SOCKET)
            {
                err = "Socket creation failed. WSA error code: " + std::to_string(WSAGetLastError());
                if (printErrors) std::cout << err << "\n";
                if (success != nullptr) *success = false;
                return;
            }
        }
        else if (m_proto == Protocol::UDP)
        {
            m_bSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (m_bSocket == INVALID_SOCKET)
            {
                err = "Socket creation failed. WSA error code: " + std::to_string(WSAGetLastError());
                if (printErrors) std::cout << err << "\n";
                if (success != nullptr) *success = false;
                return;
            }
        }

        m_open = true;
        if (success != nullptr) *success = true;
    }

    void Garnet::Socket::bind(Address addr, bool* success)
    {
        m_addr = addr;
        SOCKADDR_IN bAddr = addr_gtob(addr);
        m_bAddr = bAddr;
        m_bAddrSize = sizeof(bAddr);

        if (::bind(m_bSocket, (SOCKADDR*)&bAddr, sizeof(bAddr)) == SOCKET_ERROR)
        {
            err = "Socket binding failed. WSA error code: " + std::to_string(WSAGetLastError());
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        if (success != nullptr) *success = true;    
        return;
    }

    void Garnet::Socket::listen(int backlog, bool* success)
    {
        if (::listen(m_bSocket, backlog) == SOCKET_ERROR)
        {
            err = "Socket listening failed. WSA error code: " + std::to_string(WSAGetLastError());
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;        
            return;
        }

        if (success != nullptr) *success = true;
        return;
    }

    Garnet::Socket Garnet::Socket::accept(bool* success)
    {
        Socket retval;

        ::SOCKET acceptSocket = INVALID_SOCKET;
        acceptSocket = ::accept(m_bSocket, (SOCKADDR*)&retval.m_bAddr, &retval.m_bAddrSize);
        if (acceptSocket == INVALID_SOCKET)
        {
            err = "Socket accept failed. WSA error code: " + std::to_string(WSAGetLastError());
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return retval;
        }

        retval.m_bSocket = acceptSocket;
        retval.m_addr = addr_btog(retval.m_bAddr);
        retval.m_proto = m_proto;

        if (success != nullptr) *success = true;

        return retval;
    }

    void Garnet::Socket::connect(Address addr, bool* success)
    {
        SOCKADDR_IN bAddr;
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(addr.host.c_str(), std::to_string(addr.port).c_str(), &hints, &res) != 0)
        {
            err = "Socket connect failed: failed to resolve host";
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        bAddr = *(SOCKADDR_IN*)res->ai_addr;
        freeaddrinfo(res);

        if (::connect(m_bSocket, (SOCKADDR*)&bAddr, sizeof(bAddr)) == SOCKET_ERROR)
        {
            err = "Socket connect failed. WSA error code: " + std::to_string(WSAGetLastError());
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        if (success != nullptr) *success = true;
    }

    int Garnet::Socket::send(void* data, int size, bool* success)
    {
        int nBytes = ::send(m_bSocket, (char*)data, size, 0);
        if (success != nullptr) *success = nBytes != SOCKET_ERROR;
        return nBytes;
    }

    int Garnet::Socket::receive(void* buffer, int bufferSize, bool* success)
    {
        int nBytes = ::recv(m_bSocket, (char*)buffer, bufferSize, 0);
        if (success != nullptr) *success = nBytes != SOCKET_ERROR;
        return nBytes;
    }

    int Garnet::Socket::sendTo(void* data, int size, Address to, bool* success)
    {
        SOCKADDR_IN wsTo;
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        if (getaddrinfo(to.host.c_str(), std::to_string(to.port).c_str(), &hints, &res) != 0)
        {
            err = "sendTo failed: failed to resolve host";
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return SOCKET_ERROR;
        }

        wsTo = *(SOCKADDR_IN*)res->ai_addr;
        freeaddrinfo(res);

        int nBytes = ::sendto(m_bSocket, (char*)data, size, 0, (SOCKADDR*)&wsTo, sizeof(wsTo));
        if (success != nullptr) *success = nBytes != SOCKET_ERROR;
        return nBytes;
    }

    int Garnet::Socket::receiveFrom(void* buffer, int bufferSize, Address* from, bool* success)
    {
        SOCKADDR_IN wsFrom;
        int wsFromSize = sizeof(wsFrom);
        int nBytes = ::recvfrom(m_bSocket, (char*)buffer, bufferSize, 0, (SOCKADDR*)&wsFrom, &wsFromSize);
        if (success != nullptr) *success = nBytes != SOCKET_ERROR;
        if (from != nullptr && nBytes != SOCKET_ERROR) *from = addr_btog(wsFrom);
        return nBytes;
    }

    void Garnet::Socket::close()
    {
        closesocket(m_bSocket);
        m_open = false;
    }

#elif defined(GNET_OS_UNIX)
    Garnet::Socket::Socket()
    {
        m_addr.host = "";
        m_addr.port = 0;
        m_proto = Protocol::Null;
        m_bSocket = -1;
        m_bAddr.sin_family = AF_INET;
        m_bAddrSize = sizeof(m_bAddr);
        m_open = false;
    }

    Garnet::Socket::Socket(Protocol proto, bool* success)
    {
        m_addr.host = "";
        m_addr.port = 0;
        m_proto = proto;
        m_bSocket = -1;
        m_bAddr.sin_family = AF_INET;
        m_bAddrSize = sizeof(m_bAddr);

        if (m_proto == Protocol::Null)
        {
            err = "Socket creation failed: protocol cannot be null";
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return;
        }
        else if (m_proto == Protocol::TCP)
        {
            m_bSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (m_bSocket == -1)
            {
                err = "Socket creation failed. Error: " + std::string(strerror(errno));
                if (printErrors) std::cout << err << "\n";
                if (success != nullptr) *success = false;
                return;
            }
        }
        else if (m_proto == Protocol::UDP)
        {
            m_bSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (m_bSocket == -1)
            {
                err = "Socket creation failed. Error: " + std::string(strerror(errno));
                if (printErrors) std::cout << err << "\n";
                if (success != nullptr) *success = false;
                return;
            }
        }

        m_open = true;
        if (success != nullptr) *success = true;
    }

    void Garnet::Socket::bind(Address addr, bool* success)
    {
        m_addr = addr;
        sockaddr_in bAddr = addr_gtob(addr);
        m_bAddr = bAddr;
        m_bAddrSize = sizeof(bAddr);

        if (::bind(m_bSocket, (SOCKADDR*)&bAddr, sizeof(bAddr)) == -1)
        {
            err = "Socket binding failed. Error: " + std::string(strerror(errno));
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        if (success != nullptr) *success = true;    
        return;
    }

    void Garnet::Socket::listen(int backlog, bool* success)
    {
        if (::listen(m_bSocket, backlog) == -1)
        {
            err = "Socket listening failed. Error: " + std::string(strerror(errno));
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;        
            return;
        }

        if (success != nullptr) *success = true;
        return;
    }

    Garnet::Socket Garnet::Socket::accept(bool* success)
    {
        Socket retval;

        ::SOCKET acceptSocket = -1;
        acceptSocket = ::accept(m_bSocket, (SOCKADDR*)&retval.m_bAddr, &retval.m_bAddrSize);
        if (acceptSocket == -1)
        {
            err = "Socket accept failed. Error: " + std::string(strerror(errno));
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return retval;
        }

        retval.m_bSocket = acceptSocket;
        retval.m_addr = addr_btog(retval.m_bAddr);
        retval.m_proto = m_proto;

        if (success != nullptr) *success = true;

        return retval;
    }

    void Garnet::Socket::connect(Address addr, bool* success)
    {
        sockaddr_in bAddr;
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(addr.host.c_str(), std::to_string(addr.port).c_str(), &hints, &res) != 0)
        {
            err = "Socket connect failed: failed to resolve host";
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        bAddr = *(sockaddr_in*)res->ai_addr;
        freeaddrinfo(res);

        if (::connect(m_bSocket, (SOCKADDR*)&bAddr, sizeof(bAddr)) == -1)
        {
            err = "Socket connect failed. Error: " + std::string(strerror(errno));
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        if (success != nullptr) *success = true;
    }

    int Garnet::Socket::send(void* data, int size, bool* success)
    {
        int nBytes = ::send(m_bSocket, (char*)data, size, 0);
        if (success != nullptr) *success = nBytes != -1;
        return nBytes;
    }

    int Garnet::Socket::receive(void* buffer, int bufferSize, bool* success)
    {
        int nBytes = ::recv(m_bSocket, (char*)buffer, bufferSize, 0);
        if (success != nullptr) *success = nBytes != -1;
        return nBytes;
    }

    int Garnet::Socket::sendTo(void* data, int size, Address to, bool* success)
    {
        sockaddr_in wsTo;
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        if (getaddrinfo(to.host.c_str(), std::to_string(to.port).c_str(), &hints, &res) != 0)
        {
            err = "sendTo failed: failed to resolve host";
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return -1;
        }

        wsTo = *(sockaddr_in*)res->ai_addr;
        freeaddrinfo(res);

        int nBytes = ::sendto(m_bSocket, (char*)data, size, 0, (SOCKADDR*)&wsTo, sizeof(wsTo));
        if (success != nullptr) *success = nBytes != -1;
        return nBytes;
    }

    int Garnet::Socket::receiveFrom(void* buffer, int bufferSize, Address* from, bool* success)
    {
        sockaddr_in wsFrom;
        int wsFromSize = sizeof(wsFrom);
        int nBytes = ::recvfrom(m_bSocket, (char*)buffer, bufferSize, 0, (SOCKADDR*)&wsFrom, &wsFromSize);
        if (success != nullptr) *success = nBytes != -1;
        if (from != nullptr && nBytes != -1) *from = addr_btog(wsFrom);
        return nBytes;
    }

    void Garnet::Socket::close()
    {
        ::close(m_bSocket);
        m_open = false;
    }
    
#endif

const Garnet::Address& Garnet::Socket::getAddress() const
{
    return m_addr;
}

const Garnet::Protocol& Garnet::Socket::getProtocol() const
{
    return m_proto;
}

bool Garnet::Socket::isOpen() const
{
    return m_open;
}

Garnet::ServerTCP::ServerTCP()
{
    m_addr.host = "";
    m_addr.port = 0;
    m_bufSize.exchange(256);
    m_nClients.exchange(0);
    m_open.exchange(false);
    m_pReceiveCallback = nullptr;
    m_pClientConnectCallback = nullptr;
    m_pClientDisconnectCallback = nullptr;
}

Garnet::ServerTCP::ServerTCP(Address addr, bool* success)
{
    m_addr = addr;
    bool successA, successB;
    m_socket = Socket(Protocol::TCP, &successA);
    m_socket.bind(addr, &successB);
    m_bufSize.exchange(256);
    m_nClients.exchange(0);
    m_open.exchange(false);
    m_pReceiveCallback = nullptr;
    m_pClientConnectCallback = nullptr;
    m_pClientDisconnectCallback = nullptr;

    if (success != nullptr) *success = successA && successB; 
}

void Garnet::ServerTCP::open(int backlog, bool* success)
{
    if (m_open.load())
    {
        err = "Failed to open ServerTCP: already open";
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_open.exchange(true);
    bool successA;
    m_socket.listen(backlog, &successA);
    if (success != nullptr) *success = successA;
    if (successA)
    {
        m_accepting = std::thread(&Garnet::ServerTCP::accept, this);
    }
    else m_open.exchange(false);
}

void Garnet::ServerTCP::send(void* data, int size, Address clientAddr, bool* success)
{
    m_clientMap[clientAddr].send(data, size, success);
}

void Garnet::ServerTCP::close(bool* success)
{
    if (!m_open.load())
    {
        err = "Failed to close ServerTCP: not open yet or already closed";
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_socket.close();
    for (Address& acceptedAddr : m_clientAddrs)
    {
        m_clientMap[acceptedAddr].close();
    }
    m_open.exchange(false);
    m_accepting.detach();
    for (std::thread& receiving : m_receivings) receiving.detach();
    m_receivings.clear();
    m_clientAddrsMtx.lock();
    m_clientMapMtx.lock();
    m_clientAddrs.clear();
    m_clientMap.clear();
    m_clientAddrsMtx.unlock();
    m_clientMapMtx.unlock();
    if (success != nullptr) *success = true;
}

bool Garnet::ServerTCP::isOpen() const
{
    return m_open.load();
}

int Garnet::ServerTCP::getBufferSize() const
{
    return m_bufSize.load();
}

int Garnet::ServerTCP::getNumClients() const
{
    return m_nClients.load();
}

Garnet::Socket& Garnet::ServerTCP::getClientAcceptedSocket(Address clientAddr)
{
    return m_clientMap[clientAddr];
}

const std::list<Garnet::Address>& Garnet::ServerTCP::getClientAddresses() const
{
    return m_clientAddrs;
}

const std::unordered_map<Garnet::Address, Garnet::Socket>& Garnet::ServerTCP::getClientMap() const
{
    return m_clientMap;
}

void Garnet::ServerTCP::setBufferSize(int size)
{
    m_bufSize.exchange(size);
}

void Garnet::ServerTCP::setReceiveCallback(void(*callback)(void* buffer, int bufferSize, int actualSize, Address fromClientAddr))
{
    m_pReceiveCallback = callback;
}

void Garnet::ServerTCP::setClientConnectCallback(void(*callback)(Address clientAddr))
{
    m_pClientConnectCallback = callback;
}

void Garnet::ServerTCP::setClientDisconnectCallback(void(*callback)(Address clientAddr))
{
    m_pClientDisconnectCallback = callback;
}

void Garnet::ServerTCP::accept()
{
    while (m_open.load())
    {
        bool success;
        Socket acceptedSocket;

        bool prevPrintErrors = printErrors;
        printErrors = false;
        acceptedSocket = m_socket.accept(&success);
        printErrors = prevPrintErrors;
        if (!success) continue;
        else
        {
            m_clientAddrsMtx.lock();
            m_clientMapMtx.lock();
            m_clientAddrs.push_back(acceptedSocket.getAddress());
            m_clientMap.insert({ acceptedSocket.getAddress(), acceptedSocket });
            m_clientAddrsMtx.unlock();
            m_clientMapMtx.unlock();

            m_receivings.push_back(std::thread(&Garnet::ServerTCP::receive, this, acceptedSocket));
            m_nClients.exchange(m_nClients.load() + 1);

            if (m_pClientConnectCallback != nullptr) m_pClientConnectCallback(acceptedSocket.getAddress());
        }
    }
}

void Garnet::ServerTCP::receive(Socket acceptedSocket)
{
    while (m_open.load())
    {
        if (m_pReceiveCallback == nullptr) continue;

        byte* buf = new byte[m_bufSize];
        bool recvSuccess;
        int nBytes = acceptedSocket.receive(buf, m_bufSize, &recvSuccess);
        if (!recvSuccess)
        {
            // client disconnected
            m_clientAddrsMtx.lock();
            m_clientMapMtx.lock();
            m_clientAddrs.remove(acceptedSocket.getAddress());
            m_clientMap.erase(acceptedSocket.getAddress());
            m_clientAddrsMtx.unlock();
            m_clientMapMtx.unlock();
            m_nClients.exchange(m_nClients.load() - 1);

            if (m_pClientDisconnectCallback != nullptr) m_pClientDisconnectCallback(acceptedSocket.getAddress());

            delete buf;
            break;
        }

        m_pReceiveCallback(buf, m_bufSize, nBytes, acceptedSocket.getAddress());
    }
}

Garnet::ServerUDP::ServerUDP()
{
    m_addr.host = "";
    m_addr.port = 0;
    m_bufSize.exchange(256);
    m_open.exchange(false);
    m_pReceiveCallback = nullptr;
}

Garnet::ServerUDP::ServerUDP(Address addr, bool* success)
{
    m_addr = addr;
    bool successA, successB;
    m_socket = Socket(Protocol::UDP, &successA);
    m_socket.bind(addr, &successB);
    m_bufSize.exchange(256);
    m_open.exchange(false);
    m_pReceiveCallback = nullptr;

    if (success != nullptr) *success = successA && successB;
}

void Garnet::ServerUDP::open(bool* success)
{
    if (m_open.load())
    {
        err = "Failed to open ServerUDP: already open";
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_open.exchange(true);
    m_receiving = std::thread(&Garnet::ServerUDP::receive, this);
    if (success != nullptr) *success = true;
}

void Garnet::ServerUDP::send(void* data, int size, Address addr, bool* success)
{
    m_socket.sendTo(data, size, addr, success);
}

void Garnet::ServerUDP::close(bool* success)
{
    if (!m_open.load())
    {
        err = "Failed to close ServerUDP: not open yet or already closed";
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_socket.close();
    m_open.exchange(false);
    m_receiving.detach();
    if (success != nullptr) *success = true;
}

bool Garnet::ServerUDP::isOpen() const
{
    return m_open.load();
}

int Garnet::ServerUDP::getBufferSize() const
{
    return m_bufSize.load();
}

void Garnet::ServerUDP::setBufferSize(int size)
{
    m_bufSize.exchange(size);
}

void Garnet::ServerUDP::setReceiveCallback(void (*callback)(void* buffer, int bufferSize, int actualSize, Address fromAddr))
{
    m_pReceiveCallback = callback;
}

void Garnet::ServerUDP::receive()
{
    while (m_open.load())
    {
        if (m_pReceiveCallback == nullptr) continue;

        bool recvSuccess;
        Address from;
        byte* buf = new byte[m_bufSize];
        int nBytes = m_socket.receiveFrom(buf, m_bufSize, &from, &recvSuccess);
        if (!recvSuccess)
        {
            delete buf;
            continue;
        }

        m_pReceiveCallback(buf, m_bufSize, nBytes, from);
    }
}

Garnet::ClientTCP::ClientTCP()
{
    m_bufSize.exchange(256);
    m_pReceiveCallback = nullptr;
    m_connected.exchange(false);
}

Garnet::ClientTCP::ClientTCP(char dummy, bool* success)
{
    m_bufSize.exchange(256);
    m_pReceiveCallback = nullptr;
    m_connected.exchange(false);
    m_socket = Socket(Protocol::TCP, success);
}

void Garnet::ClientTCP::connect(Address serverAddr, bool* success)
{
    if (m_connected.load())
    {
        err = "Failed to connect ClientTCP: already connected";
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    bool successA;
    m_socket.connect(serverAddr, &successA);
    if (successA)
    {
        m_connected.exchange(true);
        if (success != nullptr) *success = true;
    }
    else
    {
        if (success != nullptr) *success = false;
        return;
    }

    m_receiving = std::thread(&Garnet::ClientTCP::receive, this);
}

void Garnet::ClientTCP::send(void* data, int size, bool* success)
{
    m_socket.send(data, size, success);
}

void Garnet::ClientTCP::disconnect(bool* success)
{
    if (!m_connected.load())
    {
        err = "Failed to disconnect ClientTCP: not connected yet or already disconnected";
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }
    
    m_socket.close();
    m_connected.exchange(false);
    m_receiving.detach();
    if (success != nullptr) *success = true;
}

bool Garnet::ClientTCP::isConnected() const
{
    return m_connected.load();
}

int Garnet::ClientTCP::getBufferSize() const
{
    return m_bufSize.load();
}

void Garnet::ClientTCP::setBufferSize(int bufferSize)
{
    m_bufSize.exchange(bufferSize);
}

void Garnet::ClientTCP::setReceiveCallback(void (*callback)(void* buffer, int bufferSize, int actualSize))
{
    m_pReceiveCallback = callback;
}

void Garnet::ClientTCP::receive()
{
    while (m_connected.load())
    {
        if (m_pReceiveCallback == nullptr) continue;

        byte* buf = new byte[m_bufSize];
        bool recvSuccess;
        int nBytes = m_socket.receive(buf, m_bufSize, &recvSuccess);
        if (!recvSuccess) continue;
        
        m_pReceiveCallback(buf, m_bufSize, nBytes);
    }
}

Garnet::ClientUDP::ClientUDP()
{
    m_bufSize.exchange(256);
    m_pReceiveCallback = nullptr;
    m_connected.exchange(false);
}

Garnet::ClientUDP::ClientUDP(char dummyPutAnything, bool* success)
{
    m_bufSize.exchange(256);
    m_pReceiveCallback = nullptr;
    m_connected.exchange(true);
    m_socket = Socket(Protocol::UDP, success);
}

void Garnet::ClientUDP::send(void* data, int size, Address addr, bool* success)
{
    m_socket.sendTo(data, size, addr, success);
}

void Garnet::ClientUDP::disconnect(bool* success)
{
    if (!m_connected.load())
    {
        err = "Failed to disconnect ClientUDP: not connected yet or already disconnected";
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_socket.close();
    m_connected.exchange(false);
    m_receiving.detach();
    if (success != nullptr) *success = true;
}

bool Garnet::ClientUDP::isConnected() const
{
    return m_connected.load();
}

int Garnet::ClientUDP::getBufferSize() const
{
    return m_bufSize.load();
}

void Garnet::ClientUDP::setBufferSize(int size)
{
    m_bufSize.exchange(size);
}

void Garnet::ClientUDP::setReceiveCallback(void (*callback)(void* buffer, int bufferSize, int actualSize, Address fromServerAddress))
{
    m_pReceiveCallback = callback;
}

void Garnet::ClientUDP::receive()
{
    while (m_connected.load())
    {
        if (m_pReceiveCallback == nullptr) continue;

        bool recvSuccess;
        Address from;
        byte* buf = new byte[m_bufSize];
        int nBytes = m_socket.receiveFrom(buf, m_bufSize, &from, &recvSuccess);
        if (!recvSuccess)
        {
            delete buf;
            continue;
        }

        m_pReceiveCallback(buf, m_bufSize, nBytes, from);
    }
}

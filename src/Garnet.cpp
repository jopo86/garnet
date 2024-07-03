#include "Garnet.h"

#include <iostream>
#include <vector>

bool wsaInitialized = false;
WSADATA wsaData;
std::string err;
bool printErrors = false;
void* userPtr;

SOCKADDR_IN addr_gtow(Garnet::Address addr)
{
    SOCKADDR_IN wsAddr;
    wsAddr.sin_family = AF_INET;
    inet_pton(AF_INET, addr.IP.c_str(), &wsAddr.sin_addr.s_addr);
    wsAddr.sin_port = htons(addr.port);
    return wsAddr;
}

Garnet::Address addr_wtog(SOCKADDR_IN addr)
{
    Garnet::Address gAddr;
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr.s_addr, buf, sizeof(buf));
    gAddr.IP = std::string(buf);
    gAddr.port = ntohs(addr.sin_port); 
    return gAddr;
}

bool Garnet::Address::operator==(const Address& other) const
{
    return (IP == other.IP && port == other.port);
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

bool Garnet::Init(bool _printErrors)
{
    if (wsaInitialized)
    {
        err = "Initialization failed: already initialized";
        if (printErrors) std::cout << err << "\n";
        return false;
    }

#ifndef GNET_OS_WINDOWS
    err = "Initialization failed: Garnet currently only supports Windows operating systems.";
    if (printErrors) std::cout << err << "\n";
    return false;
#endif

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        err = "Initialization failed: Winsock DLL not found";
        if (printErrors) std::cout << err << "\n";
        return false;
    }

    wsaInitialized = true;
    printErrors = _printErrors;
    return true;
}

void Garnet::Terminate()
{
    WSACleanup();
}

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

Garnet::Socket::Socket()
{
    m_addr.IP = "";
    m_addr.port = 0;
    m_proto = Protocol::Null;
    m_wsSocket = INVALID_SOCKET;
    m_wsAddr.sin_family = AF_INET;
    m_wsAddrSize = sizeof(m_wsAddr);
    m_open = false;
}

Garnet::Socket::Socket(Protocol proto, bool* success)
{
    m_addr.IP = "";
    m_addr.port = 0;
    m_proto = proto;
    m_wsSocket = INVALID_SOCKET;
    m_wsAddr.sin_family = AF_INET;
    m_wsAddrSize = sizeof(m_wsAddr);

    SOCKET wsSocketCopy = INVALID_SOCKET;

    if (m_proto == Protocol::Null)
    {
        err = "Socket creation failed: protocol cannot be null";
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }
    else if (m_proto == Protocol::TCP)
    {
        wsSocketCopy = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        m_wsSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_wsSocket == INVALID_SOCKET || wsSocketCopy == INVALID_SOCKET)
        {
            err = "Socket creation failed. WSA error code: " + std::to_string(WSAGetLastError());
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return;
        }
    }
    else if (m_proto == Protocol::UDP)
    {
        wsSocketCopy = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        m_wsSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_wsSocket == INVALID_SOCKET || wsSocketCopy == INVALID_SOCKET)
        {
            err = "Socket creation failed. WSA error code: " + std::to_string(WSAGetLastError());
            if (printErrors) std::cout << err << "\n";
            if (success != nullptr) *success = false;
            return;
        }
    }

    SOCKADDR_IN localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(0);

    if (::bind(wsSocketCopy, (SOCKADDR*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR)
    {
        err = "Socket creation failed: could not bind to local address. WSA error code: " + std::to_string(WSAGetLastError());
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    if (getsockname(wsSocketCopy, (SOCKADDR*)&m_wsAddr, &m_wsAddrSize) == SOCKET_ERROR)
    {
        err = "Socket creation failed: could not get socket address. WSA error code: " + std::to_string(WSAGetLastError());
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_addr = addr_wtog(m_wsAddr);
    m_open = true;

    if (success != nullptr) *success = true;

    closesocket(wsSocketCopy);
}

void Garnet::Socket::bind(Address addr, bool* success)
{
    m_addr = addr;
    SOCKADDR_IN wsAddr = addr_gtow(addr);
    m_wsAddr = wsAddr;
    m_wsAddrSize = sizeof(wsAddr);

    if (::bind(m_wsSocket, (SOCKADDR*)&wsAddr, sizeof(wsAddr)) == SOCKET_ERROR)
    {
        err = "Socket binding failed. WSA error code: " + std::to_string(WSAGetLastError());
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    if (success != nullptr) *success = true;    
    return;
}

void Garnet::Socket::listen(int maxClients, bool* success)
{
    if (::listen(m_wsSocket, maxClients) == SOCKET_ERROR)
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
    acceptSocket = ::accept(m_wsSocket, (SOCKADDR*)&retval.m_wsAddr, &retval.m_wsAddrSize);
    if (acceptSocket == INVALID_SOCKET)
    {
        err = "Socket accept failed. WSA error code: " + std::to_string(WSAGetLastError());
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return retval;
    }

    retval.m_wsSocket = acceptSocket;
    retval.m_addr = addr_wtog(retval.m_wsAddr);
    retval.m_proto = m_proto;

    if (success != nullptr) *success = true;

    return retval;
}

void Garnet::Socket::connect(Address addr, bool* success)
{
    SOCKADDR_IN wsAddr = addr_gtow(addr);

    if (::connect(m_wsSocket, (SOCKADDR*)&wsAddr, sizeof(wsAddr)) == SOCKET_ERROR)
    {
        err = "Socket connect failed. WSA error code: " + std::to_string(WSAGetLastError());
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    if (success != nullptr) *success = true;
    return;
}

int Garnet::Socket::send(void* data, int size, bool* success)
{
    int byteCount = ::send(m_wsSocket, (char*)data, size, 0);
    if (success != nullptr)
    {
        if (byteCount == SOCKET_ERROR) *success = false;
        else *success = true;
    }
    return byteCount;
}

int Garnet::Socket::receive(void* buffer, int bufferSize)
{
    return ::recv(m_wsSocket, (char*)buffer, bufferSize, 0);
}

int Garnet::Socket::sendTo(void* data, int size, Address to, bool* success)
{
    SOCKADDR_IN wsTo = addr_gtow(to);
    int byteCount = ::sendto(m_wsSocket, (char*)data, size, 0, (SOCKADDR*)&wsTo, sizeof(wsTo));
    if (success != nullptr)
    {
        if (byteCount == SOCKET_ERROR) *success = false;
        else *success = true;
    }
    return byteCount;
}

int Garnet::Socket::receiveFrom(void* buffer, int bufferSize, Address* from, bool* success)
{
    SOCKADDR_IN wsFrom;
    int wsFromSize = sizeof(wsFrom);
    int byteCount = ::recvfrom(m_wsSocket, (char*)buffer, bufferSize, 0, (SOCKADDR*)&wsFrom, &wsFromSize);
    if (success != nullptr)
    {
        if (byteCount == SOCKET_ERROR) *success = false;
        else *success = true;
    }
    if (from != nullptr && byteCount != SOCKET_ERROR) *from = addr_wtog(wsFrom);
    return byteCount;
}

void Garnet::Socket::close()
{
    closesocket(m_wsSocket);
    m_open = false;
}

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
    m_addr.IP = "0.0.0.0";
    m_addr.port = 0;
    m_bufSize.exchange(256);
    m_nClients.exchange(0);
    m_open.exchange(false);
    m_pReceiveCallback = nullptr;
}

Garnet::ServerTCP::ServerTCP(Address addr, bool* success)
{
    m_addr.IP = addr.IP;
    m_addr.port = addr.port;
    bool successA, successB;
    m_socket = Socket(Protocol::TCP, &successA);
    m_socket.bind(addr, &successB);
    m_bufSize.exchange(256);
    m_nClients.exchange(0);
    m_open.exchange(false);
    m_pReceiveCallback = nullptr;

    if (success != nullptr) *success = successA && successB; 
}

void Garnet::ServerTCP::open(int maxClients, int bufSize, bool* success)
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
    m_socket.listen(maxClients, &successA);
    if (success != nullptr) *success = successA;
    if (successA)
    {
        m_accepting = std::thread(&Garnet::ServerTCP::accept, this);
    }
    else m_open.exchange(false);
}

void Garnet::ServerTCP::send(Address clientAddr, void* data, int size, bool* success)
{
    m_clientMap[clientAddr].send(data, size, success);
}

void Garnet::ServerTCP::close(bool* success)
{
    if (m_open.load())
    {
        err = "Failed to close ServerTCP: not open yet or already closed";
        if (printErrors) std::cout << err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_socket.close();
    m_accepting.detach();
    for (std::thread& receiving : m_receivings) receiving.detach();
    m_open.exchange(true);
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

const Garnet::Socket& Garnet::ServerTCP::getClientAcceptedSocket(Address clientAddr)
{
    return m_clientMap[clientAddr];
}

const std::list<Garnet::Address>& Garnet::ServerTCP::getClientAddresses()
{
    return m_clientAddrs;
}

const std::unordered_map<Garnet::Address, Garnet::Socket>& Garnet::ServerTCP::getClientMap()
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

void Garnet::ServerTCP::accept()
{
    while (m_open.load())
    {
        bool success;
        Socket acceptedSocket;

        acceptedSocket = m_socket.accept(&success);
        if (!success) continue;
        else
        {
            m_clientAddrsMtx.lock();
            m_clientMapMtx.lock();
            m_clientAddrs.push_back(acceptedSocket.getAddress());
            m_clientMap.insert({ acceptedSocket.getAddress(), acceptedSocket });
            m_clientAddrsMtx.unlock();
            m_clientMapMtx.unlock();

            m_receivings.push_back(std::thread(&Garnet::ServerTCP::receive, this, acceptedSocket, m_nClients.load()));
            m_nClients.exchange(m_nClients.load() + 1);
        }
    }
}

void Garnet::ServerTCP::receive(Socket acceptedSocket, int clientIdx)
{
    while (m_open.load())
    {
        byte* buf = new byte[m_bufSize];
        int nBytes = acceptedSocket.receive(buf, m_bufSize);
        if (nBytes == SOCKET_ERROR)
        {
            // client disconnected
            m_clientAddrsMtx.lock();
            m_clientMapMtx.lock();
            m_clientAddrs.remove(acceptedSocket.getAddress());
            m_clientMap.erase(acceptedSocket.getAddress());
            m_clientAddrsMtx.unlock();
            m_clientMapMtx.unlock();
            m_nClients.exchange(m_nClients.load() - 1);
            break;
        }

        if (m_pReceiveCallback != nullptr) m_pReceiveCallback(buf, m_bufSize, nBytes, acceptedSocket.getAddress());
        else delete buf;
    }
}



Garnet::ClientTCP::ClientTCP(int bufferSize, bool* success)
{
    m_bufSize.exchange(bufferSize);
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
    else if (success != nullptr) *success = false;

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
    m_receiving.detach();
    if (success != nullptr) *success = true;
    m_connected.exchange(false);
}

bool Garnet::ClientTCP::isConnected() const
{
    return m_connected.load();
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
        byte* buf = new byte[m_bufSize];
        int nBytes = m_socket.receive(buf, m_bufSize);
        if (nBytes == SOCKET_ERROR) continue;
        
        if (m_pReceiveCallback != nullptr) m_pReceiveCallback(buf, m_bufSize, nBytes);
        else delete buf;
    }
}

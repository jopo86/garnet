#include "Garnet.h"

#include <iostream>
#include <vector>

bool wsaInitialized = false;
WSADATA wsaData;
std::string err;
bool printErrors = false;

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
    std::cout << "Garnet currently only supports Windows operating systems. Initialization failed.";
    return;
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

Garnet::Socket::Socket()
{
    m_addr.IP = "";
    m_addr.port = 0;
    m_prot = Protocol::Null;
    m_wsSocket = INVALID_SOCKET;
    m_wsAddr.sin_family = AF_INET;
    m_wsAddrSize = sizeof(m_wsAddr);
    m_open = false;
}

Garnet::Socket::Socket(Protocol prot, bool* result)
{
    m_addr.IP = "";
    m_addr.port = 0;
    m_prot = prot;
    m_wsSocket = INVALID_SOCKET;
    m_wsAddr.sin_family = AF_INET;
    m_wsAddrSize = sizeof(m_wsAddr);

    SOCKET wsSocketCopy = INVALID_SOCKET;

    if (m_prot == Protocol::Null)
    {
        err = "Socket creation failed: protocol cannot be null";
        if (printErrors) std::cout << err << "\n";
        if (result != nullptr) *result = false;
        return;
    }
    else if (m_prot == Protocol::TCP)
    {
        wsSocketCopy = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        m_wsSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_wsSocket == INVALID_SOCKET || wsSocketCopy == INVALID_SOCKET)
        {
            err = "Socket creation failed. WSA error code: " + std::to_string(WSAGetLastError());
            if (printErrors) std::cout << err << "\n";
            if (result != nullptr) *result = false;
            return;
        }
    }
    else if (m_prot == Protocol::UDP)
    {
        wsSocketCopy = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        m_wsSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_wsSocket == INVALID_SOCKET || wsSocketCopy == INVALID_SOCKET)
        {
            err = "Socket creation failed. WSA error code: " + std::to_string(WSAGetLastError());
            if (printErrors) std::cout << err << "\n";
            if (result != nullptr) *result = false;
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
        if (result != nullptr) *result = false;
        return;
    }

    if (getsockname(wsSocketCopy, (SOCKADDR*)&m_wsAddr, &m_wsAddrSize) == SOCKET_ERROR)
    {
        err = "Socket creation failed: could not get socket address. WSA error code: " + std::to_string(WSAGetLastError());
        if (printErrors) std::cout << err << "\n";
        if (result != nullptr) *result = false;
        return;
    }

    m_addr = addr_wtog(m_wsAddr);
    m_open = true;

    if (result != nullptr) *result = true;

    closesocket(wsSocketCopy);
}

bool Garnet::Socket::bind(Address addr)
{
    m_addr = addr;
    SOCKADDR_IN wsAddr = addr_gtow(addr);
    m_wsAddr = wsAddr;
    m_wsAddrSize = sizeof(wsAddr);

    if (::bind(m_wsSocket, (SOCKADDR*)&wsAddr, sizeof(wsAddr)) == SOCKET_ERROR)
    {
        err = "Socket binding failed. WSA error code: " + std::to_string(WSAGetLastError());
        if (printErrors) std::cout << err << "\n";
        return false;
    }

    return true;
}

bool Garnet::Socket::listen(int maxClients)
{
    if (::listen(m_wsSocket, maxClients) == SOCKET_ERROR)
    {
        err = "Socket listening failed. WSA error code: " + std::to_string(WSAGetLastError());
        if (printErrors) std::cout << err << "\n";
        return false;
    }
    return true;
}

Garnet::Socket Garnet::Socket::accept(bool* result)
{
    Socket retval;

    ::SOCKET acceptSocket = INVALID_SOCKET;
    acceptSocket = ::accept(m_wsSocket, (SOCKADDR*)&retval.m_wsAddr, &retval.m_wsAddrSize);
    if (acceptSocket == INVALID_SOCKET)
    {
        err = "Socket accept failed. WSA error code: " + std::to_string(WSAGetLastError());
        if (printErrors) std::cout << err << "\n";
        if (result != nullptr) *result = false;
        return retval;
    }

    retval.m_wsSocket = acceptSocket;
    retval.m_addr = addr_wtog(retval.m_wsAddr);
    retval.m_prot = m_prot;

    if (result != nullptr) *result = true;

    return retval;
}

bool Garnet::Socket::connect(Address addr)
{
    SOCKADDR_IN wsAddr = addr_gtow(addr);

    if (::connect(m_wsSocket, (SOCKADDR*)&wsAddr, sizeof(wsAddr)) == SOCKET_ERROR)
    {
        err = "Socket connect failed. WSA error code: " + std::to_string(WSAGetLastError());
        if (printErrors) std::cout << err << "\n";
        return false;
    }

    return true;
}

bool Garnet::Socket::send(void* data, int size)
{
    if (::send(m_wsSocket, (char*)data, size, 0) <= 0) return false;
    else return true;
}
void Garnet::Socket::receive(void* buffer, int bufferSize)
{
    ::recv(m_wsSocket, (char*)buffer, bufferSize, 0);
}
bool Garnet::Socket::sendTo(void* data, int size, Address to)
{
    SOCKADDR_IN wsTo = addr_gtow(to);
    if (::sendto(m_wsSocket, (char*)data, size, 0, (SOCKADDR*)&wsTo, sizeof(wsTo)) <= 0) return false;
    else return true;
}
void Garnet::Socket::receiveFrom(void* buffer, int bufferSize, Address* from)
{
    SOCKADDR_IN wsFrom;
    ::recvfrom(m_wsSocket, (char*)buffer, bufferSize, 0, (SOCKADDR*)&wsFrom, nullptr);
    *from = addr_wtog(wsFrom);
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
    return m_prot;
}

bool Garnet::Socket::isOpen() const
{
    return m_open;
}

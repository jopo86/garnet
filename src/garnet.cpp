#include "garnet.h"

#include <iostream>
#include <vector>

#ifdef GNET_OS_WINDOWS
    bool g_wsa_initialized = false;
    WSADATA g_wsa_data;
#endif

std::string g_err;
bool g_print_errors = false;
void* g_user_ptr = nullptr;

#ifdef GNET_OS_WINDOWS

    SOCKADDR_IN addr_to_backend(garnet::Address addr)
    {
        SOCKADDR_IN backend_addr;
        backend_addr.sin_family = AF_INET;
        inet_pton(AF_INET, addr.host.c_str(), &backend_addr.sin_addr.s_addr);
        backend_addr.sin_port = htons(addr.port);
        return backend_addr;
    }

    garnet::Address addr_from_backend(SOCKADDR_IN addr)
    {
        garnet::Address garnet_addr;
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr.s_addr, buf, sizeof(buf));
        garnet_addr.host = std::string(buf);
        garnet_addr.port = ntohs(addr.sin_port);
        return garnet_addr;
    }

#elif defined(GNET_OS_UNIX)

    sockaddr_in addr_to_backend(garnet::Address addr)
    {
        sockaddr_in backend_addr;
        memset(&backend_addr, 0, sizeof(backend_addr)); // Ensure struct is zeroed out
        backend_addr.sin_family = AF_INET;
        inet_pton(AF_INET, addr.host.c_str(), &backend_addr.sin_addr.s_addr);
        backend_addr.sin_port = htons(addr.port);
        return backend_addr;
    }

    garnet::Address addr_from_backend(sockaddr_in addr)
    {
        garnet::Address garnet_addr;
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr.s_addr, buf, sizeof(buf));
        garnet_addr.host = std::string(buf);
        garnet_addr.port = ntohs(addr.sin_port);
        return garnet_addr;
    }

#endif

void garnet::Address::operator=(const Address& other)
{
    host = other.host;
    port = other.port;
}

bool garnet::Address::operator==(const Address& other) const
{
    return (host == other.host && port == other.port);
}

int garnet::get_version_major()
{
    return GNET_VERSION_MAJOR;
}

int garnet::get_version_minor()
{
    return GNET_VERSION_MINOR;
}

int garnet::get_version_patch()
{
    return GNET_VERSION_PATCH;
}

std::string garnet::get_version_string()
{
    std::string v = std::to_string(GNET_VERSION_MAJOR) + "." + std::to_string(GNET_VERSION_MINOR) + "." + std::to_string(GNET_VERSION_PATCH);
    if (!GNET_STABLE)
    {
        if (GNET_DEV) v += "-dev";
        else if (GNET_ALPHA) v += "-alpha";
        else if (GNET_BETA) v += "-beta";
    }
    return v;
}

#ifdef GNET_OS_WINDOWS

    bool garnet::init(bool print_errors)
    {
        g_print_errors = print_errors;

        if (g_wsa_initialized)
        {
            g_err = "Initialization failed: already initialized";
            if (g_print_errors) std::cout << g_err << "\n";
            return false;
        }

        if (WSAStartup(MAKEWORD(2, 2), &g_wsa_data) != 0)
        {
            g_err = "Initialization failed: Winsock DLL not found";
            if (g_print_errors) std::cout << g_err << "\n";
            return false;
        }

        g_wsa_initialized = true;
        return true;
    }

    void garnet::terminate()
    {
        WSACleanup();
    }

#elif defined(GNET_OS_UNIX)

    bool garnet::init(bool print_errors)
    {
        g_print_errors = print_errors;
        return true;
    }

    void garnet::terminate() {}

#endif

const std::string& garnet::get_last_error()
{
    return g_err;
}

void garnet::set_user_ptr(void* ptr)
{
    g_user_ptr = ptr;
}

void* garnet::get_user_ptr()
{
    return g_user_ptr;
}

std::string garnet::hostname_to_ip(const std::string& hostname, bool* success)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  // Use IPv4
    hints.ai_socktype = SOCK_STREAM;  // Use TCP

    if (getaddrinfo(hostname.c_str(), nullptr, &hints, &res) != 0)
    {
        g_err = "Failed to resolve hostname: '" + hostname + "'";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return "";
    }

    char ip_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(((struct sockaddr_in*)res->ai_addr)->sin_addr), ip_str, sizeof(ip_str)) == nullptr)
    {
        freeaddrinfo(res);
        g_err = "Failed to convert address to string";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return "";
    }

    std::string ip_addr(ip_str);
    freeaddrinfo(res);
    if (success != nullptr) *success = true;
    return ip_addr;
}

#ifdef GNET_OS_WINDOWS

    garnet::Socket::Socket()
    {
        m_addr.host = "";
        m_addr.port = 0;
        m_proto = Protocol::Null;
        m_backend_socket = INVALID_SOCKET;
        m_backend_addr.sin_family = AF_INET;
        m_backend_addr_size = sizeof(m_backend_addr);
        m_open = false;
    }

    garnet::Socket::Socket(Protocol proto, bool* success)
    {
        m_addr.host = "";
        m_addr.port = 0;
        m_proto = proto;
        m_backend_socket = INVALID_SOCKET;
        m_backend_addr.sin_family = AF_INET;
        m_backend_addr_size = sizeof(m_backend_addr);

        if (m_proto == Protocol::Null)
        {
            g_err = "Socket creation failed: protocol cannot be null";
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return;
        }
        else if (m_proto == Protocol::Tcp)
        {
            m_backend_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (m_backend_socket == INVALID_SOCKET)
            {
                g_err = "Socket creation failed. WSA error code: " + std::to_string(WSAGetLastError());
                if (g_print_errors) std::cout << g_err << "\n";
                if (success != nullptr) *success = false;
                return;
            }
        }
        else if (m_proto == Protocol::Udp)
        {
            m_backend_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (m_backend_socket == INVALID_SOCKET)
            {
                g_err = "Socket creation failed. WSA error code: " + std::to_string(WSAGetLastError());
                if (g_print_errors) std::cout << g_err << "\n";
                if (success != nullptr) *success = false;
                return;
            }
        }

        int opt = 1;
        if (setsockopt(m_backend_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR)
        {
            g_err = "Socket creation incomplete: failed to set SO_REUSEADDR (not critical)";
            if (g_print_errors) std::cout << g_err << "\n";
        }

        m_open = true;
        if (success != nullptr) *success = true;
    }

    void garnet::Socket::bind(Address addr, bool* success)
    {
        m_addr = addr;
        SOCKADDR_IN backend_addr = addr_to_backend(addr);
        m_backend_addr = backend_addr;
        m_backend_addr_size = sizeof(backend_addr);

        if (::bind(m_backend_socket, (SOCKADDR*)&backend_addr, sizeof(backend_addr)) == SOCKET_ERROR)
        {
            g_err = "Socket binding failed. WSA error code: " + std::to_string(WSAGetLastError());
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        if (success != nullptr) *success = true;
        return;
    }

    void garnet::Socket::listen(int backlog, bool* success)
    {
        if (::listen(m_backend_socket, backlog) == SOCKET_ERROR)
        {
            g_err = "Socket listening failed. WSA error code: " + std::to_string(WSAGetLastError());
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        if (success != nullptr) *success = true;
        return;
    }

    garnet::Socket garnet::Socket::accept(bool* success)
    {
        Socket retval;

        ::SOCKET accept_socket = INVALID_SOCKET;
        accept_socket = ::accept(m_backend_socket, (SOCKADDR*)&retval.m_backend_addr, &retval.m_backend_addr_size);
        if (accept_socket == INVALID_SOCKET)
        {
            g_err = "Socket accept failed. WSA error code: " + std::to_string(WSAGetLastError());
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return retval;
        }

        retval.m_backend_socket = accept_socket;
        retval.m_addr = addr_from_backend(retval.m_backend_addr);
        retval.m_proto = m_proto;

        if (success != nullptr) *success = true;

        return retval;
    }

    void garnet::Socket::connect(Address addr, bool* success)
    {
        SOCKADDR_IN backend_addr;
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(addr.host.c_str(), std::to_string(addr.port).c_str(), &hints, &res) != 0)
        {
            g_err = "Socket connect failed: failed to resolve host";
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        backend_addr = *(SOCKADDR_IN*)res->ai_addr;
        freeaddrinfo(res);

        if (::connect(m_backend_socket, (SOCKADDR*)&backend_addr, sizeof(backend_addr)) == SOCKET_ERROR)
        {
            g_err = "Socket connect failed. WSA error code: " + std::to_string(WSAGetLastError());
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        if (success != nullptr) *success = true;
    }

    int garnet::Socket::send(void* data, int size, bool* success)
    {
        int num_bytes = ::send(m_backend_socket, (char*)data, size, 0);
        if (success != nullptr) *success = num_bytes != SOCKET_ERROR;
        return num_bytes;
    }

    int garnet::Socket::receive(void* buffer, int buffer_size, bool* success)
    {
        int num_bytes = ::recv(m_backend_socket, (char*)buffer, buffer_size, 0);
        if (success != nullptr) *success = num_bytes != SOCKET_ERROR;
        return num_bytes;
    }

    int garnet::Socket::send_to(void* data, int size, Address to, bool* success)
    {
        SOCKADDR_IN backend_to;
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        if (getaddrinfo(to.host.c_str(), std::to_string(to.port).c_str(), &hints, &res) != 0)
        {
            g_err = "send_to failed: failed to resolve host";
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return SOCKET_ERROR;
        }

        backend_to = *(SOCKADDR_IN*)res->ai_addr;
        freeaddrinfo(res);

        int num_bytes = ::sendto(m_backend_socket, (char*)data, size, 0, (SOCKADDR*)&backend_to, sizeof(backend_to));
        if (success != nullptr) *success = num_bytes != SOCKET_ERROR;
        return num_bytes;
    }

    int garnet::Socket::receive_from(void* buffer, int buffer_size, Address* from, bool* success)
    {
        SOCKADDR_IN backend_from;
        int backend_from_size = sizeof(backend_from);
        int num_bytes = ::recvfrom(m_backend_socket, (char*)buffer, buffer_size, 0, (SOCKADDR*)&backend_from, &backend_from_size);
        if (success != nullptr) *success = num_bytes != SOCKET_ERROR;
        if (from != nullptr && num_bytes != SOCKET_ERROR) *from = addr_from_backend(backend_from);
        return num_bytes;
    }

    void garnet::Socket::close()
    {
        closesocket(m_backend_socket);
        m_open = false;
    }

#elif defined(GNET_OS_UNIX)
    garnet::Socket::Socket()
    {
        m_addr.host = "";
        m_addr.port = 0;
        m_proto = Protocol::Null;
        m_backend_socket = -1;
        m_backend_addr.sin_family = AF_INET;
        m_backend_addr_size = sizeof(m_backend_addr);
        m_open = false;
    }

    garnet::Socket::Socket(Protocol proto, bool* success)
    {
        m_addr.host = "";
        m_addr.port = 0;
        m_proto = proto;
        m_backend_socket = -1;
        m_backend_addr.sin_family = AF_INET;
        m_backend_addr_size = sizeof(m_backend_addr);

        if (m_proto == Protocol::Null)
        {
            g_err = "Socket creation failed: protocol cannot be null";
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return;
        }
        else if (m_proto == Protocol::Tcp)
        {
            m_backend_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (m_backend_socket == -1)
            {
                g_err = "Socket creation failed. Error: " + std::string(strerror(errno));
                if (g_print_errors) std::cout << g_err << "\n";
                if (success != nullptr) *success = false;
                return;
            }
        }
        else if (m_proto == Protocol::Udp)
        {
            m_backend_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (m_backend_socket == -1)
            {
                g_err = "Socket creation failed. Error: " + std::string(strerror(errno));
                if (g_print_errors) std::cout << g_err << "\n";
                if (success != nullptr) *success = false;
                return;
            }
        }

        int opt = 1;
        if (setsockopt(m_backend_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        {
            g_err = "Socket creation incomplete: failed to set SO_REUSEADDR (not critical)";
            if (g_print_errors) std::cout << g_err << "\n";
        }

        m_open = true;
        if (success != nullptr) *success = true;
    }

    void garnet::Socket::bind(Address addr, bool* success)
    {
        m_addr = addr;
        sockaddr_in backend_addr = addr_to_backend(addr);
        m_backend_addr = backend_addr;
        m_backend_addr_size = sizeof(backend_addr);

        if (::bind(m_backend_socket, (sockaddr*)&backend_addr, sizeof(backend_addr)) == -1)
        {
            g_err = "Socket binding failed. Error: " + std::string(strerror(errno));
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        if (success != nullptr) *success = true;
        return;
    }

    void garnet::Socket::listen(int backlog, bool* success)
    {
        if (::listen(m_backend_socket, backlog) == -1)
        {
            g_err = "Socket listening failed. Error: " + std::string(strerror(errno));
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        if (success != nullptr) *success = true;
        return;
    }

    garnet::Socket garnet::Socket::accept(bool* success)
    {
        Socket retval;

        int accept_socket = -1;
        accept_socket = ::accept(m_backend_socket, (sockaddr*)&retval.m_backend_addr, &retval.m_backend_addr_size);
        if (accept_socket == -1)
        {
            g_err = "Socket accept failed. Error: " + std::string(strerror(errno));
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return retval;
        }

        retval.m_backend_socket = accept_socket;
        retval.m_addr = addr_from_backend(retval.m_backend_addr);
        retval.m_proto = m_proto;

        if (success != nullptr) *success = true;

        return retval;
    }

    void garnet::Socket::connect(Address addr, bool* success)
    {
        sockaddr_in backend_addr;
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(addr.host.c_str(), std::to_string(addr.port).c_str(), &hints, &res) != 0)
        {
            g_err = "Socket connect failed: failed to resolve host";
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        backend_addr = *(sockaddr_in*)res->ai_addr;
        freeaddrinfo(res);

        if (::connect(m_backend_socket, (sockaddr*)&backend_addr, sizeof(backend_addr)) == -1)
        {
            g_err = "Socket connect failed. Error: " + std::string(strerror(errno));
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return;
        }

        if (success != nullptr) *success = true;
    }

    int garnet::Socket::send(void* data, int size, bool* success)
    {
        int num_bytes = ::send(m_backend_socket, (char*)data, size, 0);
        if (success != nullptr) *success = num_bytes != -1;
        return num_bytes;
    }

    int garnet::Socket::receive(void* buffer, int buffer_size, bool* success)
    {
        int num_bytes = ::recv(m_backend_socket, (char*)buffer, buffer_size, 0);
        if (success != nullptr) *success = num_bytes != -1;
        return num_bytes;
    }

    int garnet::Socket::send_to(void* data, int size, Address to, bool* success)
    {
        sockaddr_in backend_to;
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        if (getaddrinfo(to.host.c_str(), std::to_string(to.port).c_str(), &hints, &res) != 0)
        {
            g_err = "send_to failed: failed to resolve host";
            if (g_print_errors) std::cout << g_err << "\n";
            if (success != nullptr) *success = false;
            return -1;
        }

        backend_to = *(sockaddr_in*)res->ai_addr;
        freeaddrinfo(res);

        int num_bytes = ::sendto(m_backend_socket, (char*)data, size, 0, (sockaddr*)&backend_to, sizeof(backend_to));
        if (success != nullptr) *success = num_bytes != -1;
        return num_bytes;
    }

    int garnet::Socket::receive_from(void* buffer, int buffer_size, Address* from, bool* success)
    {
        sockaddr_in backend_from;
        socklen_t backend_from_size = sizeof(backend_from);
        int num_bytes = ::recvfrom(m_backend_socket, (char*)buffer, buffer_size, 0, (sockaddr*)&backend_from, &backend_from_size);
        if (success != nullptr) *success = num_bytes != -1;
        if (from != nullptr && num_bytes != -1) *from = addr_from_backend(backend_from);
        return num_bytes;
    }

    void garnet::Socket::close()
    {
        ::close(m_backend_socket);
        m_open = false;
    }

#endif

const garnet::Address& garnet::Socket::get_address() const
{
    return m_addr;
}

const garnet::Protocol& garnet::Socket::get_protocol() const
{
    return m_proto;
}

bool garnet::Socket::is_open() const
{
    return m_open;
}

garnet::ServerTcp::ServerTcp()
{
    m_addr.host = "";
    m_addr.port = 0;
    m_buf_size = 256;
    m_num_clients = 0;
    m_open = false;
    m_receive_callback = nullptr;
    m_client_connect_callback = nullptr;
    m_client_disconnect_callback = nullptr;
}

garnet::ServerTcp::ServerTcp(Address addr, bool* success)
{
    m_addr = addr;
    bool success_a, success_b;
    m_socket = Socket(Protocol::Tcp, &success_a);
    m_socket.bind(addr, &success_b);
    m_buf_size = 256;
    m_num_clients = 0;
    m_open = false;
    m_receive_callback = nullptr;
    m_client_connect_callback = nullptr;
    m_client_disconnect_callback = nullptr;

    if (success != nullptr) *success = success_a && success_b;
}

void garnet::ServerTcp::open(int backlog, bool* success)
{
    if (m_open)
    {
        g_err = "Failed to open ServerTcp: already open";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_open = true;
    bool success_a;
    m_socket.listen(backlog, &success_a);
    if (success != nullptr) *success = success_a;
    if (success_a)
    {
        m_accepting = std::thread(&garnet::ServerTcp::accept, this);
    }
    else m_open = false;
}

void garnet::ServerTcp::send(void* data, int size, Address client_addr, bool* success)
{
    m_client_map[client_addr].send(data, size, success);
}

void garnet::ServerTcp::close(bool* success)
{
    if (!m_open)
    {
        g_err = "Failed to close ServerTcp: not open yet or already closed";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_socket.close();
    for (Address& accepted_addr : m_client_addrs)
    {
        m_client_map[accepted_addr].close();
    }
    m_open = false;
    m_accepting.detach();
    for (std::thread& receiving : m_receivings) receiving.detach();
    m_receivings.clear();
    m_client_addrs_mtx.lock();
    m_client_map_mtx.lock();
    m_client_addrs.clear();
    m_client_map.clear();
    m_client_addrs_mtx.unlock();
    m_client_map_mtx.unlock();
    if (success != nullptr) *success = true;
}

bool garnet::ServerTcp::is_open() const
{
    return m_open;
}

int garnet::ServerTcp::get_buffer_size() const
{
    return m_buf_size;
}

int garnet::ServerTcp::get_num_clients() const
{
    return m_num_clients;
}

garnet::Socket& garnet::ServerTcp::get_client_accepted_socket(Address client_addr)
{
    return m_client_map[client_addr];
}

const std::list<garnet::Address>& garnet::ServerTcp::get_client_addresses() const
{
    return m_client_addrs;
}

const std::unordered_map<garnet::Address, garnet::Socket>& garnet::ServerTcp::get_client_map() const
{
    return m_client_map;
}

void garnet::ServerTcp::set_buffer_size(int size)
{
    m_buf_size = size;
}

void garnet::ServerTcp::set_receive_callback(void(*callback)(void* buffer, int buffer_size, int actual_size, Address from_client_addr))
{
    m_receive_callback = callback;
}

void garnet::ServerTcp::set_client_connect_callback(void(*callback)(Address client_addr))
{
    m_client_connect_callback = callback;
}

void garnet::ServerTcp::set_client_disconnect_callback(void(*callback)(Address client_addr))
{
    m_client_disconnect_callback = callback;
}

void garnet::ServerTcp::accept()
{
    while (m_open)
    {
        bool success;
        Socket accepted_socket;

        bool prev_print_errors = g_print_errors;
        g_print_errors = false;
        accepted_socket = m_socket.accept(&success);
        g_print_errors = prev_print_errors;
        if (!success) continue;
        else
        {
            m_client_addrs_mtx.lock();
            m_client_map_mtx.lock();
            m_client_addrs.push_back(accepted_socket.get_address());
            m_client_map.insert({ accepted_socket.get_address(), accepted_socket });
            m_client_addrs_mtx.unlock();
            m_client_map_mtx.unlock();

            m_receivings.push_back(std::thread(&garnet::ServerTcp::receive, this, accepted_socket));
            m_num_clients = m_num_clients + 1;

            if (m_client_connect_callback != nullptr) m_client_connect_callback(accepted_socket.get_address());
        }
    }
}

void garnet::ServerTcp::receive(Socket accepted_socket)
{
    while (m_open)
    {
        if (m_receive_callback == nullptr) continue;

        char* buf = new char[m_buf_size];
        bool recv_success;
        int num_bytes = accepted_socket.receive(buf, m_buf_size, &recv_success);
        if (!recv_success)
        {
            // client disconnected
            m_client_addrs_mtx.lock();
            m_client_map_mtx.lock();
            m_client_addrs.remove(accepted_socket.get_address());
            m_client_map.erase(accepted_socket.get_address());
            m_client_addrs_mtx.unlock();
            m_client_map_mtx.unlock();
            m_num_clients = m_num_clients - 1;

            if (m_client_disconnect_callback != nullptr) m_client_disconnect_callback(accepted_socket.get_address());

            delete[] buf;
            break;
        }

        m_receive_callback(buf, m_buf_size, num_bytes, accepted_socket.get_address());
    }
}

garnet::ServerUdp::ServerUdp()
{
    m_addr.host = "";
    m_addr.port = 0;
    m_buf_size = 256;
    m_open = false;
    m_receive_callback = nullptr;
}

garnet::ServerUdp::ServerUdp(Address addr, bool* success)
{
    m_addr = addr;
    bool success_a, success_b;
    m_socket = Socket(Protocol::Udp, &success_a);
    m_socket.bind(addr, &success_b);
    m_buf_size = 256;
    m_open = false;
    m_receive_callback = nullptr;

    if (success != nullptr) *success = success_a && success_b;
}

void garnet::ServerUdp::open(bool* success)
{
    if (m_open)
    {
        g_err = "Failed to open ServerUdp: already open";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_open = true;
    m_receiving = std::thread(&garnet::ServerUdp::receive, this);
    if (success != nullptr) *success = true;
}

void garnet::ServerUdp::send(void* data, int size, Address addr, bool* success)
{
    m_socket.send_to(data, size, addr, success);
}

void garnet::ServerUdp::close(bool* success)
{
    if (!m_open)
    {
        g_err = "Failed to close ServerUdp: not open yet or already closed";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_socket.close();
    m_open = false;
    m_receiving.detach();
    if (success != nullptr) *success = true;
}

bool garnet::ServerUdp::is_open() const
{
    return m_open;
}

int garnet::ServerUdp::get_buffer_size() const
{
    return m_buf_size;
}

void garnet::ServerUdp::set_buffer_size(int size)
{
    m_buf_size = size;
}

void garnet::ServerUdp::set_receive_callback(void (*callback)(void* buffer, int buffer_size, int actual_size, Address from_addr))
{
    m_receive_callback = callback;
}

void garnet::ServerUdp::receive()
{
    while (m_open)
    {
        if (m_receive_callback == nullptr) continue;

        bool recv_success;
        Address from;
        char* buf = new char[m_buf_size];
        int num_bytes = m_socket.receive_from(buf, m_buf_size, &from, &recv_success);
        if (!recv_success)
        {
            delete[] buf;
            continue;
        }

        m_receive_callback(buf, m_buf_size, num_bytes, from);
    }
}

garnet::ClientTcp::ClientTcp()
{
    m_buf_size = 256;
    m_receive_callback = nullptr;
    m_connected = false;
}

garnet::ClientTcp::ClientTcp(char dummy, bool* success)
{
    m_buf_size = 256;
    m_receive_callback = nullptr;
    m_connected = false;
    m_socket = Socket(Protocol::Tcp, success);
}

void garnet::ClientTcp::connect(Address server_addr, bool* success)
{
    if (m_connected)
    {
        g_err = "Failed to connect ClientTcp: already connected";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    bool success_a;
    m_socket.connect(server_addr, &success_a);
    if (success_a)
    {
        m_connected = true;
        if (success != nullptr) *success = true;
    }
    else
    {
        if (success != nullptr) *success = false;
        return;
    }

    m_receiving = std::thread(&garnet::ClientTcp::receive, this);
}

void garnet::ClientTcp::send(void* data, int size, bool* success)
{
    m_socket.send(data, size, success);
}

void garnet::ClientTcp::disconnect(bool* success)
{
    if (!m_connected)
    {
        g_err = "Failed to disconnect ClientTcp: not connected yet or already disconnected";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_connected = false;
    m_socket.close();
    m_receiving.detach();
    if (success != nullptr) *success = true;
}

bool garnet::ClientTcp::is_connected() const
{
    return m_connected;
}

int garnet::ClientTcp::get_buffer_size() const
{
    return m_buf_size;
}

void garnet::ClientTcp::set_buffer_size(int buffer_size)
{
    m_buf_size = buffer_size;
}

void garnet::ClientTcp::set_receive_callback(void (*callback)(void* buffer, int buffer_size, int actual_size))
{
    m_receive_callback = callback;
}

void garnet::ClientTcp::receive()
{
    while (m_connected)
    {
        if (m_receive_callback == nullptr) continue;

        char* buf = new char[m_buf_size];
        bool recv_success;
        int num_bytes = m_socket.receive(buf, m_buf_size, &recv_success);
        if (!recv_success)
        {
            delete[] buf;
            continue;
        }

        m_receive_callback(buf, m_buf_size, num_bytes);
    }
}

garnet::ClientUdp::ClientUdp()
{
    m_buf_size = 256;
    m_receive_callback = nullptr;
    m_connected = false;
}

garnet::ClientUdp::ClientUdp(char dummy, bool* success)
{
    m_buf_size = 256;
    m_receive_callback = nullptr;
    m_connected = true;
    m_socket = Socket(Protocol::Udp, success);

    m_receiving = std::thread(&garnet::ClientUdp::receive, this);
}

void garnet::ClientUdp::send(void* data, int size, Address addr, bool* success)
{
    m_socket.send_to(data, size, addr, success);
}

void garnet::ClientUdp::disconnect(bool* success)
{
    if (!m_connected)
    {
        g_err = "Failed to disconnect ClientUdp: not connected yet or already disconnected";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_connected = false;
    m_socket.close();
    m_receiving.detach();
    if (success != nullptr) *success = true;
}

bool garnet::ClientUdp::is_connected() const
{
    return m_connected;
}

int garnet::ClientUdp::get_buffer_size() const
{
    return m_buf_size;
}

void garnet::ClientUdp::set_buffer_size(int size)
{
    m_buf_size = size;
}

void garnet::ClientUdp::set_receive_callback(void (*callback)(void* buffer, int buffer_size, int actual_size, Address from_server_address))
{
    m_receive_callback = callback;
}

void garnet::ClientUdp::receive()
{
    while (m_connected)
    {
        if (m_receive_callback == nullptr) continue;

        bool recv_success;
        Address from;
        char* buf = new char[m_buf_size];
        int num_bytes = m_socket.receive_from(buf, m_buf_size, &from, &recv_success);
        if (!recv_success)
        {
            delete[] buf;
            continue;
        }

        m_receive_callback(buf, m_buf_size, num_bytes, from);
    }
}

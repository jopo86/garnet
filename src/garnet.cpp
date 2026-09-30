#include <garnet/garnet.hpp>

#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>

#ifdef GNET_OS_WINDOWS
    bool g_wsa_initialized = false;
    WSADATA g_wsa_data;
#endif

thread_local std::string g_err;
std::atomic<bool> g_print_errors = false;
void* g_user_ptr = nullptr;

#ifdef GNET_OS_WINDOWS

    SOCKADDR_IN addr_to_backend(gnet::Address addr)
    {
        SOCKADDR_IN backend_addr;
        backend_addr.sin_family = AF_INET;
        inet_pton(AF_INET, addr.host.c_str(), &backend_addr.sin_addr.s_addr);
        backend_addr.sin_port = htons(addr.port);
        return backend_addr;
    }

    gnet::Address addr_from_backend(SOCKADDR_IN addr)
    {
        gnet::Address gnet_addr;
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr.s_addr, buf, sizeof(buf));
        gnet_addr.host = std::string(buf);
        gnet_addr.port = ntohs(addr.sin_port);
        return gnet_addr;
    }

#elif defined(GNET_OS_UNIX)

    sockaddr_in addr_to_backend(gnet::Address addr)
    {
        sockaddr_in backend_addr;
        memset(&backend_addr, 0, sizeof(backend_addr)); // Ensure struct is zeroed out
        backend_addr.sin_family = AF_INET;
        inet_pton(AF_INET, addr.host.c_str(), &backend_addr.sin_addr.s_addr);
        backend_addr.sin_port = htons(addr.port);
        return backend_addr;
    }

    gnet::Address addr_from_backend(sockaddr_in addr)
    {
        gnet::Address gnet_addr;
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr.s_addr, buf, sizeof(buf));
        gnet_addr.host = std::string(buf);
        gnet_addr.port = ntohs(addr.sin_port);
        return gnet_addr;
    }

#endif

bool gnet::Address::operator==(const Address& other) const
{
    return (host == other.host && port == other.port);
}

int gnet::get_version_major()
{
    return GNET_VERSION_MAJOR;
}

int gnet::get_version_minor()
{
    return GNET_VERSION_MINOR;
}

int gnet::get_version_patch()
{
    return GNET_VERSION_PATCH;
}

std::string gnet::get_version_string()
{
    return std::to_string(GNET_VERSION_MAJOR) + "." + std::to_string(GNET_VERSION_MINOR) + "." + std::to_string(GNET_VERSION_PATCH);
}

#ifdef GNET_OS_WINDOWS

    bool gnet::init(bool print_errors)
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

    void gnet::terminate()
    {
        WSACleanup();
    }

#elif defined(GNET_OS_UNIX)

    bool gnet::init(bool print_errors)
    {
        g_print_errors = print_errors;
        return true;
    }

    void gnet::terminate() {}

#endif

const std::string& gnet::get_last_error()
{
    return g_err;
}

void gnet::set_user_ptr(void* ptr)
{
    g_user_ptr = ptr;
}

void* gnet::get_user_ptr()
{
    return g_user_ptr;
}

std::string gnet::hostname_to_ip(const std::string& hostname, bool* success)
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

    gnet::Socket::Socket()
    {
        m_addr.host = "";
        m_addr.port = 0;
        m_proto = Protocol::Null;
        m_backend_socket = INVALID_SOCKET;
        m_backend_addr.sin_family = AF_INET;
        m_backend_addr_size = sizeof(m_backend_addr);
        m_open = false;
    }

    gnet::Socket::Socket(Protocol proto, bool* success)
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

    void gnet::Socket::bind(Address addr, bool* success)
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

    void gnet::Socket::listen(int backlog, bool* success)
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

    gnet::Socket gnet::Socket::accept(bool* success)
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

    void gnet::Socket::connect(Address addr, bool* success)
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

    int gnet::Socket::send(void* data, int size, bool* success)
    {
        int num_bytes = ::send(m_backend_socket, (char*)data, size, 0);
        if (success != nullptr) *success = num_bytes != SOCKET_ERROR;
        return num_bytes;
    }

    int gnet::Socket::receive(void* buffer, int buffer_size, bool* success)
    {
        int num_bytes = ::recv(m_backend_socket, (char*)buffer, buffer_size, 0);
        if (success != nullptr) *success = num_bytes != SOCKET_ERROR;
        return num_bytes;
    }

    int gnet::Socket::send_to(void* data, int size, Address to, bool* success)
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

    int gnet::Socket::receive_from(void* buffer, int buffer_size, Address* from, bool* success)
    {
        SOCKADDR_IN backend_from;
        int backend_from_size = sizeof(backend_from);
        int num_bytes = ::recvfrom(m_backend_socket, (char*)buffer, buffer_size, 0, (SOCKADDR*)&backend_from, &backend_from_size);
        if (success != nullptr) *success = num_bytes != SOCKET_ERROR;
        if (from != nullptr && num_bytes != SOCKET_ERROR) *from = addr_from_backend(backend_from);
        return num_bytes;
    }

    void gnet::Socket::close()
    {
        shutdown(m_backend_socket, SD_BOTH);
        closesocket(m_backend_socket);
        m_open = false;
    }

#elif defined(GNET_OS_UNIX)
    gnet::Socket::Socket()
    {
        m_addr.host = "";
        m_addr.port = 0;
        m_proto = Protocol::Null;
        m_backend_socket = -1;
        m_backend_addr.sin_family = AF_INET;
        m_backend_addr_size = sizeof(m_backend_addr);
        m_open = false;
    }

    gnet::Socket::Socket(Protocol proto, bool* success)
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

    void gnet::Socket::bind(Address addr, bool* success)
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

    void gnet::Socket::listen(int backlog, bool* success)
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

    gnet::Socket gnet::Socket::accept(bool* success)
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

    void gnet::Socket::connect(Address addr, bool* success)
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

    int gnet::Socket::send(void* data, int size, bool* success)
    {
        int num_bytes = ::send(m_backend_socket, (char*)data, size, 0);
        if (success != nullptr) *success = num_bytes != -1;
        return num_bytes;
    }

    int gnet::Socket::receive(void* buffer, int buffer_size, bool* success)
    {
        int num_bytes = ::recv(m_backend_socket, (char*)buffer, buffer_size, 0);
        if (success != nullptr) *success = num_bytes != -1;
        return num_bytes;
    }

    int gnet::Socket::send_to(void* data, int size, Address to, bool* success)
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

    int gnet::Socket::receive_from(void* buffer, int buffer_size, Address* from, bool* success)
    {
        sockaddr_in backend_from;
        socklen_t backend_from_size = sizeof(backend_from);
        int num_bytes = ::recvfrom(m_backend_socket, (char*)buffer, buffer_size, 0, (sockaddr*)&backend_from, &backend_from_size);
        if (success != nullptr) *success = num_bytes != -1;
        if (from != nullptr && num_bytes != -1) *from = addr_from_backend(backend_from);
        return num_bytes;
    }

    void gnet::Socket::close()
    {
        shutdown(m_backend_socket, SHUT_RDWR);
        ::close(m_backend_socket);
        m_open = false;
    }

#endif

const gnet::Address& gnet::Socket::get_address() const
{
    return m_addr;
}

const gnet::Protocol& gnet::Socket::get_protocol() const
{
    return m_proto;
}

bool gnet::Socket::is_open() const
{
    return m_open;
}

// TCP is a byte stream, so ServerTcp and ClientTcp frame each message with a 4-byte big-endian length prefix.

static bool send_all(gnet::Socket& socket, const char* data, int size)
{
    int sent = 0;
    while (sent < size)
    {
        int num_bytes = socket.send((void*)(data + sent), size - sent);
        if (num_bytes <= 0) return false;
        sent += num_bytes;
    }
    return true;
}

static bool receive_all(gnet::Socket& socket, char* buffer, int size)
{
    int received = 0;
    while (received < size)
    {
        int num_bytes = socket.receive(buffer + received, size - received);
        if (num_bytes <= 0) return false; // 0 means the peer closed the connection
        received += num_bytes;
    }
    return true;
}

static bool send_message(gnet::Socket& socket, void* data, int size)
{
    if (size < 0) return false;

    // header and payload go out in one buffer so concurrent senders can't interleave them
    std::vector<char> packet(4 + size);
    uint32_t len = htonl((uint32_t)size);
    memcpy(packet.data(), &len, 4);
    if (size > 0) memcpy(packet.data() + 4, data, size);
    return send_all(socket, packet.data(), (int)packet.size());
}

// Returns the full size of the message (which may exceed buffer_size; the excess is discarded), or -1 if the connection closed or errored.
static int receive_message(gnet::Socket& socket, char* buffer, int buffer_size)
{
    uint32_t len_net;
    if (!receive_all(socket, (char*)&len_net, 4)) return -1;
    uint32_t len = ntohl(len_net);
    if (len > INT32_MAX) return -1;

    int to_copy = (int)len < buffer_size ? (int)len : buffer_size;
    if (to_copy > 0 && !receive_all(socket, buffer, to_copy)) return -1;

    int remaining = (int)len - to_copy;
    char discard[512];
    while (remaining > 0)
    {
        int chunk = remaining < (int)sizeof(discard) ? remaining : (int)sizeof(discard);
        if (!receive_all(socket, discard, chunk)) return -1;
        remaining -= chunk;
    }

    return (int)len;
}

gnet::ServerTcp::ServerTcp(Address addr, bool* success)
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

void gnet::ServerTcp::open(int backlog, bool* success)
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
        m_accepting = std::thread(&gnet::ServerTcp::accept, this);
    }
    else m_open = false;
}

void gnet::ServerTcp::send(void* data, int size, Address client_addr, bool* success)
{
    std::lock_guard lock(m_client_map_mtx);
    auto it = m_client_map.find(client_addr);
    bool sent = it != m_client_map.end() && send_message(it->second, data, size);
    if (success != nullptr) *success = sent;
}


void gnet::ServerTcp::close(bool* success)
{
    if (!m_open)
    {
        g_err = "Failed to close ServerTcp: not open yet or already closed";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_open = false;
    m_socket.close();
    if (m_accepting.get_id() == std::this_thread::get_id()) m_accepting.detach();
    else m_accepting.join();

    m_client_addrs_mtx.lock();
    m_client_map_mtx.lock();
    for (auto& [addr, sock] : m_client_map) sock.close();
    m_client_addrs.clear();
    m_client_map.clear();
    m_client_addrs_mtx.unlock();
    m_client_map_mtx.unlock();
    m_receivings_mtx.lock();
    for (std::thread& receiving : m_receivings)
    {
        if (receiving.get_id() == std::this_thread::get_id()) receiving.detach();
        else receiving.join();
    }
    m_receivings.clear();
    m_receivings_mtx.unlock();
    if (success != nullptr) *success = true;
}

bool gnet::ServerTcp::is_open() const
{
    return m_open;
}

int gnet::ServerTcp::get_buffer_size() const
{
    return m_buf_size;
}

int gnet::ServerTcp::get_num_clients() const
{
    return m_num_clients;
}

gnet::Socket gnet::ServerTcp::get_client_accepted_socket(Address client_addr, bool* success)
{
    std::lock_guard lock(m_client_map_mtx);
    auto it = m_client_map.find(client_addr);
    if (success != nullptr) *success = it != m_client_map.end();
    return it != m_client_map.end() ? it->second : Socket();
}

const std::list<gnet::Address> gnet::ServerTcp::get_client_addresses()
{
    m_client_addrs_mtx.lock();
    auto ret = m_client_addrs;
    m_client_addrs_mtx.unlock();
    return ret;
}

const std::unordered_map<gnet::Address, gnet::Socket> gnet::ServerTcp::get_client_map()
{
    m_client_map_mtx.lock();
    auto ret = m_client_map;
    m_client_map_mtx.unlock();
    return ret;
}

void gnet::ServerTcp::set_buffer_size(int size)
{
    m_buf_size = size;
}

void gnet::ServerTcp::set_receive_callback(void(*callback)(void* buffer, int buffer_size, int actual_size, Address from_client_addr))
{
    m_receive_callback = callback;
}

void gnet::ServerTcp::set_client_connect_callback(void(*callback)(Address client_addr))
{
    m_client_connect_callback = callback;
}

void gnet::ServerTcp::set_client_disconnect_callback(void(*callback)(Address client_addr))
{
    m_client_disconnect_callback = callback;
}

gnet::ServerTcp::~ServerTcp()
{
    if (m_open) close();
}

void gnet::ServerTcp::accept()
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

            m_receivings_mtx.lock();
            m_receivings.push_back(std::thread(&gnet::ServerTcp::receive, this, accepted_socket));
            m_receivings_mtx.unlock();
            m_num_clients = m_num_clients + 1;

            auto cb = m_client_connect_callback.load();
            if (cb != nullptr) cb(accepted_socket.get_address());
        }
    }
}

void gnet::ServerTcp::receive(Socket accepted_socket)
{
    while (m_open)
    {
        char* buf = new char[m_buf_size];
        int num_bytes = receive_message(accepted_socket, buf, m_buf_size);
        if (num_bytes < 0)
        {
            // client disconnected
            m_client_addrs_mtx.lock();
            m_client_map_mtx.lock();
            m_client_addrs.remove(accepted_socket.get_address());
            bool owned = m_client_map.erase(accepted_socket.get_address()) > 0;
            if (owned) accepted_socket.close();   // otherwise close() already closed it
            m_client_addrs_mtx.unlock();
            m_client_map_mtx.unlock();

            m_num_clients = m_num_clients - 1;

            auto cb = m_client_disconnect_callback.load();
            if (cb != nullptr) cb(accepted_socket.get_address());

            delete[] buf;
            break;
        }

        auto cb = m_receive_callback.load();
        if (cb != nullptr) cb(buf, m_buf_size, num_bytes, accepted_socket.get_address());
        else delete[] buf;
    }
}

gnet::ServerUdp::ServerUdp(Address addr, bool* success)
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

void gnet::ServerUdp::open(bool* success)
{
    if (m_open)
    {
        g_err = "Failed to open ServerUdp: already open";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_open = true;
    m_receiving = std::thread(&gnet::ServerUdp::receive, this);
    if (success != nullptr) *success = true;
}

void gnet::ServerUdp::send(void* data, int size, Address addr, bool* success)
{
    m_socket.send_to(data, size, addr, success);
}

void gnet::ServerUdp::close(bool* success)
{
    if (!m_open)
    {
        g_err = "Failed to close ServerUdp: not open yet or already closed";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_open = false;
    m_socket.close();
    if (m_receiving.get_id() == std::this_thread::get_id()) m_receiving.detach();
    else m_receiving.join();
    if (success != nullptr) *success = true;
}

bool gnet::ServerUdp::is_open() const
{
    return m_open;
}

int gnet::ServerUdp::get_buffer_size() const
{
    return m_buf_size;
}

void gnet::ServerUdp::set_buffer_size(int size)
{
    m_buf_size = size;
}

void gnet::ServerUdp::set_receive_callback(void (*callback)(void* buffer, int buffer_size, int actual_size, Address from_addr))
{
    m_receive_callback = callback;
}

gnet::ServerUdp::~ServerUdp()
{
    if (m_open) close();
}

void gnet::ServerUdp::receive()
{
    while (m_open)
    {
        bool recv_success;
        Address from;
        char* buf = new char[m_buf_size];
        int num_bytes = m_socket.receive_from(buf, m_buf_size, &from, &recv_success);
        if (!recv_success)
        {
            delete[] buf;
            continue;
        }

        auto cb = m_receive_callback.load();
        if (cb != nullptr) cb(buf, m_buf_size, num_bytes, from);
        else delete[] buf;
    }
}

gnet::ClientTcp::ClientTcp(bool* success)
{
    m_buf_size = 256;
    m_receive_callback = nullptr;
    m_disconnect_callback = nullptr;
    m_connected = false;
    m_socket = Socket(Protocol::Tcp, success);
}

void gnet::ClientTcp::connect(Address server_addr, bool* success)
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

    m_receiving = std::thread(&gnet::ClientTcp::receive, this);
}

void gnet::ClientTcp::send(void* data, int size, bool* success)
{
    bool sent = send_message(m_socket, data, size);
    if (success != nullptr) *success = sent;
}

void gnet::ClientTcp::disconnect(bool* success)
{
    // m_connected may already be false if the server closed the connection, but the socket and thread still need cleaning up
    if (!m_receiving.joinable())
    {
        g_err = "Failed to disconnect ClientTcp: not connected yet or already disconnected";
        if (g_print_errors) std::cout << g_err << "\n";
        if (success != nullptr) *success = false;
        return;
    }

    m_connected = false;
    m_socket.close();
    if (m_receiving.get_id() == std::this_thread::get_id()) m_receiving.detach();
    else m_receiving.join();
    if (success != nullptr) *success = true;
}

bool gnet::ClientTcp::is_connected() const
{
    return m_connected;
}

int gnet::ClientTcp::get_buffer_size() const
{
    return m_buf_size;
}

void gnet::ClientTcp::set_buffer_size(int buffer_size)
{
    m_buf_size = buffer_size;
}

void gnet::ClientTcp::set_receive_callback(void (*callback)(void* buffer, int buffer_size, int actual_size))
{
    m_receive_callback = callback;
}

void gnet::ClientTcp::set_disconnect_callback(void (*callback)())
{
    m_disconnect_callback = callback;
}

gnet::ClientTcp::~ClientTcp()
{
    if (m_receiving.joinable()) disconnect();
}

void gnet::ClientTcp::receive()
{
    while (m_connected)
    {
        char* buf = new char[m_buf_size];
        int num_bytes = receive_message(m_socket, buf, m_buf_size);
        if (num_bytes < 0)
        {
            // server closed the connection (or disconnect() closed the socket)
            delete[] buf;
            // only report it if the server dropped us, not if disconnect() was called
            bool was_connected = m_connected.exchange(false);
            auto cb = m_disconnect_callback.load();
            if (was_connected && cb != nullptr) cb();
            break;
        }

        auto cb = m_receive_callback.load();
        if (cb != nullptr) cb(buf, m_buf_size, num_bytes);
        else delete[] buf;
    }
}

gnet::ClientUdp::ClientUdp(bool* success)
{
    m_buf_size = 256;
    m_receive_callback = nullptr;
    m_connected = true;
    m_socket = Socket(Protocol::Udp, success);

    m_receiving = std::thread(&gnet::ClientUdp::receive, this);
}

void gnet::ClientUdp::send(void* data, int size, Address addr, bool* success)
{
    m_socket.send_to(data, size, addr, success);
}

void gnet::ClientUdp::disconnect(bool* success)
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
    if (m_receiving.get_id() == std::this_thread::get_id()) m_receiving.detach();
    else m_receiving.join();
    if (success != nullptr) *success = true;
}

bool gnet::ClientUdp::is_connected() const
{
    return m_connected;
}

int gnet::ClientUdp::get_buffer_size() const
{
    return m_buf_size;
}

void gnet::ClientUdp::set_buffer_size(int size)
{
    m_buf_size = size;
}

void gnet::ClientUdp::set_receive_callback(void (*callback)(void* buffer, int buffer_size, int actual_size, Address from_server_address))
{
    m_receive_callback = callback;
}

gnet::ClientUdp::~ClientUdp()
{
    if (m_connected) disconnect();
}

void gnet::ClientUdp::receive()
{
    while (m_connected)
    {
        bool recv_success;
        Address from;
        char* buf = new char[m_buf_size];
        int num_bytes = m_socket.receive_from(buf, m_buf_size, &from, &recv_success);
        if (!recv_success)
        {
            delete[] buf;
            continue;
        }

        auto cb = m_receive_callback.load();
        if (cb != nullptr) cb(buf, m_buf_size, num_bytes, from);
        else delete[] buf;
    }
}

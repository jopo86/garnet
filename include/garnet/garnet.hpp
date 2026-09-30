#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <list>

#define GNET_VERSION_MAJOR  1
#define GNET_VERSION_MINOR  0
#define GNET_VERSION_PATCH  0

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
    #define GNET_OS_WINDOWS
#elif defined(__unix__) || defined(__unix) || defined(unix) || defined(__APPLE__) || defined(__MACH__)
    #define GNET_OS_UNIX
    #if defined(__APPLE__) || defined(__MACH__)
        #define GNET_OS_MAC
    #elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
        #define GNET_OS_LINUX
    #else
        #define GNET_OS_UNKNOWN_UNIX
    #endif
#else
    #define GNET_OS_UNKNOWN
#endif

typedef unsigned short ushort;

#ifdef GNET_OS_WINDOWS
    #include <winsock2.h>
    #include <ws2tcpip.h>

#elif defined(GNET_OS_UNIX)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <cstring>
    #include <unistd.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <netdb.h>

#endif

/*
    @brief The Garnet library namespace.
    Garnet is a small, cross-platform C++ networking library providing both high-level server/client architecture and low-level socket operations.
 */
namespace gnet
{
    /*
        @brief Gets the major version of the library.
        @return The major version of the library (X.y.z).
     */
    int get_version_major();

    /*
        @brief Gets the minor version of the library.
        @return The minor version of the library (x.Y.z).
     */
    int get_version_minor();

    /*
        @brief Gets the patch version of the library.
        @return The patch version of the library (x.y.Z).
     */
    int get_version_patch();

    /*
        @brief Gets the version of the library as a string.
        @return The version of the library as a string in the format 'x.y.z' or 'x.y.z-alpha/beta'.
     */
    std::string get_version_string();

    /*
        @brief Initializes the library.
        This function is required on Windows, but not on Unix systems.
        @param print_errors If true, errors will always be printed to the console. Helpful for quick debugging.
        @return True if the library was successfully initialized, false otherwise. Always returns true on Unix systems.
     */
    bool init(bool print_errors = false);

    /*
        @brief Terminates the library.
        This function is required on Windows, but not on Unix systems.
        It literally does nothing on Unix systems.
     */
    void terminate();

    /*
        @brief Gets the last error message.
        @return The last error message as a string.
     */
    const std::string& get_last_error();

    /*
        @brief Sets the user pointer for the library.
        @param ptr The pointer to set.
     */
    void set_user_ptr(void* ptr);

    /*
        @brief Gets the user pointer for the library.
        This is useful for storing user data that needs to be accessed in callbacks.
        @return The user pointer. Default is `nullptr`.
     */
    void* get_user_ptr();

    /*
        @brief An enum class to represent a network protocol.
     */
    enum class Protocol
    {
        Null,   // No protocol. Should only be used as a default value.
        Tcp,    // Transmission Control Protocol.
        Udp     // User Datagram Protocol.
    };

    /*
        @brief A struct to represent an address.
     */
    struct Address
    {
        std::string host = "";   // The IP address or hostname / domain name.
        ushort port = 0;        // The port number.

        bool operator==(const Address& other) const;
    };

    /*
        @brief Converts a hostname to an IP address.
        @param hostname The hostname / domain name to convert.
        @param success A pointer to a boolean to store whether the conversion was successful.
        @return The IP address as a string. If the conversion was unsuccessful, an empty string is returned.
     */
    std::string hostname_to_ip(const std::string& hostname, bool* success = nullptr);
};

namespace std
{
    template <>
    struct hash<gnet::Address>
    {
        size_t operator()(const gnet::Address& addr) const
        {
            return hash<string>()(addr.host) ^ (hash<int>()(addr.port) << 1);
        }
    };
};

/*
    @brief The Garnet library namespace.
    Garnet is a small, cross-platform C++ networking library providing both high-level server/client architecture and low-level socket operations.
 */
namespace gnet
{
    /*
        @brief A class to represent a socket.
        This class provides a simple cross-platform interface for creating and managing sockets.
        It can be used for both TCP and UDP sockets.
     */
    class Socket
    {
    public:
        /*
            @brief Creates a socket with no protocol.
            Should not be actually used to create or manage a socket.
         */
        Socket();

        /*
            @brief Creates a socket with the specified protocol.
            @param protocol The protocol to use for the socket.
            @param success A pointer to a boolean to store whether the socket was successfully created.
         */
        Socket(Protocol protocol, bool* success = nullptr);

        /*
            @brief Binds the socket to the specified address.
            This is usually used to set up a server socket.
            @param address The address to bind the socket to, usually a server address.
            @param success A pointer to a boolean to store whether the binding was successful.
         */
        void bind(Address address, bool* success = nullptr);

        /*
            @brief Listens for incoming connections on the socket.
            @param backlog The maximum number of pending connections.
            @param success A pointer to a boolean to store whether the listening was successful.
         */
        void listen(int backlog, bool* success = nullptr);

        /*
            @brief Accepts an incoming connection on the socket.
         !  This is a blocking function - it will wait until there is a pending connection.
            @param success A pointer to a boolean to store whether the connection was successfully accepted.
            @return The accepted socket.
         */
        Socket accept(bool* success = nullptr);

        /*
            @brief Connects the socket to the specified server address.
            This is usually used to set up a client socket.
            @param server_address The address of the server to connect to.
            @param success A pointer to a boolean to store whether the connection was successful.
         */
        void connect(Address server_address, bool* success = nullptr);

        /*
            @brief Sends data through the socket.
         !  This function is only meant for TCP sockets. For UDP sockets, use `send_to()`.
            @param data The data to send.
            @param size The size of the data in bytes.
            @param success A pointer to a boolean to store whether the data was successfully sent.
            @return The number of bytes sent. If an error occurred, -1 is returned.
         */
        int send(void* data, int size, bool* success = nullptr);

        /*
            @brief Receives data through the socket.
         !  This is a blocking function - it will wait until there is data to receive.
         !  This function is only meant for TCP sockets. For UDP sockets, use `receive_from()`.
            @param buffer The buffer to store the received data.
            @param buffer_size The size of the buffer in bytes.
            @param success A pointer to a boolean to store whether the data was successfully received.
            @return The number of bytes received (regardless of `buffer_size`). If an error occurred, -1 is returned.
         */
        int receive(void* buffer, int buffer_size, bool* success = nullptr);

        /*
            @brief Sends data through the socket to the specified address.
         !  This function is only meant for UDP sockets. For TCP sockets, use `send()`.
            @param data The data to send.
            @param size The size of the data in bytes.
            @param to The address to send the data to.
            @param success A pointer to a boolean to store whether the data was successfully sent.
            @return The number of bytes sent. If an error occurred, -1 is returned.
         */
        int send_to(void* data, int size, Address to, bool* success = nullptr);

        /*
            @brief Receives data through the socket from the specified address.
         *  This is a blocking function.
         !  This function is only meant for UDP sockets. For TCP sockets, use `receive()`.
            @param buffer The buffer to store the received data.
            @param buffer_size The size of the buffer in bytes.
            @param from The address to receive the data from.
            @param success A pointer to a boolean to store whether the data was successfully received. This could be false simply because there was no data to receive.
            @return The number of bytes received (regardless of `buffer_size`). If an error occurred (which could just be because there was no data to receive), -1 is returned.
         */
        int receive_from(void* buffer, int buffer_size, Address* from, bool* success = nullptr);

        /*
            @brief Closes the socket.
         !  This function should always be called when the socket is no longer needed.
         */
        void close();

        /*
            @brief Gets the address of the socket.
         !  If the socket was not bound to an address, the address will be empty/invalid.
            @return The address of the socket.
         */
        const Address& get_address() const;

        /*
            @brief Gets the protocol of the socket.
            @return The protocol of the socket.
         */
        const Protocol& get_protocol() const;

        /*
            @brief Checks whether the socket is open.
            The socket is considered 'open' if it was created with the constructor that takes a protocol and it has not been closed.
            @return True if the socket is open, false otherwise.
         */
        bool is_open() const;

    private:
        Address m_addr;
        Protocol m_proto;

    #ifdef GNET_OS_WINDOWS
        SOCKET m_backend_socket;
        SOCKADDR_IN m_backend_addr;
        int m_backend_addr_size;
    #elif defined(GNET_OS_UNIX)
        int m_backend_socket;
        sockaddr_in m_backend_addr;
        socklen_t m_backend_addr_size;
    #endif

        bool m_open;
    };

    /*
        @brief A class to represent a TCP server.
        This class provides a simple but comprehensive interface for creating and managing TCP servers.
        The server is multithreaded to allow for concurrent accepting of clients and receiving of data.
     */
    class ServerTcp
    {
    public:
        /*
            @brief Creates a TCP server with the specified server address.
            @param server_address The address of the server.
            @param success A pointer to a boolean to store whether the server was successfully created.
         */
        explicit ServerTcp(Address server_address, bool* success = nullptr);

        ServerTcp(const ServerTcp&) = delete;
        ServerTcp& operator=(const ServerTcp&) = delete;

        /*
            @brief Opens the server for incoming connections.
            This function starts listening for incoming connects and starts the thread that coninuously accepts them.
            @param backlog The maximum number of pending connections. Default is 10.
            @param success A pointer to a boolean to store whether the server was successfully opened.
         */
        void open(int backlog = 10, bool* success = nullptr);

        /*
            @brief Sends data to the specified client.
            Each call is delivered as exactly one message to the client's receive callback (a 4-byte length prefix is added internally).
         !  Because of this framing, the client must also use `ClientTcp` rather than a raw `Socket`.
            @param data The data to send.
            @param size The size of the data in bytes.
            @param client_address The address of the client to send the data to.
            @param success A pointer to a boolean to store whether the data was successfully sent.
         */
        void send(void* data, int size, Address client_address, bool* success = nullptr);

        /*
            @brief Closes the server and and clears client data (does not affect the actual clients).
            This function is called when a `ServerTcp` is destroyed.
            @param success A pointer to a boolean to store whether the server was successfully closed.
         */
        void close(bool* success = nullptr);

        /*
            @brief Checks whether the server is open.
            The server is considered 'open' if `open()` was called and `close()` was not.
            @return True if the server is open, false otherwise.
         */
        bool is_open() const;

        /*
            @brief Gets the size of the receiving buffer.
            @return The size of the receiving buffer in bytes.
         */
        int get_buffer_size() const;

        /*
            @brief Gets the number of connected clients.
            @return The number of connected clients.
         */
        int get_num_clients() const;

        /*
            @brief Gets the socket representing the accepted connection with the client at the specified address.
            Returns a blank Socket if the address was not found.
            @param client_address The address of the client.
            @param success A pointer to store whether the client was found.
            @return The socket representing the accepted connection with the client.
         */
        Socket get_client_accepted_socket(Address client_address, bool* success = nullptr);
        const std::list<Address> get_client_addresses();
        const std::unordered_map<Address, Socket> get_client_map();

        /*
            @brief Sets the size of the receiving buffer.
            The default is 256 bytes.
            @param size The size of the receiving buffer in bytes.
         */
        void set_buffer_size(int size);

        /*
            @brief Sets the receive callback function.
         !  THE USER IS RESPONSIBLE FOR DELETING THE BUFFER IF A CALLBACK IS USED.
            This function will be called whenever data is received from a client.
            @param callback The receive callback function. The callback function should adhere to the following signature:
            `void callback(void* buffer, int size, int actual_size, Address from_client_address);`
            - `buffer`: A buffer created (on the heap) by the library holding the data received. This should be freed with `delete[] (char*)buffer` when done.
            - `buffer_size`: The size of the given data in bytes.
            - `actual_size`: The original size of the data that was sent from the client (regardless of `buffer_size`), in bytes.
            - `from_client_address`: The address of the client that sent the data.
         */
        void set_receive_callback(void (*callback)(void* buffer, int buffer_size, int actual_size, Address from_client_address));

        /*
            @brief Sets the client connect callback function.
            This function will be called whenever a client connects to the server.
            @param callback The client connect callback function. The callback function should adhere to the following signature:
            `void callback(Address client_address);`
            - `client_address`: The address of the client that connected.
         */
        void set_client_connect_callback(void (*callback)(Address client_address));

        /*
            @brief Sets the client disconnect callback function.
            This function will be called whenever a client disconnects from the server.
            @param callback The client disconnect callback function. The callback function should adhere to the following signature:
            `void callback(Address client_address);`
            - `client_address`: The address of the client that disconnected.
         */
        void set_client_disconnect_callback(void (*callback)(Address client_address));

        ~ServerTcp();

    private:
        Address m_addr;
        Socket m_socket;

        std::atomic<int> m_buf_size;
        std::atomic<int> m_num_clients;

        std::list<Address> m_client_addrs;
        std::unordered_map<Address, Socket> m_client_map;
        std::mutex m_client_addrs_mtx;
        std::mutex m_client_map_mtx;

        std::atomic<bool> m_open;

        void accept();
        void receive(Socket accepted_socket);
        std::thread m_accepting;
        std::vector<std::thread> m_receivings;
        std::mutex m_receivings_mtx;

        std::atomic<void(*)(void* buffer, int buffer_size, int actual_size, Address from_addr)> m_receive_callback;
        std::atomic<void(*)(Address client_addr)> m_client_connect_callback;
        std::atomic<void(*)(Address client_addr)> m_client_disconnect_callback;
    };

    /*
        @brief A class to represent a UDP server.
        This class provides a simple but comprehensive interface for creating and managing UDP servers.
        The server is multithreaded to allow for concurrent receiving of data.
     */
    class ServerUdp
    {
    public:
        /*
            @brief Creates a UDP server with the specified server address.
            @param server_address The address of the server.
            @param success A pointer to a boolean to store whether the server was successfully created.
         */
        explicit ServerUdp(Address server_address, bool* success = nullptr);

        ServerUdp(const ServerUdp&) = delete;
        ServerUdp& operator=(const ServerUdp&) = delete;

        /*
            @brief Opens the server for incoming connections.
            This function starts the thread that continuously receives data.
            @param success A pointer to a boolean to store whether the server was successfully opened.
         */
        void open(bool* success = nullptr);

        /*
            @brief Sends data to the specified client.
            @param data The data to send.
            @param size The size of the data in bytes.
            @param client_address The address of the client to send the data to.
            @param success A pointer to a boolean to store whether the data was successfully sent.
         */
        void send(void* data, int size, Address client_address, bool* success = nullptr);

        /*
            @brief Closes the server.
            This function is called when a `ServerUdp` is destroyed.
            @param success A pointer to a boolean to store whether the server was successfully closed.
         */
        void close(bool* success = nullptr);

        /*
            @brief Checks whether the server is open.
            The server is considered 'open' if `open()` was called and `close()` was not.
            @return True if the server is open, false otherwise.
         */
        bool is_open() const;

        /*
            @brief Gets the size of the receiving buffer.
            @return The size of the receiving buffer in bytes.
         */
        int get_buffer_size() const;

        /*
            @brief Sets the size of the receiving buffer.
            The default is 256 bytes.
            @param size The size of the receiving buffer in bytes.
         */
        void set_buffer_size(int size);

        /*
            @brief Sets the receive callback function.
         !  THE USER IS RESPONSIBLE FOR DELETING THE BUFFER IF A CALLBACK IS USED.
            This function will be called whenever data is received from a client.
            @param callback The receive callback function. The callback function should adhere to the following signature:
            `void callback(void* buffer, int size, int actual_size, Address from_client_address);`
            - `buffer`: A buffer created (on the heap) by the library holding the data received. This should be freed with `delete[] (char*)buffer` when done.
            - `buffer_size`: The size of the given data in bytes.
            - `actual_size`: The original size of the data that was sent from the client (regardless of `buffer_size`), in bytes.
            - `from_client_address`: The address of the client that sent the data.
         */
        void set_receive_callback(void (*callback)(void* buffer, int buffer_size, int actual_size, Address from_client_address));

        ~ServerUdp();

    private:
        Address m_addr;
        Socket m_socket;

        std::atomic<int> m_buf_size;
        std::atomic<bool> m_open;

        void receive();
        std::thread m_receiving;

        std::atomic<void(*)(void* buffer, int buffer_size, int actual_size, Address from_addr)> m_receive_callback;
    };

    /*
        @brief A class to represent a TCP client.
        This class provides a simple but comprehensive interface for creating and managing TCP clients.
        The client is multithreaded to allow for concurrent receiving of data.
     */
    class ClientTcp
    {
    public:
        /*
            @brief Creates a TCP client. Call `connect()` to connect it to a server.
            @param success A pointer to a boolean to store whether the client was successfully created.
         */
        explicit ClientTcp(bool* success = nullptr);

        ClientTcp(const ClientTcp&) = delete;
        ClientTcp& operator=(const ClientTcp&) = delete;

        /*
            @brief Connects the client to the specified server address.
            @param server_address The address of the server to connect to.
            @param success A pointer to a boolean to store whether the connection was successful.
         */
        void connect(Address server_address, bool* success = nullptr);

        /*
            @brief Sends data to the server.
            Each call is delivered as exactly one message to the server's receive callback (a 4-byte length prefix is added internally).
         !  Because of this framing, the server must also use `ServerTcp` rather than a raw `Socket`.
            @param data The data to send.
            @param size The size of the data in bytes.
            @param success A pointer to a boolean to store whether the data was successfully sent.
         */
        void send(void* data, int size, bool* success = nullptr);

        /*
            @brief Disconnects the client.
            This function is called when a `ClientTcp` is destroyed.
            @param success A pointer to a boolean to store whether the client was successfully disconnected.
         */
        void disconnect(bool* success = nullptr);

        /*
            @brief Checks whether the client is connected.
            The client is considered 'connected' if `connect()` was called and neither `disconnect()` was called nor the server closed the connection.
            `disconnect()` should still be called after the server closes the connection.
            @return True if the client is connected, false otherwise.
         */
        bool is_connected() const;

        /*
            @brief Gets the size of the receiving buffer.
            @return The size of the receiving buffer in bytes.
         */
        int get_buffer_size() const;

        /*
            @brief Sets the size of the receiving buffer.
            The default is 256 bytes.
            @param size The size of the receiving buffer in bytes.
         */
        void set_buffer_size(int size);

        /*
            @brief Sets the receive callback function.
         !  THE USER IS RESPONSIBLE FOR DELETING THE BUFFER IF A CALLBACK IS USED.
            This function will be called whenever data is received from the server.
            @param callback The receive callback function. The callback function should adhere to the following signature:
            `void callback(void* buffer, int size, int actual_size);`
            - `buffer`: A buffer created (on the heap) by the library holding the data received. This should be freed with `delete[] (char*)buffer` when done.
            - `buffer_size`: The size of the given data in bytes.
            - `actual_size`: The original size of the data that was sent from the server (regardless of `buffer_size`), in bytes.
         */
        void set_receive_callback(void (*callback)(void* buffer, int buffer_size, int actual_size));

        /*
            @brief Sets the disconnect callback function.
            This function will be called (from the receiving thread) when the server closes the connection or the connection is lost.
            It is not called when the client disconnects itself with `disconnect()`.
         !  `disconnect()` should still be called afterwards to clean up the socket and thread.
            @param callback The disconnect callback function. The callback function should adhere to the following signature:
            `void callback();`
         */
        void set_disconnect_callback(void (*callback)());

        ~ClientTcp();

    private:
        Socket m_socket;

        std::atomic<int> m_buf_size;
        std::atomic<bool> m_connected;

        void receive(); // receive() and callback while true until error (from server or client closure)
        std::thread m_receiving;

        std::atomic<void(*)(void* buffer, int buffer_size, int actual_size)> m_receive_callback;
        std::atomic<void(*)()> m_disconnect_callback;
    };

    /*
        @brief A class to represent a UDP client.
        This class provides a simple but comprehensive interface for creating and managing UDP clients.
        The client is multithreaded to allow for concurrent receiving of data.
     */
    class ClientUdp
    {
    public:
        /*
            @brief Creates a UDP client and starts its receiving thread.
            @param success A pointer to a boolean to store whether the client was successfully created.
         */
        explicit ClientUdp(bool* success = nullptr);

        ClientUdp(const ClientUdp&) = delete;
        ClientUdp& operator=(const ClientUdp&) = delete;

        /*
            @brief Sends data to the server.
            @param data The data to send.
            @param size The size of the data in bytes.
            @param server_address The address of the server to send the data to.
            @param success A pointer to a boolean to store whether the data was successfully sent.
         */
        void send(void* data, int size, Address server_address, bool* success = nullptr);

        /*
            @brief Disconnects the client.
            This function is called when a `ClientUdp` is destroyed.
         *  While the client isn't really "connected" (since it uses UDP), this function stops the receiving thread and closes the socket.
         */
        void disconnect(bool* success = nullptr);

        /*
            @brief Checks whether the client is connected.
            The client is considered 'connected' from construction until `disconnect()` is called.
            @return True if the client is connected, false otherwise.
         */
        bool is_connected() const;

        /*
            @brief Gets the size of the receiving buffer.
            @return The size of the receiving buffer in bytes.
         */
        int get_buffer_size() const;

        /*
            @brief Sets the size of the receiving buffer.
            The default is 256 bytes.
            @param size The size of the receiving buffer in bytes.
         */
        void set_buffer_size(int size);

        /*
            @brief Sets the receive callback function.
         !  THE USER IS RESPONSIBLE FOR DELETING THE BUFFER IF A CALLBACK IS USED.
            This function will be called whenever data is received from the server.
            @param callback The receive callback function. The callback function should adhere to the following signature:
            `void callback(void* buffer, int size, int actual_size, Address from_server_address);`
            - `buffer`: A buffer created (on the heap) by the library holding the data received. This should be freed with `delete[] (char*)buffer` when done.
            - `buffer_size`: The size of the given data in bytes.
            - `actual_size`: The original size of the data that was sent from the server (regardless of `buffer_size`), in bytes.
            - `from_server_address`: The address of the server that sent the data.
         */
        void set_receive_callback(void (*callback)(void* buffer, int buffer_size, int actual_size, Address from_server_address));

        ~ClientUdp();

    private:
        Socket m_socket;

        std::atomic<int> m_buf_size;
        std::atomic<bool> m_connected;

        void receive();
        std::thread m_receiving;

        std::atomic<void(*)(void* buffer, int buffer_size, int actual_size, Address from_addr)> m_receive_callback;
    };
};

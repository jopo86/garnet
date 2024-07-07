#include <functional>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <list>

#define GNET_VERSION_MAJOR  0
#define GNET_VERSION_MINOR  0
#define GNET_VERSION_PATCH  2

#define GNET_DEV            true
#define GNET_ALPHA          false
#define GNET_BETA           false
#define GNET_STABLE         false

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
namespace Garnet
{
    /*
        @brief Gets the major version of the library.
        @return The major version of the library (X.y.z).
     */
    int GetVersionMajor();

    /*
        @brief Gets the minor version of the library.
        @return The minor version of the library (x.Y.z).
     */
    int GetVersionMinor();

    /*
        @brief Gets the patch version of the library.
        @return The patch version of the library (x.y.Z).
     */
    int GetVersionPatch();

    /*
        @brief Gets the version of the library as a string.
        @return The version of the library as a string in the format 'x.y.z' or 'x.y.z-alpha/beta'.
     */
    std::string GetVersionString();

    /*
        @brief Initializes the library.
        This function is required on Windows, but not on Unix systems.  
        @param printErrors If true, errors will always be printed to the console. Helpful for quick debugging.
        @return True if the library was successfully initialized, false otherwise. Always returns true on Unix systems.
     */
    bool Init(bool printErrors = false);

    /*
        @brief Terminates the library.
        This function is required on Windows, but not on Unix systems.
        It literally does nothing on Unix systems.
     */
    void Terminate();

    /*
        @brief Gets the last error message.
        @return The last error message as a string.
     */
    const std::string& GetLastError();

    /*
        @brief Sets the user pointer for the library.
        @param ptr The pointer to set.
     */
    void SetUserPtr(void* ptr);

    /*
        @brief Gets the user pointer for the library.
        This is useful for storing user data that needs to be accessed in callbacks.
        @return The user pointer. Default is `nullptr`.
     */
    void* GetUserPtr();

    /*
        @brief An enum class to represent a network protocol.
     */
    enum class Protocol
    {
        Null,   // No protocol. Should only be used as a default value.
        TCP,    // Transmission Control Protocol.
        UDP     // User Datagram Protocol.
    };

    /*
        @brief A struct to represent an address.
     */
    struct Address
    {
        std::string host;   // The IP address or hostname / domain name.
        ushort port;        // The port number.

        void operator=(const Address& other);
        bool operator==(const Address& other) const;
    };

    /*
        @brief Converts a hostname to an IP address.
        @param hostname The hostname / domain name to convert.
        @param success A pointer to a boolean to store whether the conversion was successful.
        @return The IP address as a string. If the conversion was unsuccessful, an empty string is returned.
     */
    std::string HostnameToIP(const std::string& hostname, bool* success = nullptr);
};

namespace std
{
    template <>
    struct hash<Garnet::Address>
    {
        size_t operator()(const Garnet::Address& addr) const 
        {
            return hash<string>()(addr.host) ^ (hash<int>()(addr.port) << 1);
        }
    };
};

/*
    @brief The Garnet library namespace.
    Garnet is a small, cross-platform C++ networking library providing both high-level server/client architecture and low-level socket operations.  
 */
namespace Garnet
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
            @param serverAddress The address of the server to connect to.
            @param success A pointer to a boolean to store whether the connection was successful.
         */
        void connect(Address serverAddress, bool* success = nullptr);

        /*
            @brief Sends data through the socket.
         !  This function is only meant for TCP sockets. For UDP sockets, use `sendTo()`.
            @param data The data to send.
            @param size The size of the data in bytes.
            @param success A pointer to a boolean to store whether the data was successfully sent.
            @return The number of bytes sent. If an error occurred, -1 is returned.
         */
        int send(void* data, int size, bool* success = nullptr);

        /*
            @brief Receives data through the socket.
         !  This is a blocking function - it will wait until there is data to receive.
         !  This function is only meant for TCP sockets. For UDP sockets, use `receiveFrom()`.
            @param buffer The buffer to store the received data.
            @param bufferSize The size of the buffer in bytes.
            @param success A pointer to a boolean to store whether the data was successfully received.
            @return The number of bytes received (regardless of `bufferSize`). If an error occurred, -1 is returned.
         */
        int receive(void* buffer, int bufferSize, bool* success = nullptr);

        /*
            @brief Sends data through the socket to the specified address.
         !  This function is only meant for UDP sockets. For TCP sockets, use `send()`.
            @param data The data to send.
            @param size The size of the data in bytes.
            @param to The address to send the data to.
            @param success A pointer to a boolean to store whether the data was successfully sent.
            @return The number of bytes sent. If an error occurred, -1 is returned.
         */
        int sendTo(void* data, int size, Address to, bool* success = nullptr);

        /*
            @brief Receives data through the socket from the specified address.
         *  This is NOT a blocking function - it will return immediately if there is no data to receive.
         !  This function is only meant for UDP sockets. For TCP sockets, use `receive()`.
            @param buffer The buffer to store the received data.
            @param bufferSize The size of the buffer in bytes.
            @param from The address to receive the data from.
            @param success A pointer to a boolean to store whether the data was successfully received. This could be false simply because there was no data to receive.
            @return The number of bytes received (regardless of `bufferSize`). If an error occurred (which could just be because there was no data to receive), -1 is returned.
         */
        int receiveFrom(void* buffer, int bufferSize, Address* from, bool* success = nullptr);

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
        const Address& getAddress() const;

        /*
            @brief Gets the protocol of the socket.
            @return The protocol of the socket.
         */
        const Protocol& getProtocol() const;

        /*
            @brief Checks whether the socket is open.
            The socket is considered 'open' if it was created with the constructor that takes a protocol and it has not been closed.
            @return True if the socket is open, false otherwise.
         */
        bool isOpen() const;

    private:
        Address m_addr;
        Protocol m_proto;

    #ifdef GNET_OS_WINDOWS
        SOCKET m_bSocket;
        SOCKADDR_IN m_bAddr;
        int m_bAddrSize;
    #elif defined(GNET_OS_UNIX)
        int m_bSocket;
        sockaddr_in m_bAddr;
        socklen_t m_bAddrSize;
    #endif

        bool m_open;
    };

    /*
        @brief A class to represent a TCP server.
        This class provides a simple but comprehensive interface for creating and managing TCP servers.
        The server is multithreaded to allow for concurrent accepting of clients and receiving of data.
     */
    class ServerTCP
    {
    public:
        /*
            @brief Creates an empty TCP server.
            Should not be actually used to create or manage a server.
         */
        ServerTCP();

        /*
            @brief Creates a TCP server with the specified server address.
            @param serverAddress The address of the server.
            @param success A pointer to a boolean to store whether the server was successfully created.
         */
        ServerTCP(Address serverAddress, bool* success = nullptr);

        /*
            @brief Opens the server for incoming connections.
            This function starts listening for incoming connects and starts the thread that coninuously accepts them.
            @param backlog The maximum number of pending connections. Default is 10.
            @param success A pointer to a boolean to store whether the server was successfully opened.
         */
        void open(int backlog = 10, bool* success = nullptr);

        /*
            @brief Sends data to the specified client.
         !  This function will throw an error if the client address is not in the list of connected clients.
            @param data The data to send.
            @param size The size of the data in bytes.
            @param clientAddress The address of the client to send the data to.
            @param success A pointer to a boolean to store whether the data was successfully sent.
         */
        void send(void* data, int size, Address clientAddress, bool* success = nullptr);

        /*
            @brief Closes the server and and clears client data (does not affect the actual clients).
         !  This function should always be called when the server is no longer needed.
            @param success A pointer to a boolean to store whether the server was successfully closed.
         */
        void close(bool* success = nullptr);
        
        /*
            @brief Checks whether the server is open.
            The server is considered 'open' if `open()` was called and `close()` was not.
            @return True if the server is open, false otherwise.
         */
        bool isOpen() const;

        /*
            @brief Gets the size of the receiving buffer.
            @return The size of the receiving buffer in bytes.
         */
        int getBufferSize() const;

        /*
            @brief Gets the number of connected clients.
            @return The number of connected clients.
         */
        int getNumClients() const;

        /*
            @brief Gets the socket representing the accepted connection with the client at the specified address.
         !  This function will throw an error if the client address is not in the list of connected clients.
            @param clientAddress The address of the client.
            @return The socket representing the accepted connection with the client.
         */
        Socket& getClientAcceptedSocket(Address clientAddress);
        const std::list<Address>& getClientAddresses() const;
        const std::unordered_map<Address, Socket>& getClientMap() const;

        /*
            @brief Sets the size of the receiving buffer.
            The default is 256 bytes.
            @param size The size of the receiving buffer in bytes.
         */
        void setBufferSize(int size);

        /*
            @brief Sets the receive callback function.
         !  THE USER IS RESPONSIBLE FOR DELETING THE BUFFER IF A CALLBACK IS USED.
            This function will be called whenever data is received from a client.
            @param callback The receive callback function. The callback function should adhere to the following signature:
            `void callback(void* buffer, int size, int actualSize, Address fromClientAddress);`
            - `buffer`: A buffer created (on the heap) by the library holding the data received. This should be deleted when done.
            - `bufferSize`: The size of the given data in bytes.
            - `actualSize`: The original size of the data that was sent from the client (regardless of `bufferSize`), in bytes.
            - `fromClientAddress`: The address of the client that sent the data.
         */
        void setReceiveCallback(void (*callback)(void* buffer, int bufferSize, int actualSize, Address fromClientAddress));

        /*
            @brief Sets the client connect callback function.
            This function will be called whenever a client connects to the server.
            @param callback The client connect callback function. The callback function should adhere to the following signature:
            `void callback(Address clientAddress);`
            - `clientAddress`: The address of the client that connected.
         */
        void setClientConnectCallback(void (*callback)(Address clientAddress));

        /*
            @brief Sets the client disconnect callback function.
            This function will be called whenever a client disconnects from the server.
            @param callback The client disconnect callback function. The callback function should adhere to the following signature:
            `void callback(Address clientAddress);`
            - `clientAddress`: The address of the client that disconnected.
         */
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

        void accept();
        void receive(Socket acceptedSocket);
        std::thread m_accepting;
        std::vector<std::thread> m_receivings;

        void (*m_pReceiveCallback)(void* buffer, int bufferSize, int actualSize, Address fromAddr);
        void (*m_pClientConnectCallback)(Address clientAddr);
        void (*m_pClientDisconnectCallback)(Address clientAddr);
    };

    /*
        @brief A class to represent a UDP server.
        This class provides a simple but comprehensive interface for creating and managing UDP servers.
        The server is multithreaded to allow for concurrent receiving of data.
     */
    class ServerUDP
    {
    public:
        /*
            @brief Creates an empty UDP server.
            Should not be actually used to create or manage a server.
         */
        ServerUDP();

        /*
            @brief Creates a UDP server with the specified server address.
            @param serverAddress The address of the server.
            @param success A pointer to a boolean to store whether the server was successfully created.
         */
        ServerUDP(Address serverAddress, bool* success = nullptr);

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
            @param clientAddress The address of the client to send the data to.
            @param success A pointer to a boolean to store whether the data was successfully sent.
         */
        void send(void* data, int size, Address clientAddress, bool* success = nullptr);

        /*
            @brief Closes the server.
         !  This function should always be called when the server is no longer needed.
            @param success A pointer to a boolean to store whether the server was successfully closed.
         */
        void close(bool* success = nullptr);

        /*
            @brief Checks whether the server is open.
            The server is considered 'open' if `open()` was called and `close()` was not.
            @return True if the server is open, false otherwise.
         */
        bool isOpen() const;

        /*
            @brief Gets the size of the receiving buffer.
            @return The size of the receiving buffer in bytes.
         */
        int getBufferSize() const;

        /*
            @brief Sets the size of the receiving buffer.
            The default is 256 bytes.
            @param size The size of the receiving buffer in bytes.
         */
        void setBufferSize(int size);

        /*
            @brief Sets the receive callback function.
         !  THE USER IS RESPONSIBLE FOR DELETING THE BUFFER IF A CALLBACK IS USED.
            This function will be called whenever data is received from a client.
            @param callback The receive callback function. The callback function should adhere to the following signature:
            `void callback(void* buffer, int size, int actualSize, Address fromClientAddress);`
            - `buffer`: A buffer created (on the heap) by the library holding the data received. This should be deleted when done.
            - `bufferSize`: The size of the given data in bytes.
            - `actualSize`: The original size of the data that was sent from the client (regardless of `bufferSize`), in bytes.
            - `fromClientAddress`: The address of the client that sent the data.
         */
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

    /*
        @brief A class to represent a TCP client.
        This class provides a simple but comprehensive interface for creating and managing TCP clients.
        The client is multithreaded to allow for concurrent receiving of data.
     */
    class ClientTCP
    {
    public:
        /*
            @brief Creates an empty TCP client.
            Should not be actually used to create or manage a client.
         */
        ClientTCP();
        /*
            @brief Creates a TCP client.
            @param dummyPutAnything A dummy parameter to differentiate this constructor from the default constructor. Put anything!
            @param success A pointer to a boolean to store whether the client was successfully created.
         */
        ClientTCP(char dummyPutAnything, bool* success = nullptr);
        
        /*
            @brief Connects the client to the specified server address.
            @param serverAddress The address of the server to connect to.
            @param success A pointer to a boolean to store whether the connection was successful.
         */
        void connect(Address serverAddress, bool* success = nullptr);

        /*
            @brief Sends data to the server.
            @param data The data to send.
            @param size The size of the data in bytes.
            @param success A pointer to a boolean to store whether the data was successfully sent.
         */
        void send(void* data, int size, bool* success = nullptr);

        /*
            @brief Disconnects the client.
         !  This function should always be called when the client is no longer needed.
            @param success A pointer to a boolean to store whether the client was successfully disconnected.
         */
        void disconnect(bool* success = nullptr);

        /*
            @brief Checks whether the client is connected.
            The client is considered 'connected' if `connect()` was called and `disconnect()` was not.
            @return True if the client is connected, false otherwise.
         */
        bool isConnected() const;

        /*
            @brief Gets the size of the receiving buffer.
            @return The size of the receiving buffer in bytes.
         */
        int getBufferSize() const;

        /*
            @brief Sets the size of the receiving buffer.
            The default is 256 bytes.
            @param size The size of the receiving buffer in bytes.
         */
        void setBufferSize(int size);

        /*
            @brief Sets the receive callback function.
         !  THE USER IS RESPONSIBLE FOR DELETING THE BUFFER IF A CALLBACK IS USED.
            This function will be called whenever data is received from the server.
            @param callback The receive callback function. The callback function should adhere to the following signature:
            `void callback(void* buffer, int size, int actualSize);`
            - `buffer`: A buffer created (on the heap) by the library holding the data received. This should be deleted when done.
            - `bufferSize`: The size of the given data in bytes.
            - `actualSize`: The original size of the data that was sent from the server (regardless of `bufferSize`), in bytes.
         */
        void setReceiveCallback(void (*callback)(void* buffer, int bufferSize, int actualSize));

    private:
        Socket m_socket;

        std::atomic<int> m_bufSize;
        std::atomic<bool> m_connected;

        void receive(); // receive() and callback while true until error (from server or client closure)
        std::thread m_receiving;

        void (*m_pReceiveCallback)(void* buffer, int bufferSize, int actualSize);
    };

    /*
        @brief A class to represent a UDP client.
        This class provides a simple but comprehensive interface for creating and managing UDP clients.
        The client is multithreaded to allow for concurrent receiving of data.
     */
    class ClientUDP
    {
    public:
        /*
            @brief Creates an empty UDP client.
            Should not be actually used to create or manage a client.
         */
        ClientUDP();

        /*
            @brief Creates a UDP client.
            @param dummyPutAnything A dummy parameter to differentiate this constructor from the default constructor. Put anything!
            @param success A pointer to a boolean to store whether the client was successfully created.
         */
        ClientUDP(char dummyPutAnything, bool* success = nullptr);

        /*
            @brief Sends data to the server.
            @param data The data to send.
            @param size The size of the data in bytes.
            @param serverAddress The address of the server to send the data to.
            @param success A pointer to a boolean to store whether the data was successfully sent.
         */
        void send(void* data, int size, Address serverAddress, bool* success = nullptr);

        /*
            @brief Disconnects the client.
         *  While the client isn't really connected (since it uses UDP), this function stops the receiving thread and closes the socket.
         */
        void disconnect(bool* success = nullptr);

        /*
            @brief Checks whether the client is connected.
            The client is considered 'connected' if the constructor with the dummy parameter was used and `disconnect()` was not called.
            @return True if the client is connected, false otherwise.
         */
        bool isConnected() const;

        /*
            @brief Gets the size of the receiving buffer.
            @return The size of the receiving buffer in bytes.
         */
        int getBufferSize() const;

        /*
            @brief Sets the size of the receiving buffer.
            The default is 256 bytes.
            @param size The size of the receiving buffer in bytes.
         */
        void setBufferSize(int size);

        /*
            @brief Sets the receive callback function.
         !  THE USER IS RESPONSIBLE FOR DELETING THE BUFFER IF A CALLBACK IS USED.
            This function will be called whenever data is received from the server.
            @param callback The receive callback function. The callback function should adhere to the following signature:
            `void callback(void* buffer, int size, int actualSize, Address fromServerAddress);`
            - `buffer`: A buffer created (on the heap) by the library holding the data received. This should be deleted when done.
            - `bufferSize`: The size of the given data in bytes.
            - `actualSize`: The original size of the data that was sent from the server (regardless of `bufferSize`), in bytes.
            - `fromServerAddress`: The address of the server that sent the data.
         */
        void setReceiveCallback(void (*callback)(void* buffer, int bufferSize, int actualSize, Address fromServerAddress));

    private:
        Socket m_socket;

        std::atomic<int> m_bufSize;
        std::atomic<bool> m_connected;

        void receive();
        std::thread m_receiving;

        void (*m_pReceiveCallback)(void* buffer, int bufferSize, int actualSize, Address fromAddr);
    };
};

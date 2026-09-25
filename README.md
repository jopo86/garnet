<p align="center">
  <img src="logo.png" alt="Garnet logo">
</p>

# Garnet

Garnet is a small, cross-platform C++20 networking library for Windows, Linux, and macOS. It wraps Winsock and POSIX sockets in one API with two layers:

- **Low level:** a `Socket` class for direct TCP/UDP communication. It exposes the same operations as the native APIs, with less boilerplate.
- **High level:** multithreaded, callback-driven `Server`/`Client` classes. They handle accepting connections, receiving data, and message framing in the background, so your main thread stays free.

## Features

- **Same API on every platform.** Winsock (Windows) and BSD sockets (Linux/macOS) share one interface, so application code needs no platform checks.
- **TCP and UDP** at both the socket level and the server/client level.
- **Message framing for TCP.** `ServerTcp` and `ClientTcp` add a length prefix to each message, so one `send()` on one side triggers exactly one receive callback on the other. Messages are never merged or split across callbacks.
- **Background threads.** Servers accept and receive on worker threads. Your code reacts through callbacks for received data and (for TCP) client connect and disconnect events.
- **TCP disconnect detection.** An orderly shutdown and a dropped connection both end the session, and the server fires the disconnect callback.
- **Simple error handling.** Every operation that can fail takes an optional `bool* success` out-parameter. `garnet::get_last_error()` returns a readable message, and `garnet::init(true)` prints errors as they happen while debugging.
- **Hostname resolution** through `garnet::hostname_to_ip()`.

## Requirements

- A C++20 compiler (MSVC 2019 16.10+, GCC 10+, or Clang 10+)
- CMake 3.13 or newer
- On Windows, the library links `ws2_32` automatically.

## Building

```sh
git clone https://github.com/jopo86/garnet.git
cd garnet
cmake -S . -B build
cmake --build build --config Release
```

The library is written to:

| Generator | Output |
| --- | --- |
| Visual Studio (multi-config) | `build/Release/garnet.lib` |
| Makefiles / Ninja (single-config) | `build/libgarnet.a` |

The examples are built by default and placed in `build/examples/`. To build only the library, change `set(BUILD_EXAMPLES ON)` to `OFF` in the root `CMakeLists.txt`.

## Using Garnet in your project

Add `src/` to your include path, include `garnet.h`, and link against the built library. On Windows, also link `ws2_32`.

Every program must call `garnet::init()` before any other Garnet function and `garnet::terminate()` when it's finished. On Windows these start up and shut down Winsock. On Unix they do nothing, but calling them keeps the code portable.

### Quick start: TCP echo server

```cpp
#include <garnet.h>
#include <iostream>
#include <string>

void on_receive(void* buffer, int buffer_size, int actual_size, garnet::Address from)
{
    std::string msg((char*)buffer, actual_size < buffer_size ? actual_size : buffer_size);
    std::cout << from.host << ":" << from.port << " sent: " << msg << "\n";

    auto& server = *(garnet::ServerTcp*)garnet::get_user_ptr();
    server.send(msg.data(), (int)msg.size(), from); // echo it back

    delete[] (char*)buffer; // the callback owns the buffer
}

int main()
{
    garnet::init(true);

    garnet::ServerTcp server(garnet::Address{ .host = "127.0.0.1", .port = 55555 });
    garnet::set_user_ptr(&server);
    server.set_receive_callback(on_receive);
    server.open();

    std::cin.get(); // serve until Enter is pressed

    server.close();
    garnet::terminate();
}
```

### Quick start: TCP client

```cpp
#include <garnet.h>
#include <iostream>
#include <string>

void on_receive(void* buffer, int buffer_size, int actual_size)
{
    std::cout << "Server: " << std::string((char*)buffer, actual_size < buffer_size ? actual_size : buffer_size) << "\n";
    delete[] (char*)buffer;
}

int main()
{
    garnet::init(true);

    garnet::ClientTcp client('c'); // the argument is a placeholder that selects the initializing constructor
    client.set_receive_callback(on_receive);
    client.connect(garnet::Address{ .host = "127.0.0.1", .port = 55555 });

    std::string line;
    while (client.is_connected() && std::getline(std::cin, line))
        client.send(line.data(), (int)line.size());

    client.disconnect();
    garnet::terminate();
}
```

## API overview

| Class | Purpose |
| --- | --- |
| `Socket` | Low-level TCP/UDP socket: `bind`, `listen`, `accept`, `connect`, `send`/`receive`, `send_to`/`receive_from`. |
| `ServerTcp` | Multithreaded TCP server. Accepts clients in the background and tracks connected clients. Has callbacks for receive, connect, and disconnect. |
| `ClientTcp` | TCP client with a background receive thread and a receive callback. |
| `ServerUdp` | UDP server with a background receive thread and a receive callback that includes the sender's address. |
| `ClientUdp` | UDP client with a background receive thread. Messages can be sent to any address. |

Every public function is documented in [`src/garnet.h`](src/garnet.h). See the [Wiki](https://github.com/jopo86/garnet/wiki) for longer guides.

### Receive callbacks

The high-level classes pass received data to your callback in a heap-allocated buffer. **The callback owns that buffer and must free it** with `delete[] (char*)buffer`.

- `buffer_size` is the capacity of the buffer. It's 256 bytes by default and can be changed with `set_buffer_size()`.
- `actual_size` is the length of the message that was sent. It can be larger than `buffer_size`, so always read at most `min(actual_size, buffer_size)` bytes.

Callbacks run on Garnet's worker threads. Guard any state that your callbacks and your main thread both use.

## Examples

The [`examples/src`](examples/src) directory contains four chat programs, each with a server and a client. Start the server first, then one or more clients, each in its own terminal.

| Example | Demonstrates |
| --- | --- |
| `server_tcp` / `client_tcp` | Raw `Socket` over TCP, turn-based one-to-one chat |
| `server_udp` / `client_udp` | Raw `Socket` over UDP, turn-based one-to-one chat |
| `server_tcp_class` / `client_tcp_class` | `ServerTcp` / `ClientTcp`, group chat with connect/disconnect notices |
| `server_udp_class` / `client_udp_class` | `ServerUdp` / `ClientUdp`, group chat |

Type `!quit` to leave a chat.

## Limitations

- **IPv4 only.**
- **Framed TCP on both ends.** `ServerTcp` and `ClientTcp` use a 4-byte big-endian length prefix, so both ends must use Garnet's high-level TCP classes. To talk to a program that doesn't use this framing, use `Socket` directly.
- **UDP is not reliable.** Datagrams may be dropped or arrive out of order, and a datagram larger than the receive buffer is dropped (Windows) or truncated (Linux/macOS).

## License

Garnet is released under the [MIT License](LICENSE).

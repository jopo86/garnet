# ![Garnet Logo](logo.png)
# Garnet
A small, cross-platform C++ networking library providing both high-level server/client architecture and low-level socket operations using TCP or UDP.  

See the [Wiki](https://github.com/jopo86/garnet/wiki) for usage guides & examples.  

## Download
There are pre-compiled binaries for Windows MinGW64 and Linux, but they are not guaranteed to work, so if they don't for you, you can compile yourself using CMake. See [Build](#build) for build instructions.

[Latest Release (1.0.0-beta)](https://github.com/jopo86/garnet/releases/tag/v1.0.0-beta)  
[All Releases](https://github.com/jopo86/garnet/releases)

## Features
- `Socket` class
    - Low-level cross-platform communication between systems
    - More intuitive structure for sockets than with WSA or POSIX but with the same functionalities
    - Native support for TCP or UDP

- `ServerTCP` and `ServerUDP` classes
    - High-level cross-platform basic server functionality
    - Multithreaded to allow for concurrent accepting / receiving & main thread
    - Callback-based structure (client connect/disconnect callback (TCP only), receive callback)

- `ClientTCP` and `ClientUDP` classes
    - High-level cross-platform basic client functionality
    - Multithreaded to allow for concurrent receiving & main thread
    - Callback-based structure (receive callback)

## Build
Garnet uses CMake as its build system. To build, you will need CMake and a C++ compiler such as g++ or clang. I would also recommend MinGW for Windows users.  

**Step 1:**: clone this repository to some directory on your system: 
```
git clone https://github.com/jopo86/garnet.git <dir>
```  
If you don't have Git, you definitely should, but if not you can just download the repository from the GitHub website.  

**Step 2:** open any terminal inside the folder it was cloned to. Create a `build` directory and navigate to that directory in the terminal:
```
mkdir build
cd build
```  

**Step 3:** use CMake to build your project. First, create CMake files from the build directory. I'm on Windows, so I'm going to generate `MinGW Makefiles` (Use `Unix Makefiles` for Linux/macOS): 
```
cmake -G "MinGW Makefiles" ../
```  

**Step 4:** build the project from the build directory: 
```
cmake --build .
```  

Now, in the build directory, you should have a file named `libgarnet.a` (or `garnet.lib`, depending on how you generated your CMake files). This is your compiled library file, and you can now link against this library (and include `Garnet.h`) to use Garnet in other projects!  

To build examples, head into `CMakeLists.txt` (located inside the root directory from where you cloned to), and where it says `set(BUILD_EXAMPLES OFF)`, just switch that `OFF` to `ON` and rerun `cmake --build .` (or repeat steps 2-4 from the `examples` dir instead of the root).  

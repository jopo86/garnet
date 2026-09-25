# TODO before v1.0.0

## Must fix

### 1. Linux build fails at link time
Every executable fails with `undefined reference to pthread_create` (seen with Ubuntu 20.04, g++ 9.4). CMake never links the threads library, which glibc before 2.34 needs for `std::thread`.

- [ ] In the root `CMakeLists.txt`:
  ```cmake
  find_package(Threads REQUIRED)
  target_link_libraries(garnet PUBLIC Threads::Threads)
  ```

### 2. Shutdown can crash (thread lifetime)
- [ ] **No destructors.** `ServerTcp`, `ServerUdp`, `ClientTcp`, and `ClientUdp` have none. If one is destroyed while its `std::thread` is still joinable (no `close()`/`disconnect()`, an early return, or an exception), the program calls `std::terminate`.
- [ ] **Detached threads keep using the object.** `close()`/`disconnect()` call `.detach()`, but the worker threads keep reading members like `m_open`, `m_socket`, and the callbacks. If the object is destroyed right after `close()`, a thread waking up from `recv`/`accept` uses freed memory.
- [ ] Fix: in `close()`/`disconnect()`, close the socket (so blocking calls return), then **join** the threads instead of detaching. Add destructors that do the same if still open or connected.
  - Watch out: joining from inside a callback deadlocks, because the callback runs on the thread being joined. Either document "don't call `close()` from a callback", or check `std::this_thread::get_id()` and detach in that case.

### 3. Data races in `ServerTcp`
- [ ] `get_client_addresses()` / `get_client_map()` return references to containers that the accept and receive threads change. Callers (including `examples/src/server_tcp_class.cpp`) loop over them with no lock. Return a copy taken under the mutex instead.
- [ ] `send()` uses `m_client_map[client_addr]` without the lock. `operator[]` also inserts an empty `Socket` for an unknown address, while the header says it throws. Lock the mutex, use `find()`, and fail (set `*success = false`) if the address is unknown.
- [ ] `m_receivings` is added to from the accept thread and read in `close()` with no lock. Guard it with a mutex.
- [ ] Minor: `m_client_addrs_mtx` and `m_client_map_mtx` are always taken together. Merge them into one mutex, or use `std::scoped_lock` to rule out lock-order bugs.

## Should fix

### 4. 100% CPU when no receive callback is set
- [ ] Every receive loop starts with `if (m_receive_callback == nullptr) continue;`, which spins without ever blocking. Receive anyway and drop the data if there's no callback (also needed so TCP disconnects are still detected), or wait on a condition variable.

### 5. `Address::operator=` returns `void`
- [ ] It produces 14 `-Wdeprecated-copy` warnings on GCC and breaks `a = b = c`. Delete both the declaration and the definition; the compiler-generated one is correct. (`operator==` can stay, or become `bool operator==(const Address&) const = default;` in C++20.)

### 6. Wrong docs for `receive_from`
- [ ] `include/garnet/garnet.hpp` says `Socket::receive_from()` "is NOT a blocking function", but nothing puts the socket in non-blocking mode (`ioctlsocket` / `fcntl(O_NONBLOCK)`), so it blocks. Fix the comment. If non-blocking is wanted, add an explicit `set_blocking(bool)` option.

### 7. Unsafe global `g_print_errors`
- [ ] `ServerTcp::accept()` switches `g_print_errors` on and off from its own thread. The main thread and other workers read it at the same time. Make it `std::atomic<bool>`, or better, don't change the global: have `accept()` skip printing for that one call some other way.
- [ ] Related: `g_err` (the `std::string` behind `get_last_error()`) is written from several threads. Consider making it `thread_local`.

## Release checklist

### Repo.
- [ ] Decide whether `logo.psd` belongs in the repo. It's a large binary that users don't need, so it could live outside the repo or be attached to a release instead.
- [ ] Remove `TODO.md` or move it out of the repo before tagging, if you don't want it public.

### CI
- [ ] Add a GitHub Actions workflow (`.github/workflows/build.yml`) that builds the library and examples on `windows-latest`, `ubuntu-latest`, and `macos-latest`. This would have caught fix #1.
- [ ] Add the build-status badge to the top of `README.md`.
- [ ] Optional: turn on warnings (`/W4`, `-Wall -Wextra`) in CI so new warnings are noticed.

### Verify
- [ ] Run each example pair by hand (server first, then one or more clients). Check joining, chatting, `!quit`, and killing a client or the server mid-session.
- [ ] Build on Linux (and macOS, if you have access) after fix #1, not just on Windows.
- [ ] Recompile the two README quick-start samples against the final API.
- [ ] Re-read `include/garnet/garnet.hpp` doc comments for anything outdated (constructors, framing, blocking behavior).

### Versioning & release
- [ ] Confirm `GNET_VERSION_*` in `garnet.hpp` and `project(Garnet VERSION ...)` in `CMakeLists.txt` both say `1.0.0`.
- [ ] Check the GitHub Wiki against the current API, or remove the Wiki link from the README.
- [ ] Tag the release: `git tag -a v1.0.0 -m "Garnet 1.0.0"` then `git push origin v1.0.0`.
- [ ] Write the GitHub release notes, including the **breaking changes** since `v1.0.0-beta`:
  - Header moved to `include/garnet/garnet.hpp` (use `#include <garnet/garnet.hpp>`)
  - `ClientTcp('c')` / `ClientUdp('c')` → `ClientTcp` / `ClientUdp` (no placeholder argument)
  - Default constructors for `ServerTcp`, `ServerUdp`, `ClientTcp`, `ClientUdp` removed. Server/client objects can no longer be copied.
  - `ServerTcp`/`ClientTcp` now add a 4-byte length prefix to each message, so they only work with each other, not with raw sockets
  - C++20 required
- [ ] Optional: attach prebuilt binaries (for example MSVC `garnet.lib` and Linux `libgarnet.a`) to the release.

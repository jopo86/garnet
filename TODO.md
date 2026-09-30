# TODO before v1.0.0

## Release checklist

### Repo.
- [ ] Remove `TODO.md` or move it out of the repo before tagging, if you don't want it public.

### CI
- [ ] Add a GitHub Actions workflow (`.github/workflows/build.yml`) that builds the library and examples on `windows-latest`, `ubuntu-latest`, and `macos-latest`. This would have caught fix #1.
- [ ] Add the build-status badge to the top of `README.md`.
- [ ] Turn on warnings (`/W4`, `-Wall -Wextra`) in CI so new warnings are noticed.

### Verify
- [ ] Run each example pair by hand (server first, then one or more clients). Check joining, chatting, `!quit`, and killing a client or the server mid-session.
- [ ] Build on Linux (and macOS, if you have access) after fix #1, not just on Windows.
- [ ] Recompile the two README quick-start samples against the final API.
- [ ] Re-read `include/garnet/garnet.hpp` doc comments for anything outdated (constructors, framing, blocking behavior).

### Versioning & release
- [ ] Tag the release: `git tag -a v1.0.0 -m "Garnet 1.0.0"` then `git push origin v1.0.0`.
- [ ] Write the GitHub release notes, including the **breaking changes** since `v1.0.0-beta`:
  - Header moved to `include/garnet/garnet.hpp` (use `#include <garnet/garnet.hpp>`)
  - Namespace renamed from `garnet` to `gnet` (`namespace garnet = gnet;` alias kept for compatibility)
  - `ClientTcp('c')` / `ClientUdp('c')` → `ClientTcp` / `ClientUdp` (no placeholder argument)
  - Default constructors for `ServerTcp`, `ServerUdp`, `ClientTcp`, `ClientUdp` removed. Server/client objects can no longer be copied.
  - `ServerTcp`/`ClientTcp` now add a 4-byte length prefix to each message, so they only work with each other, not with raw sockets
  - C++20 required
- [ ] Optional: attach prebuilt binaries (for example MSVC `garnet.lib` and Linux `libgarnet.a`) to the release.

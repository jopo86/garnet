# TODO before v1.0.0

## Release checklist

### Repo.
- [ ] Decide whether `logo.psd` belongs in the repo. It's a large binary that users don't need, so it could live outside the repo or be attached to a release instead.
- [ ] Remove `TODO.md` or move it out of the repo before tagging, if you don't want it public.
- [ ] Decide whether to keep the `namespace garnet = gnet;` alias at the bottom of `garnet.hpp`. Keeping it means beta code still compiles. Dropping it means there's only one name to document.
- [ ] Update the GitHub Wiki to use `gnet::` (or remove the Wiki link, see below).

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
  - Namespace renamed from `garnet` to `gnet` (`namespace garnet = gnet;` alias kept for compatibility)
  - `ClientTcp('c')` / `ClientUdp('c')` → `ClientTcp` / `ClientUdp` (no placeholder argument)
  - Default constructors for `ServerTcp`, `ServerUdp`, `ClientTcp`, `ClientUdp` removed. Server/client objects can no longer be copied.
  - `ServerTcp`/`ClientTcp` now add a 4-byte length prefix to each message, so they only work with each other, not with raw sockets
  - C++20 required
- [ ] Optional: attach prebuilt binaries (for example MSVC `garnet.lib` and Linux `libgarnet.a`) to the release.

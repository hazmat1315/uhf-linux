# UHF Linux

A native **Linux/macOS IPTV player** — a faithful, cross-platform reimplementation
of the UHF iOS app's feature set and UX, built on **Qt 6 / QML** with **libmpv**
for playback.

> Status: **early Phase 1.** The core domain (playlist parsing, Xtream API,
> library/search, player control) and a real libmpv backend are implemented and
> test-covered. The QML UI, networking, persistence and OpenSubtitles are in
> progress. See [`UHF-Linux-Design-Spec.md`](UHF-Linux-Design-Spec.md) for the
> full design and roadmap.

## What it does (target)

- Load **M3U8** and **Xtream Codes** playlists (Stremio later).
- Browse **Live TV / Movies / Series** by category, search ("filter by name"),
  favorites & favorite lists, continue-watching, per-item customizations.
- Play with **libmpv**: audio/subtitle tracks, aspect ratio, speed, subtitle
  delay, buffer tuning, hardware decode, channel history.
- **OpenSubtitles** subtitle search/download.
- Local-only: no accounts, no cloud sync, no paywall.

## Architecture

```
QML UI  ->  C++ view-models  ->  Core domain (Qt Core only)
                                   - sources/   M3U + Xtream adapters
                                   - library/   search, categories, favorites
                                   - player/    IPlayerBackend + controller
                                   - store/     SQLite (planned)
                              ->  libmpv backend (isolated CMake target)
```

The pure core depends only on **Qt Core** and is headless-testable. `libmpv` is
isolated in its own guarded target, so a machine without it still builds and
tests the core.

## Build

**Dependencies:** Qt 6 (Core, Quick, QuickControls2, Sql, Network, Test), CMake
≥ 3.21, a C++20 compiler, and (for playback) **libmpv**.

```bash
# macOS (Homebrew)
brew install qt mpv cmake          # ffmpeg comes in as an mpv dependency

# Debian/Ubuntu
sudo apt install qt6-base-dev qt6-declarative-dev libmpv-dev cmake g++ pkg-config
```

Configure, build, and test:

```bash
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"  # drop -DCMAKE_PREFIX_PATH on Linux
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The core + its tests build even without libmpv; the `libmpv` backend and its
smoke test are compiled only when libmpv is detected.

## Tests

Test-driven: every core unit has QtTest coverage (`tests/`). The libmpv backend
has a headless smoke test that decodes a short ffmpeg-generated file end-to-end.

## Provenance & legal

This project was informed by reverse-engineering the shipped UHF iOS app with
[REA](https://github.com/morluto/rea) to understand its feature set and data
formats. Behavior is **reimplemented from understanding** — no UHF proprietary
source, bundled assets, icons, embedded credentials, paywall, or backend are
copied or shipped. The reference app bundle is not part of this repository. The
IPTV-playing functionality (M3U/Xtream parsing, playback, organization) is
generic. This is **not** an official nakio studio release, and the name/branding
here are placeholders.

## License

Not yet chosen. Until a license is added, no usage rights are granted.

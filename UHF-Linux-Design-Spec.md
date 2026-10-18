# UHF Linux — Design Spec

**Date:** 2026-10-18
**Status:** Approved (Checkpoint 1); detailed sections captured here for review
**Reference artifact:** `UHF.app` — UHF for iOS v1.87.1 (`com.nakiostudio.uhf.ios`), arm64 Mach-O, min iOS 18. SHA-256 `b76874564f3f0e6b38c5daed6c502a608094e25acbe766d2407ed31f13232aa4`.

---

## 1. Goal & decisions

Build a **native Linux port of UHF** — a faithful reimplementation of its feature set and UX — informed by reverse-engineering the shipped app with REA. Behavior is reimplemented from understanding; UHF's proprietary source, bundled assets (icons/`Assets.car`/storyboards), embedded secrets (`GoogleService-Info.plist`, API keys), and paid-backend/paywall logic are **not** copied or shipped.

| Decision | Choice |
|---|---|
| Scope | Faithful UHF port, delivered in phases |
| UI toolkit | Qt 6 / QML |
| Language | C++ (QML for views) |
| Player engine | libmpv (ffmpeg under the hood) |
| Accounts / cloud sync | **Dropped entirely** — no backend, no accounts, local-only |
| Paywall / subscriptions | **Dropped** — all features free |
| VPN (bundled Atom/OpenVPN SDK) | **Out of scope** — users bring their own VPN |

---

## 2. What UHF is (evidence-grounded)

Observations from the binary's strings/resources (REA + local `strings`):

- **IPTV player** with four tabs + search: **Home, Live TV, Movies, Series, Search** (`TabBar.strings`), plus a **desktop sidebar** layout with pin/expand/favorites (`GeneralSettings` `SECTION_SIDEBAR`).
- **Media types:** Live streams (channels), Movies (VOD), Series (seasons/episodes), EPG (TV Guide).
- **Playlist source types:** M3U8, Xtream Codes, Stremio, Stremio V2 (`PlaylistForm.strings` `ROW_TYPE_*`). Each playlist has a name, icon, optional EPG URL. Xtream playlists expose connections / expiration date / online-offline status.
- **M3U parser** (regexes in binary): `#EXTM3U` header with `x-tvg-url`; `#EXTINF` with duration; attributes `tvg-id`, `tvg-name`, `tvg-logo`, `tvg-chno`, `tvg-country`, `tvg-language`, `tvg-shift`, `group-title`.
- **Xtream Codes** via standard `player_api.php` actions: `get_live_categories`, `get_live_streams`, `get_vod_categories`, `get_series_categories` (and the rest of the standard set); timeshift/catchup (`timeshift_abs-${start-timestamp}.m3u8`).
- **Metadata enrichment:** TMDB (`api.themoviedb.org/3/`, user-supplyable API key) and a GitHub channel-logos repository.
- **Swappable player engines:** AVKit, VLCKit (OpenGL), KSPlayer (Metal), **MPV (experimental)** — UHF already treats the player as an abstraction and ships mpv. Player features: aspect ratio, playback speed, audio/subtitle track selection, subtitle delay, deinterlacing (filters + spatial check), hardware decode, buffer duration, PiP, lock/power-saving screen, media info/stats, skip intro/ads, external player, channel history, next-episode.
- **Content organization:** categories per media type; favorites + **favorite lists** (custom + predefined suggestions); recents ("Continue watching"); per-item customizations — rename, lock/hide, move (top/bottom), merge categories, edit information, reset customizations; search/filter by name.
- **Subtitles:** OpenSubtitles.com integration (username/password; "Matching subtitles" in the player).
- **Settings:** playlists management, player, network (VPN/DNS), theme (light/dark/system), home-section config, advanced customizations, storage/cache cleanup (EPG, TMDB, downloads, temporal), autosync schedule (daily / every 3 days / weekly / monthly / manual), logs/debug, sidebar prefs.

---

## 3. Roadmap (phased; each phase = its own spec → plan → build)

| Phase | Subsystem |
|---|---|
| **1 (this spec)** | Core: libmpv player + M3U/Xtream playlists + browse/search/favorites + local storage + **OpenSubtitles** |
| 2 | EPG (XMLTV + Xtream short-EPG), TV-guide grid, now/next |
| 3 | VOD/Series depth: seasons/episodes, TMDB enrichment, continue-watching, logo repos |
| 4 | Recordings + scheduling (ffmpeg), timeshift/catchup |
| 5 | Casting (Chromecast/GoogleCast; AirPlay optional) |
| 6 | Stremio add-on support |
| 7 | Extras: Trakt, command palette, themes polish, widgets-equivalent |
| — | VPN: dropped. Sync/accounts/paywall: dropped. |

---

## 4. Phase 1 scope

**In:**
- App shell: desktop **sidebar** + Home / Live TV / Movies / Series / Search.
- Playlists: add / edit / delete / reload-sync for **M3U8** and **Xtream Codes**; name + icon + optional EPG URL; Xtream status (connections / expiration / online-offline).
- Browsing: categories; item lists (channels / movies / series); item detail; search & filter by name.
- Organization: favorites + favorite lists; recents / continue-watching; customizations (rename, lock/hide, move, merge categories, edit info, reset).
- Playback: libmpv with audio/subtitle track selection, aspect ratio, playback speed, subtitle delay, buffer duration, hardware decode toggle, media info/stats, channel history, allow-pausing-live.
- **OpenSubtitles:** search by hash/name + preferred language, download, load into mpv as external subtitle track.
- Persistence: local SQLite + config files.
- Theme: light / dark / system.

**Out (later phases):** EPG grid, recordings, casting, Stremio, Trakt, TMDB enrichment, downloads, VPN, QR device-linking, series season/episode watched-tracking depth.

---

## 5. Architecture

```
┌─────────────────────────────────────────────┐
│  QML UI layer  (views, navigation, theming)  │
├─────────────────────────────────────────────┤
│  C++ presentation  (QObject view-models,      │
│     QAbstractItemModel lists exposed to QML)  │
├─────────────────────────────────────────────┤
│  Core domain (C++, no GUI dependency):        │
│   • Source adapters: M3USource, XtreamSource  │
│     behind IPlaylistSource                    │
│   • Library service (categories/items/favs)   │
│   • Player controller → IPlayerBackend → mpv  │
│   • Subtitles service → OpenSubtitles         │
│   • Repository → SQLite                        │
└─────────────────────────────────────────────┘
```

**Principles**
- Core domain depends only on Qt Core (networking/JSON/SQLite), never Qt GUI/QML → headless unit-testable.
- `IPlaylistSource` is the single seam; everything downstream is source-agnostic.
- `IPlayerBackend` abstracts the player (libmpv impl now), mirroring UHF's engine-swapping and leaving room for a VLC backend later.
- Units are small and single-purpose with explicit interfaces.

**Module layout (proposed)**
```
uhf-linux/
  CMakeLists.txt
  src/
    core/
      model/        # Playlist, Category, MediaItem, Series, Favorite…
      sources/      # IPlaylistSource, M3USource, XtreamSource, m3u parser, xtream client
      library/      # LibraryService, search, favorites, recents, customizations
      player/       # IPlayerBackend, MpvBackend, PlayerController
      subtitles/    # OpenSubtitlesClient, SubtitlesService
      store/        # Repository, SQLite schema + migrations
    app/
      viewmodels/   # QObject + QAbstractItemModel bindings for QML
      main.cpp
    qml/            # Views, components, theme
  tests/            # Unit tests (QtTest / Catch2), fixtures
  packaging/        # Linux: AppImage / Flatpak manifests
```

---

## 6. Data model (Phase 1 entities)

- **Playlist** — id, name, icon, type (`m3u` | `xtream`), source config (URL; Xtream: host/port/username/password), optional epgUrl, status (Xtream: connections/expiration/active), lastSyncedAt.
- **Category** — id, playlistId, mediaType (`live` | `movie` | `series`), name, remoteId, sortOrder, locked/hidden, mergedIntoId, custom flags.
- **MediaItem** — id, playlistId, categoryId, mediaType, name, streamUrl (or Xtream stream id → resolved URL), logo/poster, attributes (tvg-id, chno, language, country, …), isFavorite, customizations (renamed name, moved, locked), addedAt.
- **Series** — id, playlistId, categoryId, name, poster; **Season** → **Episode** (episode has streamUrl, number, title) — minimal in Phase 1; depth deferred to Phase 3.
- **FavoriteList** — id, mediaType, name, items[]; plus predefined suggestions.
- **Recent** — mediaItemId, position (resume point), watchedAt.

Local-only; no server IDs beyond what sources provide for reload matching.

---

## 7. Source adapters

**`IPlaylistSource`** → `fetchCategories()`, `fetchItems(mediaType, category)`, `resolveStreamUrl(item)`, `status()`.

**M3USource**
- Fetch M3U(8) over HTTP(S) (configurable User-Agent — UHF exposes this).
- Parse `#EXTM3U` (capture `x-tvg-url` → EPG URL), `#EXTINF` duration + attributes (`tvg-id`, `tvg-name`, `tvg-logo`, `tvg-chno`, `tvg-country`, `tvg-language`, `tvg-shift`, `group-title`), then the stream URL line.
- Group by `group-title` → categories. Media-type inference (live vs VOD) from URL/group heuristics (as UHF does).

**XtreamSource**
- Base: `http(s)://host:port/player_api.php?username=&password=`.
- Actions: `get_live_categories`, `get_live_streams`, `get_vod_categories`, `get_vod_streams`, `get_series_categories`, `get_series`, `get_series_info`.
- Stream URL construction: `…/live/{user}/{pass}/{streamId}.{ext}`, `…/movie/…`, `…/series/…`.
- Account status from `get_user_info` → connections / expiration / active.

Both normalize into the same Category/MediaItem model.

---

## 8. Player (libmpv)

- **`IPlayerBackend`** → load(url, headers), play/pause, seek, speed, aspect, audio/subtitle track list + select, add external subtitle, subtitle delay, property observation (position, duration, cache, tracks, stats).
- **MpvBackend**: embed mpv render context into a Qt `QQuickFramebufferObject` / `QQuickRenderControl` surface; drive via `mpv_command`/`mpv_set_option`/`mpv_observe_property`. Hardware decode toggle (`hwdec`), buffer/cache tuning, user-agent & referrer headers for IPTV streams.
- **PlayerController** (view-model): bridges backend to QML, owns overlay state (controls, lock screen, media-info panel, history).

---

## 9. OpenSubtitles integration

- OpenSubtitles.com REST API; store username/password (not email) in local settings; token login.
- Search by file hash and/or title + preferred language (from player settings).
- Download chosen subtitle → temp file → `MpvBackend.addExternalSubtitle()` → appears in the subtitle menu ("Matching subtitles").
- No credentials bundled; user supplies their own account.

---

## 10. UI / UX

- **Desktop sidebar** as primary nav (Home, Live TV, Movies, Series, Search, Settings), collapsible/pinnable — the iPad/Mac layout UHF already has, appropriate for a Linux window.
- **Home**: configurable sections — Continue watching, Favorites (channels/movies/series), Trending (deferred), TV Guide (deferred).
- **Live TV / Movies / Series**: category grid/list → item grid → detail; favorites & recents sections; filter-by-name search.
- **Player**: full-window video with overlay controls, track/subtitle/speed/aspect menus, media-info panel, lock-screen mode, channel history/search.
- **Settings**: playlists, player, theme, home sections, customizations, storage cleanup, autosync schedule, logs.
- **Empty states / toasts**: match UHF's tone where it costs nothing (its empty-state copy is distinctive); strings are our own, not copied verbatim from its resources.
- **Theme**: light / dark / system, QML theme tokens.

---

## 11. Local storage

- **SQLite** (Qt SQL) for playlists, categories, items, favorites, favorite-lists, recents, customizations; indexed by playlistId + mediaType + name for fast filter.
- **Config** (QSettings / JSON): theme, player defaults, autosync interval, user-agent, OpenSubtitles creds, TMDB key (unused until Phase 3).
- Cache dirs for logos/temporal files with "Clear caches" actions.
- Schema versioned with forward migrations.

---

## 12. Error handling

- Network/parse failures surface as non-fatal, retryable toasts (mirrors UHF's "sync failed — try again?").
- Expired Xtream account / link → clear status messaging.
- Player errors (codec/stream) → overlay with fallback guidance; never crash the shell.
- All source/network/SQLite operations return typed results (ok/err), no exceptions across the UI seam.

---

## 13. Testing strategy

- **TDD** for the pure core: M3U parser (fixtures of real-world `#EXTINF` variants), Xtream response mapping (recorded JSON fixtures), stream-URL construction, library filtering/favorites/customizations logic.
- Backend seams (`IPlaylistSource`, `IPlayerBackend`) faked in tests so the library and view-models test headless.
- mpv backend covered by a thin integration smoke test (load + play a local sample) rather than unit tests.
- Framework: QtTest or Catch2.

---

## 14. Build & target

- **Target:** native Linux (x86-64; arm64 best-effort). Primary packaging: AppImage and/or Flatpak.
- **Dev note:** development happens on macOS; Qt6 + libmpv are cross-platform, so code is portable. Linux build/verification via a Linux toolchain (container/VM/CI). The core domain is OS-agnostic and testable anywhere Qt6 is present.
- **Dependencies:** Qt 6 (Core, Quick, QuickControls2, Sql, Network), libmpv, SQLite (bundled with Qt), a JSON lib (Qt's), CMake + Ninja.

---

## 15. Scope guardrails (recap)

- No UHF proprietary source, assets, icons, `Assets.car`, storyboards, or embedded credentials are copied or shipped.
- No reproduction of UHF's paid backend, paywall, or licensed VPN SDK.
- The IPTV-playing core (M3U/Xtream parsing, playback, organization) is generic functionality, reimplemented from understanding.
- Product branding is a placeholder; this is not an official nakio studio release.

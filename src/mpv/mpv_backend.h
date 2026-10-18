#pragma once

#include <QString>

#include "core/player/player_backend.h"

struct mpv_handle;

namespace uhf::mpv {

// Concrete player backend built on libmpv. Owns an mpv core and translates the
// IPlayerBackend calls into mpv commands/properties.
//
// Rendering into a Qt surface is layered on top separately; this class drives
// playback and can run headless (vo/ao=null) for smoke tests.
class MpvBackend : public core::IPlayerBackend {
public:
    enum class Mode {
        Headless, // no A/V device; decodes only (tests, servers)
        Windowed, // mpv opens its own OS window for video + audio output
    };

    explicit MpvBackend(Mode mode = Mode::Headless);
    ~MpvBackend() override;

    MpvBackend(const MpvBackend&) = delete;
    MpvBackend& operator=(const MpvBackend&) = delete;

    bool initialized() const { return m_initialized; }

    void load(const core::StreamRequest& request) override;
    void play() override;
    void pause() override;
    void stop() override;

    // Reads a numeric mpv property (e.g. "duration"); returns fallback on error.
    double doubleProperty(const QString& name, double fallback = 0.0) const;

    mpv_handle* handle() const { return m_mpv; }

private:
    mpv_handle* m_mpv = nullptr;
    bool m_initialized = false;
    Mode m_mode = Mode::Headless;
};

} // namespace uhf::mpv

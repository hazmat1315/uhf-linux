#pragma once

#include <QString>
#include <QUrl>

namespace uhf::core {

// A request to load a stream, including IPTV-friendly HTTP headers.
struct StreamRequest {
    QUrl url;
    QString userAgent;
    QString referrer;
};

// Abstraction over the concrete video engine (libmpv in production, a fake in
// tests). Mirrors UHF's swappable-engine design and keeps the core testable
// without a GPU/codec stack.
class IPlayerBackend {
public:
    virtual ~IPlayerBackend() = default;

    virtual void load(const StreamRequest& request) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
};

} // namespace uhf::core

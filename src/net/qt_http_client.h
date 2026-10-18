#pragma once

#include "core/sources/http_client.h"

namespace uhf::net {

// IHttpClient backed by QNetworkAccessManager. Synchronous (spins a local
// event loop with a bounded timeout), so callers must invoke it off the UI
// thread.
class QtHttpClient : public core::IHttpClient {
public:
    explicit QtHttpClient(int timeoutMs = 20000) : m_timeoutMs(timeoutMs) {}

    core::HttpResponse get(const QUrl& url, const QString& userAgent = {}) override;

private:
    int m_timeoutMs;
};

} // namespace uhf::net

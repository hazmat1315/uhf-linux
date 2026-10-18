#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

namespace uhf::core {

struct HttpResponse {
    bool ok = false;
    int status = 0;
    QByteArray body;
    QString error;
};

// Minimal synchronous HTTP GET abstraction. The real implementation uses
// QNetworkAccessManager; tests inject a fake. Callers should invoke this off
// the UI thread.
class IHttpClient {
public:
    virtual ~IHttpClient() = default;
    virtual HttpResponse get(const QUrl& url, const QString& userAgent = {}) = 0;
};

} // namespace uhf::core

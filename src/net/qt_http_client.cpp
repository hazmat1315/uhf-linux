#include "net/qt_http_client.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

namespace uhf::net {

core::HttpResponse QtHttpClient::get(const QUrl& url, const QString& userAgent)
{
    // Local manager + event loop so this call is synchronous on the calling
    // (worker) thread.
    QNetworkAccessManager manager;

    QNetworkRequest request(url);
    if (!userAgent.isEmpty())
        request.setHeader(QNetworkRequest::UserAgentHeader, userAgent);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    QEventLoop loop;
    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    // Bound the wait: abort if the request does not finish in time.
    bool timedOut = false;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&] {
        timedOut = true;
        reply->abort();
        loop.quit();
    });
    if (m_timeoutMs > 0)
        timer.start(m_timeoutMs);

    loop.exec();

    core::HttpResponse result;
    result.status =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (timedOut) {
        result.ok = false;
        result.error = QStringLiteral("Request timed out");
    } else if (reply->error() == QNetworkReply::NoError) {
        result.ok = true;
        result.body = reply->readAll();
    } else {
        result.ok = false;
        result.error = reply->errorString();
    }

    reply->deleteLater();
    return result;
}

} // namespace uhf::net

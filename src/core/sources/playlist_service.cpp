#include "core/sources/playlist_service.h"

#include <utility>

#include "core/sources/http_client.h"
#include "core/sources/m3u_parser.h"
#include "core/sources/media_mapping.h"
#include "core/sources/xtream_mapper.h"

namespace uhf::core {

namespace {
void setError(QString* error, const QString& message)
{
    if (error)
        *error = message;
}
} // namespace

PlaylistService::PlaylistService(IHttpClient* http, QString userAgent)
    : m_http(http), m_userAgent(std::move(userAgent))
{
}

QList<MediaItem> PlaylistService::loadM3u(const QUrl& url, QString* error)
{
    setError(error, {});
    const HttpResponse resp = m_http->get(url, m_userAgent);
    if (!resp.ok) {
        setError(error, resp.error.isEmpty()
                            ? QStringLiteral("Failed to fetch playlist (HTTP %1)").arg(resp.status)
                            : resp.error);
        return {};
    }
    const M3UDocument doc = M3UParser::parse(QString::fromUtf8(resp.body));
    return liveItemsFromM3u(doc);
}

QList<MediaItem> PlaylistService::loadXtreamLive(const XtreamAccount& account, QString* error)
{
    setError(error, {});
    const XtreamApi api(account);

    const HttpResponse cats =
        m_http->get(api.actionUrl(QStringLiteral("get_live_categories")), m_userAgent);
    if (!cats.ok) {
        setError(error, cats.error.isEmpty()
                            ? QStringLiteral("Failed to fetch categories (HTTP %1)").arg(cats.status)
                            : cats.error);
        return {};
    }

    const HttpResponse streams =
        m_http->get(api.actionUrl(QStringLiteral("get_live_streams")), m_userAgent);
    if (!streams.ok) {
        setError(error, streams.error.isEmpty()
                            ? QStringLiteral("Failed to fetch streams (HTTP %1)").arg(streams.status)
                            : streams.error);
        return {};
    }

    const QHash<QString, QString> categories = XtreamMapper::parseCategories(cats.body);
    return XtreamMapper::parseLiveStreams(streams.body, categories, api);
}

} // namespace uhf::core

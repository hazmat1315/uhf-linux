#pragma once

#include <QList>
#include <QString>
#include <QUrl>

#include "core/model/media_item.h"
#include "core/sources/xtream_api.h"

namespace uhf::core {

class IHttpClient;

// Fetches and maps playlists (M3U URLs and Xtream accounts) using an injected
// HTTP client. Pure orchestration over the client; no Qt GUI.
class PlaylistService {
public:
    explicit PlaylistService(IHttpClient* http, QString userAgent = {});

    QList<MediaItem> loadM3u(const QUrl& url, QString* error = nullptr);
    QList<MediaItem> loadXtreamLive(const XtreamAccount& account, QString* error = nullptr);

private:
    IHttpClient* m_http;
    QString m_userAgent;
};

} // namespace uhf::core

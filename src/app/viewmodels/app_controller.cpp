#include "app/viewmodels/app_controller.h"

#include <QFile>
#include <QtConcurrent>
#include <QTextStream>
#include <QUrl>

#include "core/sources/http_client.h"
#include "core/sources/m3u_parser.h"
#include "core/sources/media_mapping.h"
#include "core/sources/playlist_service.h"

namespace uhf::app {

AppController::AppController(core::IPlayerBackend* backend, core::IHttpClient* http,
                            QObject* parent)
    : QObject(parent), m_player(backend), m_http(http)
{
    connect(&m_watcher, &QFutureWatcher<LoadResult>::finished, this, [this] {
        const LoadResult r = m_watcher.result();
        m_loading = false;
        emit loadingChanged();
        if (!r.error.isEmpty()) {
            setStatus(r.error);
            return;
        }
        setItems(r.items);
    });
}

void AppController::load(const QString& input)
{
    const QString trimmed = input.trimmed();
    if (trimmed.startsWith(QLatin1String("http://"))
        || trimmed.startsWith(QLatin1String("https://"))) {
        loadM3uUrl(trimmed);
    } else {
        loadM3uFile(trimmed);
    }
}

void AppController::loadM3uFile(const QString& path)
{
    const QString localPath =
        path.startsWith(QLatin1String("file:")) ? QUrl(path).toLocalFile() : path;

    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStatus(QStringLiteral("Could not open: %1").arg(localPath));
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    const core::M3UDocument doc = core::M3UParser::parse(stream.readAll());
    setItems(core::liveItemsFromM3u(doc));
}

void AppController::loadM3uUrl(const QString& url)
{
    if (!m_http) {
        setStatus(QStringLiteral("Networking unavailable"));
        return;
    }
    if (m_loading)
        return;

    m_loading = true;
    emit loadingChanged();
    setStatus(QStringLiteral("Loading %1 …").arg(url));

    core::IHttpClient* http = m_http;
    const QUrl u(url);
    m_watcher.setFuture(QtConcurrent::run([http, u]() -> LoadResult {
        core::PlaylistService service(http);
        QString error;
        QList<core::MediaItem> items = service.loadM3u(u, &error);
        return LoadResult{std::move(items), error};
    }));
}

void AppController::play(int index)
{
    const core::MediaItem* item = m_channels.at(index);
    if (!item)
        return;

    m_player.play(core::PlayableItem{item->id, item->name, item->url});
    m_nowPlaying = item->name;
    emit nowPlayingChanged();
    setStatus(QStringLiteral("Playing: %1").arg(item->name));
}

void AppController::setItems(QList<core::MediaItem> items)
{
    const int count = items.size();
    m_channels.setItems(std::move(items));
    setStatus(QStringLiteral("Loaded %1 channels").arg(count));
}

void AppController::setStatus(const QString& s)
{
    if (m_status != s) {
        m_status = s;
        emit statusChanged();
    }
}

} // namespace uhf::app

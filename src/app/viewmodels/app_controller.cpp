#include "app/viewmodels/app_controller.h"

#include <QFile>
#include <QTextStream>
#include <QUrl>

#include "core/sources/m3u_parser.h"

namespace uhf::app {

AppController::AppController(core::IPlayerBackend* backend, QObject* parent)
    : QObject(parent), m_player(backend)
{
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
    const QString text = stream.readAll();

    const core::M3UDocument doc = core::M3UParser::parse(text);

    QList<core::MediaItem> items;
    items.reserve(doc.entries.size());
    int n = 0;
    for (const core::M3UEntry& e : doc.entries) {
        core::MediaItem item;
        item.id = e.tvgId.isEmpty() ? QString::number(n) : e.tvgId;
        item.name = e.name;
        item.mediaType = core::MediaType::Live;
        item.categoryName = e.groupTitle;
        item.url = QUrl(e.url);
        item.logo = e.tvgLogo;
        items.append(item);
        ++n;
    }

    m_channels.setItems(items);
    setStatus(QStringLiteral("Loaded %1 channels").arg(items.size()));
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

void AppController::setStatus(const QString& s)
{
    if (m_status != s) {
        m_status = s;
        emit statusChanged();
    }
}

} // namespace uhf::app

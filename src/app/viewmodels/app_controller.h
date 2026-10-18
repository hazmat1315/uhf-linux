#pragma once

#include <QFutureWatcher>
#include <QList>
#include <QObject>
#include <QString>

#include "app/viewmodels/channel_list_model.h"
#include "core/model/media_item.h"
#include "core/player/player_controller.h"

namespace uhf::core {
class IPlayerBackend;
class IHttpClient;
} // namespace uhf::core

namespace uhf::app {

// Top-level view-model exposed to QML. Loads a playlist file, holds the channel
// model, and plays a selected channel through the core player.
class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QObject* channels READ channels CONSTANT)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString nowPlaying READ nowPlaying NOTIFY nowPlayingChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    // Backend is injected (windowed mpv in the app, a fake in tests). The HTTP
    // client is optional: when null, only local-file loading is available.
    explicit AppController(core::IPlayerBackend* backend,
                           core::IHttpClient* http = nullptr,
                           QObject* parent = nullptr);

    QObject* channels() { return &m_channels; }
    QString status() const { return m_status; }
    QString nowPlaying() const { return m_nowPlaying; }
    bool loading() const { return m_loading; }

    // Dispatches to a URL fetch or a local-file read based on the input.
    Q_INVOKABLE void load(const QString& input);
    Q_INVOKABLE void loadM3uFile(const QString& path);
    Q_INVOKABLE void loadM3uUrl(const QString& url);
    Q_INVOKABLE void play(int index);

signals:
    void statusChanged();
    void nowPlayingChanged();
    void loadingChanged();

private:
    void setStatus(const QString& s);
    void setItems(QList<core::MediaItem> items);

    struct LoadResult {
        QList<core::MediaItem> items;
        QString error;
    };

    ChannelListModel m_channels;
    core::PlayerController m_player;
    core::IHttpClient* m_http = nullptr;
    QFutureWatcher<LoadResult> m_watcher;
    bool m_loading = false;
    QString m_status;
    QString m_nowPlaying;
};

} // namespace uhf::app

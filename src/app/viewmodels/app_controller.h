#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include "app/viewmodels/channel_list_model.h"
#include "core/player/player_controller.h"

namespace uhf::core { class IPlayerBackend; }

namespace uhf::app {

// Top-level view-model exposed to QML. Loads a playlist file, holds the channel
// model, and plays a selected channel through the core player.
class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QObject* channels READ channels CONSTANT)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString nowPlaying READ nowPlaying NOTIFY nowPlayingChanged)

public:
    // Backend is injected (windowed mpv in the app, a fake in tests).
    explicit AppController(core::IPlayerBackend* backend, QObject* parent = nullptr);

    QObject* channels() { return &m_channels; }
    QString status() const { return m_status; }
    QString nowPlaying() const { return m_nowPlaying; }

    Q_INVOKABLE void loadM3uFile(const QString& path);
    Q_INVOKABLE void play(int index);

signals:
    void statusChanged();
    void nowPlayingChanged();

private:
    void setStatus(const QString& s);

    ChannelListModel m_channels;
    core::PlayerController m_player;
    QString m_status;
    QString m_nowPlaying;
};

} // namespace uhf::app

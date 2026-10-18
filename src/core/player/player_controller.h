#pragma once

#include <QList>
#include <QString>
#include <QUrl>

namespace uhf::core {

class IPlayerBackend;

// Minimal description of something playable (channel, movie, episode).
struct PlayableItem {
    QString id;
    QString name;
    QUrl url;
};

struct PlayerOptions {
    QString userAgent;
    QString referrer;
    int historyLimit = 50;
};

// Drives an IPlayerBackend and maintains channel history. Pure logic; no GUI.
class PlayerController {
public:
    explicit PlayerController(IPlayerBackend* backend, PlayerOptions options = {});

    void play(const PlayableItem& item);
    QList<PlayableItem> history() const;
    void clearHistory();

private:
    IPlayerBackend* m_backend;
    PlayerOptions m_options;
    QList<PlayableItem> m_history; // most-recent-first
};

} // namespace uhf::core

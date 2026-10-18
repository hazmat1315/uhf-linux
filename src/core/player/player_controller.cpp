#include "core/player/player_controller.h"

#include "core/player/player_backend.h"

#include <utility>

namespace uhf::core {

PlayerController::PlayerController(IPlayerBackend* backend, PlayerOptions options)
    : m_backend(backend), m_options(std::move(options))
{
}

void PlayerController::play(const PlayableItem& item)
{
    if (m_backend) {
        StreamRequest request;
        request.url = item.url;
        request.userAgent = m_options.userAgent;
        request.referrer = m_options.referrer;
        m_backend->load(request);
        m_backend->play();
    }

    // Move to front of history, de-duplicating by id.
    for (int i = 0; i < m_history.size(); ++i) {
        if (m_history.at(i).id == item.id) {
            m_history.removeAt(i);
            break;
        }
    }
    m_history.prepend(item);

    const int limit = m_options.historyLimit;
    if (limit >= 0) {
        while (m_history.size() > limit)
            m_history.removeLast();
    }
}

QList<PlayableItem> PlayerController::history() const
{
    return m_history;
}

void PlayerController::clearHistory()
{
    m_history.clear();
}

} // namespace uhf::core

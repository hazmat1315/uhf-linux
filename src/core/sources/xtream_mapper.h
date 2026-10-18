#pragma once

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QString>

#include "core/model/media_item.h"

namespace uhf::core {

class XtreamApi;

// Maps Xtream Codes player_api.php JSON responses into domain objects. Pure
// (no network); fed with already-fetched response bytes.
class XtreamMapper {
public:
    // get_live_categories / get_vod_categories / get_series_categories.
    static QHash<QString, QString> parseCategories(const QByteArray& json);

    // get_live_streams -> live MediaItems (stream URLs built via `api`).
    static QList<MediaItem> parseLiveStreams(const QByteArray& json,
                                             const QHash<QString, QString>& categories,
                                             const XtreamApi& api);
};

} // namespace uhf::core

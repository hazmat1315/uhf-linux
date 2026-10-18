#pragma once

#include <QList>

#include "core/model/media_item.h"
#include "core/sources/m3u_parser.h"

namespace uhf::core {

// Maps a parsed M3U document into live MediaItems (group-title -> category).
QList<MediaItem> liveItemsFromM3u(const M3UDocument& doc);

} // namespace uhf::core

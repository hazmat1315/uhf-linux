#include "core/sources/media_mapping.h"

namespace uhf::core {

QList<MediaItem> liveItemsFromM3u(const M3UDocument& doc)
{
    QList<MediaItem> items;
    items.reserve(doc.entries.size());
    int n = 0;
    for (const M3UEntry& e : doc.entries) {
        MediaItem item;
        item.id = e.tvgId.isEmpty() ? QString::number(n) : e.tvgId;
        item.name = e.name;
        item.mediaType = MediaType::Live;
        item.categoryName = e.groupTitle;
        item.url = QUrl(e.url);
        item.logo = e.tvgLogo;
        items.append(item);
        ++n;
    }
    return items;
}

} // namespace uhf::core

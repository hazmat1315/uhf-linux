#include "core/library/library_index.h"

#include <QSet>

#include <utility>

namespace uhf::core {

void LibraryIndex::setItems(QList<MediaItem> items)
{
    m_items = std::move(items);
}

QStringList LibraryIndex::categories(MediaType type) const
{
    QStringList result;
    QSet<QString> seen;
    for (const MediaItem& item : m_items) {
        if (item.mediaType != type)
            continue;
        if (!seen.contains(item.categoryName)) {
            seen.insert(item.categoryName);
            result.append(item.categoryName);
        }
    }
    return result;
}

QList<MediaItem> LibraryIndex::inCategory(MediaType type, const QString& category) const
{
    QList<MediaItem> result;
    for (const MediaItem& item : m_items) {
        if (item.mediaType == type && item.categoryName == category)
            result.append(item);
    }
    return result;
}

QList<MediaItem> LibraryIndex::search(MediaType type, const QString& query) const
{
    QList<MediaItem> result;
    for (const MediaItem& item : m_items) {
        if (item.mediaType == type
            && item.name.contains(query, Qt::CaseInsensitive)) {
            result.append(item);
        }
    }
    return result;
}

} // namespace uhf::core

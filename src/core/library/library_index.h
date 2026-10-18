#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include "core/model/media_item.h"

namespace uhf::core {

// In-memory index over a playlist's items: category listing, filtering by
// category, and case-insensitive name search ("Filter by name"). Pure; no I/O.
class LibraryIndex {
public:
    void setItems(QList<MediaItem> items);

    QStringList categories(MediaType type) const;
    QList<MediaItem> inCategory(MediaType type, const QString& category) const;
    QList<MediaItem> search(MediaType type, const QString& query) const;

private:
    QList<MediaItem> m_items;
};

} // namespace uhf::core

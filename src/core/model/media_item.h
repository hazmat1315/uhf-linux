#pragma once

#include <QString>
#include <QUrl>

namespace uhf::core {

enum class MediaType {
    Live,   // live TV channel
    Movie,  // VOD movie
    Series, // TV series
};

// A single browsable/playable item within a playlist.
struct MediaItem {
    QString id;
    QString name;
    MediaType mediaType = MediaType::Live;
    QString categoryName;
    QUrl url;
    QString logo;
};

} // namespace uhf::core

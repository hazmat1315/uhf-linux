#pragma once

#include <QList>
#include <QString>

namespace uhf::core {

// One playable entry parsed from an M3U/M3U8 playlist.
struct M3UEntry {
    QString name;         // title after the comma on the #EXTINF line
    QString url;          // the URL line following #EXTINF
    int duration = -1;    // #EXTINF duration in seconds (-1 = live/unknown)
    QString tvgId;        // tvg-id
    QString tvgName;      // tvg-name
    QString tvgLogo;      // tvg-logo
    QString tvgChno;      // tvg-chno (channel number)
    QString tvgCountry;   // tvg-country
    QString tvgLanguage;  // tvg-language
    QString tvgShift;     // tvg-shift
    QString groupTitle;   // group-title
};

// A parsed M3U document: header metadata plus entries.
struct M3UDocument {
    QString epgUrl;             // x-tvg-url / url-tvg from the #EXTM3U header
    QList<M3UEntry> entries;
};

class M3UParser {
public:
    static M3UDocument parse(const QString& text);
};

} // namespace uhf::core

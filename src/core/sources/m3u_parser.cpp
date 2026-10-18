#include "core/sources/m3u_parser.h"

#include <QRegularExpression>

namespace uhf::core {

namespace {

// Splits an #EXTINF payload into its attribute section and the trailing title,
// at the first comma that is not inside a quoted value.
void splitAttrsAndTitle(const QString& payload, QString& attrsOut, QString& titleOut)
{
    bool inQuotes = false;
    for (int i = 0; i < payload.size(); ++i) {
        const QChar c = payload.at(i);
        if (c == QLatin1Char('"')) {
            inQuotes = !inQuotes;
        } else if (c == QLatin1Char(',') && !inQuotes) {
            attrsOut = payload.left(i);
            titleOut = payload.mid(i + 1);
            return;
        }
    }
    attrsOut = payload;
    titleOut.clear();
}

QString attr(const QString& attrs, const QString& key)
{
    static const QString pattern = QStringLiteral("%1=\"([^\"]*)\"");
    const QRegularExpression re(pattern.arg(QRegularExpression::escape(key)));
    const QRegularExpressionMatch m = re.match(attrs);
    return m.hasMatch() ? m.captured(1) : QString();
}

} // namespace

M3UDocument M3UParser::parse(const QString& text)
{
    M3UDocument doc;

    const QStringList lines = text.split(QRegularExpression(QStringLiteral("\r\n|\n|\r")));
    M3UEntry pending;
    bool havePending = false;

    for (const QString& raw : lines) {
        const QString line = raw.trimmed();
        if (line.isEmpty())
            continue;

        if (line.startsWith(QLatin1String("#EXTM3U"))) {
            // EPG URL may be declared in the header as x-tvg-url or url-tvg.
            QString epg = attr(line, QStringLiteral("x-tvg-url"));
            if (epg.isEmpty())
                epg = attr(line, QStringLiteral("url-tvg"));
            if (!epg.isEmpty())
                doc.epgUrl = epg;
            continue;
        }

        if (line.startsWith(QLatin1String("#EXTINF:"))) {
            const QString payload = line.mid(QStringLiteral("#EXTINF:").size());

            QString attrs;
            QString title;
            splitAttrsAndTitle(payload, attrs, title);

            M3UEntry e;
            e.name = title.trimmed();

            // Duration is the leading numeric token of the attribute section.
            const QString durToken = attrs.section(QLatin1Char(' '), 0, 0);
            bool ok = false;
            const double d = durToken.toDouble(&ok);
            e.duration = ok ? static_cast<int>(d) : -1;

            e.tvgId = attr(attrs, QStringLiteral("tvg-id"));
            e.tvgName = attr(attrs, QStringLiteral("tvg-name"));
            e.tvgLogo = attr(attrs, QStringLiteral("tvg-logo"));
            e.tvgChno = attr(attrs, QStringLiteral("tvg-chno"));
            e.tvgCountry = attr(attrs, QStringLiteral("tvg-country"));
            e.tvgLanguage = attr(attrs, QStringLiteral("tvg-language"));
            e.tvgShift = attr(attrs, QStringLiteral("tvg-shift"));
            e.groupTitle = attr(attrs, QStringLiteral("group-title"));

            pending = e;
            havePending = true;
            continue;
        }

        if (line.startsWith(QLatin1Char('#')))
            continue; // other directives (ignored for now)

        // A non-directive line is the URL for the pending #EXTINF.
        if (havePending) {
            pending.url = line;
            doc.entries.append(pending);
            havePending = false;
        }
    }

    return doc;
}

} // namespace uhf::core

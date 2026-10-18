#include <QtTest>
#include "core/sources/m3u_parser.h"

using uhf::core::M3UParser;
using uhf::core::M3UDocument;
using uhf::core::M3UEntry;

class TestM3UParser : public QObject {
    Q_OBJECT

private slots:
    // A single #EXTINF entry: title, URL, and the standard tvg-* / group-title
    // attributes UHF's own parser recognises.
    void parsesSingleEntryWithAttributes()
    {
        const QString text = QStringLiteral(
            "#EXTM3U\n"
            "#EXTINF:-1 tvg-id=\"BBCOne.uk\" tvg-name=\"BBC One\" "
            "tvg-logo=\"http://logos/bbc1.png\" tvg-chno=\"101\" "
            "tvg-country=\"UK\" tvg-language=\"English\" group-title=\"News\",BBC One HD\n"
            "http://server/live/user/pass/101.ts\n");

        const M3UDocument doc = M3UParser::parse(text);

        QCOMPARE(doc.entries.size(), 1);
        const M3UEntry& e = doc.entries.at(0);
        QCOMPARE(e.name, QStringLiteral("BBC One HD"));
        QCOMPARE(e.url, QStringLiteral("http://server/live/user/pass/101.ts"));
        QCOMPARE(e.duration, -1);
        QCOMPARE(e.tvgId, QStringLiteral("BBCOne.uk"));
        QCOMPARE(e.tvgName, QStringLiteral("BBC One"));
        QCOMPARE(e.tvgLogo, QStringLiteral("http://logos/bbc1.png"));
        QCOMPARE(e.tvgChno, QStringLiteral("101"));
        QCOMPARE(e.tvgCountry, QStringLiteral("UK"));
        QCOMPARE(e.tvgLanguage, QStringLiteral("English"));
        QCOMPARE(e.groupTitle, QStringLiteral("News"));
    }

    // The #EXTM3U header can declare the EPG source via x-tvg-url or url-tvg.
    void parsesEpgUrlFromHeader()
    {
        const QString text = QStringLiteral(
            "#EXTM3U url-tvg=\"http://epg/guide.xml\" x-tvg-url=\"http://epg/other.xml\"\n");
        const M3UDocument doc = M3UParser::parse(text);
        // x-tvg-url takes precedence when both are present.
        QCOMPARE(doc.epgUrl, QStringLiteral("http://epg/other.xml"));
        QCOMPARE(doc.entries.size(), 0);
    }

    // A title may itself contain commas; only the first unquoted comma splits
    // attributes from the title.
    void keepsCommasInTitle()
    {
        const QString text = QStringLiteral(
            "#EXTM3U\n"
            "#EXTINF:-1 group-title=\"Movies\",The Good, the Bad and the Ugly\n"
            "http://server/movie/1.mkv\n");
        const M3UDocument doc = M3UParser::parse(text);
        QCOMPARE(doc.entries.size(), 1);
        QCOMPARE(doc.entries.at(0).name,
                 QStringLiteral("The Good, the Bad and the Ugly"));
        QCOMPARE(doc.entries.at(0).groupTitle, QStringLiteral("Movies"));
    }

    // Multiple entries, including a plain entry with no attributes.
    void parsesMultipleEntriesIncludingAttributeless()
    {
        const QString text = QStringLiteral(
            "#EXTM3U\n"
            "#EXTINF:-1 tvg-id=\"a\",Channel A\n"
            "http://server/a.ts\n"
            "#EXTINF:-1,Channel B\n"
            "http://server/b.ts\n");
        const M3UDocument doc = M3UParser::parse(text);
        QCOMPARE(doc.entries.size(), 2);
        QCOMPARE(doc.entries.at(0).name, QStringLiteral("Channel A"));
        QCOMPARE(doc.entries.at(0).tvgId, QStringLiteral("a"));
        QCOMPARE(doc.entries.at(1).name, QStringLiteral("Channel B"));
        QCOMPARE(doc.entries.at(1).url, QStringLiteral("http://server/b.ts"));
        QVERIFY(doc.entries.at(1).tvgId.isEmpty());
    }
};

QTEST_APPLESS_MAIN(TestM3UParser)
#include "test_m3u_parser.moc"

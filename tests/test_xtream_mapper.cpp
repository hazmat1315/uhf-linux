#include <QtTest>

#include "core/sources/xtream_api.h"
#include "core/sources/xtream_mapper.h"

using uhf::core::MediaItem;
using uhf::core::MediaType;
using uhf::core::XtreamAccount;
using uhf::core::XtreamApi;
using uhf::core::XtreamMapper;

// NOTE: fixtures use escaped string literals (not raw R"(...)" strings): moc's
// lexer mis-parses JSON raw strings and then fails to see the Q_OBJECT class.

class TestXtreamMapper : public QObject {
    Q_OBJECT

    static XtreamApi api()
    {
        return XtreamApi(XtreamAccount{QStringLiteral("http://box:8080"),
                                       QStringLiteral("u"), QStringLiteral("p")});
    }

private slots:
    // get_live_categories -> id->name map.
    void parsesCategories()
    {
        const QByteArray json =
            "[{\"category_id\":\"1\",\"category_name\":\"News\",\"parent_id\":0},"
            "{\"category_id\":\"2\",\"category_name\":\"Sports\",\"parent_id\":0}]";
        const auto cats = XtreamMapper::parseCategories(json);
        QCOMPARE(cats.value(QStringLiteral("1")), QStringLiteral("News"));
        QCOMPARE(cats.value(QStringLiteral("2")), QStringLiteral("Sports"));
    }

    // get_live_streams -> MediaItems with resolved category + built stream URL.
    void parsesLiveStreams()
    {
        const QByteArray cjson =
            "[{\"category_id\":\"1\",\"category_name\":\"News\",\"parent_id\":0}]";
        const auto cats = XtreamMapper::parseCategories(cjson);

        const QByteArray json =
            "[{\"num\":1,\"name\":\"BBC One\",\"stream_type\":\"live\",\"stream_id\":101,"
            "\"stream_icon\":\"http://logo/bbc1.png\",\"epg_channel_id\":\"bbc1\",\"category_id\":\"1\"},"
            "{\"num\":2,\"name\":\"CNN\",\"stream_type\":\"live\",\"stream_id\":202,"
            "\"stream_icon\":\"\",\"epg_channel_id\":\"cnn\",\"category_id\":\"1\"}]";

        const QList<MediaItem> items = XtreamMapper::parseLiveStreams(json, cats, api());
        QCOMPARE(items.size(), 2);

        QCOMPARE(items.at(0).id, QStringLiteral("101"));
        QCOMPARE(items.at(0).name, QStringLiteral("BBC One"));
        QCOMPARE(items.at(0).mediaType, MediaType::Live);
        QCOMPARE(items.at(0).categoryName, QStringLiteral("News"));
        QCOMPARE(items.at(0).logo, QStringLiteral("http://logo/bbc1.png"));
        QCOMPARE(items.at(0).url, QUrl(QStringLiteral("http://box:8080/live/u/p/101.ts")));

        QCOMPARE(items.at(1).id, QStringLiteral("202"));
        QCOMPARE(items.at(1).url, QUrl(QStringLiteral("http://box:8080/live/u/p/202.ts")));
    }

    // A stream whose category_id is unknown falls back to an empty category.
    void unknownCategoryYieldsEmptyName()
    {
        const QByteArray json =
            "[{\"name\":\"Orphan\",\"stream_id\":5,\"category_id\":\"999\"}]";
        const QList<MediaItem> items = XtreamMapper::parseLiveStreams(json, {}, api());
        QCOMPARE(items.size(), 1);
        QVERIFY(items.at(0).categoryName.isEmpty());
    }
};

QTEST_APPLESS_MAIN(TestXtreamMapper)
#include "test_xtream_mapper.moc"

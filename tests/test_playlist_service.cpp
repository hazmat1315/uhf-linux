#include <QtTest>

#include <QHash>

#include "core/sources/http_client.h"
#include "core/sources/playlist_service.h"
#include "core/sources/xtream_api.h"

using uhf::core::HttpResponse;
using uhf::core::IHttpClient;
using uhf::core::MediaItem;
using uhf::core::PlaylistService;
using uhf::core::XtreamAccount;

namespace {

// Serves canned bodies keyed by exact request URL.
class FakeHttpClient : public IHttpClient {
public:
    HttpResponse get(const QUrl& url, const QString& = {}) override
    {
        requested.append(url.toString(QUrl::FullyEncoded));
        const QString key = url.toString(QUrl::FullyEncoded);
        if (!bodies.contains(key))
            return HttpResponse{false, 404, {}, QStringLiteral("not found")};
        return HttpResponse{true, 200, bodies.value(key), {}};
    }

    QHash<QString, QByteArray> bodies;
    QStringList requested;
};

} // namespace

class TestPlaylistService : public QObject {
    Q_OBJECT

private slots:
    // Fetching an M3U URL parses + maps it into live items.
    void loadsM3uFromUrl()
    {
        FakeHttpClient http;
        const QString url = QStringLiteral("http://host/pl.m3u");
        http.bodies[url] =
            "#EXTM3U\n"
            "#EXTINF:-1 group-title=\"News\",BBC One\n"
            "http://host/live/bbc1.ts\n";

        PlaylistService service(&http);
        QString error;
        const QList<MediaItem> items = service.loadM3u(QUrl(url), &error);

        QVERIFY2(error.isEmpty(), qPrintable(error));
        QCOMPARE(items.size(), 1);
        QCOMPARE(items.at(0).name, QStringLiteral("BBC One"));
        QCOMPARE(items.at(0).url, QUrl(QStringLiteral("http://host/live/bbc1.ts")));
    }

    // A failed fetch reports an error and yields no items.
    void m3uFetchErrorReported()
    {
        FakeHttpClient http; // no bodies registered
        PlaylistService service(&http);
        QString error;
        const QList<MediaItem> items =
            service.loadM3u(QUrl(QStringLiteral("http://host/missing.m3u")), &error);
        QVERIFY(!error.isEmpty());
        QCOMPARE(items.size(), 0);
    }

    // Loading Xtream live fetches categories + streams and maps them.
    void loadsXtreamLive()
    {
        FakeHttpClient http;
        const XtreamAccount acct{QStringLiteral("http://box:8080"),
                                 QStringLiteral("u"), QStringLiteral("p")};

        http.bodies[QStringLiteral(
            "http://box:8080/player_api.php?username=u&password=p&action=get_live_categories")] =
            "[{\"category_id\":\"1\",\"category_name\":\"News\"}]";
        http.bodies[QStringLiteral(
            "http://box:8080/player_api.php?username=u&password=p&action=get_live_streams")] =
            "[{\"name\":\"BBC One\",\"stream_id\":101,\"category_id\":\"1\"}]";

        PlaylistService service(&http);
        QString error;
        const QList<MediaItem> items = service.loadXtreamLive(acct, &error);

        QVERIFY2(error.isEmpty(), qPrintable(error));
        QCOMPARE(items.size(), 1);
        QCOMPARE(items.at(0).name, QStringLiteral("BBC One"));
        QCOMPARE(items.at(0).categoryName, QStringLiteral("News"));
        QCOMPARE(items.at(0).url, QUrl(QStringLiteral("http://box:8080/live/u/p/101.ts")));
    }
};

QTEST_APPLESS_MAIN(TestPlaylistService)
#include "test_playlist_service.moc"

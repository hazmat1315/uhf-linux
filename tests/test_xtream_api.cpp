#include <QtTest>
#include "core/sources/xtream_api.h"

using uhf::core::XtreamAccount;
using uhf::core::XtreamApi;

class TestXtreamApi : public QObject {
    Q_OBJECT

    static XtreamApi makeApi()
    {
        return XtreamApi(XtreamAccount{
            QStringLiteral("http://box.example:8080"),
            QStringLiteral("user"),
            QStringLiteral("pa ss")}); // space to check URL-encoding
    }

private slots:
    // player_api.php action URL carries credentials + action as query items.
    void buildsActionUrl()
    {
        const QUrl url = makeApi().actionUrl(QStringLiteral("get_live_streams"));
        QCOMPARE(url.toString(QUrl::FullyEncoded),
                 QStringLiteral("http://box.example:8080/player_api.php"
                                "?username=user&password=pa%20ss&action=get_live_streams"));
    }

    // Action URL with an extra parameter (e.g. category_id).
    void buildsActionUrlWithParams()
    {
        const QUrl url = makeApi().actionUrl(
            QStringLiteral("get_vod_streams"),
            {{QStringLiteral("category_id"), QStringLiteral("5")}});
        QCOMPARE(url.toString(QUrl::FullyEncoded),
                 QStringLiteral("http://box.example:8080/player_api.php"
                                "?username=user&password=pa%20ss"
                                "&action=get_vod_streams&category_id=5"));
    }

    // Live stream URL: /live/<user>/<pass>/<id>.<ext>
    void buildsLiveStreamUrl()
    {
        const QUrl url = makeApi().liveStreamUrl(QStringLiteral("101"), QStringLiteral("ts"));
        QCOMPARE(url.toString(QUrl::FullyEncoded),
                 QStringLiteral("http://box.example:8080/live/user/pa%20ss/101.ts"));
    }

    // VOD stream URL: /movie/<user>/<pass>/<id>.<ext>
    void buildsVodStreamUrl()
    {
        const QUrl url = makeApi().vodStreamUrl(QStringLiteral("55"), QStringLiteral("mkv"));
        QCOMPARE(url.toString(QUrl::FullyEncoded),
                 QStringLiteral("http://box.example:8080/movie/user/pa%20ss/55.mkv"));
    }

    // Series episode URL: /series/<user>/<pass>/<id>.<ext>
    void buildsSeriesStreamUrl()
    {
        const QUrl url = makeApi().seriesStreamUrl(QStringLiteral("900"), QStringLiteral("mp4"));
        QCOMPARE(url.toString(QUrl::FullyEncoded),
                 QStringLiteral("http://box.example:8080/series/user/pa%20ss/900.mp4"));
    }
};

QTEST_APPLESS_MAIN(TestXtreamApi)
#include "test_xtream_api.moc"

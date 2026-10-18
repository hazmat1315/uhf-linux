#include <QtTest>

#include "core/player/player_backend.h"
#include "core/player/player_controller.h"

using uhf::core::IPlayerBackend;
using uhf::core::PlayableItem;
using uhf::core::PlayerController;
using uhf::core::PlayerOptions;
using uhf::core::StreamRequest;

namespace {

// Records what the controller asked the engine to do.
class FakeBackend : public IPlayerBackend {
public:
    void load(const StreamRequest& request) override
    {
        loads.append(request);
    }
    void play() override { ++playCount; }
    void pause() override { ++pauseCount; }
    void stop() override { ++stopCount; }

    QList<StreamRequest> loads;
    int playCount = 0;
    int pauseCount = 0;
    int stopCount = 0;
};

PlayableItem item(const QString& id, const QString& name, const QString& url)
{
    return PlayableItem{id, name, QUrl(url)};
}

} // namespace

class TestPlayerController : public QObject {
    Q_OBJECT

private slots:
    // play() loads the stream with the configured user-agent, then starts.
    void playLoadsWithHeadersThenPlays()
    {
        FakeBackend backend;
        PlayerOptions opts;
        opts.userAgent = QStringLiteral("UHF/1.0");
        PlayerController controller(&backend, opts);

        controller.play(item(QStringLiteral("1"), QStringLiteral("A"),
                             QStringLiteral("http://s/a.ts")));

        QCOMPARE(backend.loads.size(), 1);
        QCOMPARE(backend.loads.at(0).url, QUrl(QStringLiteral("http://s/a.ts")));
        QCOMPARE(backend.loads.at(0).userAgent, QStringLiteral("UHF/1.0"));
        QCOMPARE(backend.playCount, 1);
    }

    // Each played item is pushed onto history, most-recent-first.
    void recordsHistoryMostRecentFirst()
    {
        FakeBackend backend;
        PlayerController controller(&backend);
        controller.play(item(QStringLiteral("1"), QStringLiteral("A"), QStringLiteral("http://s/a")));
        controller.play(item(QStringLiteral("2"), QStringLiteral("B"), QStringLiteral("http://s/b")));

        const QList<PlayableItem> h = controller.history();
        QCOMPARE(h.size(), 2);
        QCOMPARE(h.at(0).id, QStringLiteral("2"));
        QCOMPARE(h.at(1).id, QStringLiteral("1"));
    }

    // Replaying an item moves it to the front without duplicating.
    void replayingMovesToFrontNoDuplicate()
    {
        FakeBackend backend;
        PlayerController controller(&backend);
        controller.play(item(QStringLiteral("1"), QStringLiteral("A"), QStringLiteral("http://s/a")));
        controller.play(item(QStringLiteral("2"), QStringLiteral("B"), QStringLiteral("http://s/b")));
        controller.play(item(QStringLiteral("1"), QStringLiteral("A"), QStringLiteral("http://s/a")));

        const QList<PlayableItem> h = controller.history();
        QCOMPARE(h.size(), 2);
        QCOMPARE(h.at(0).id, QStringLiteral("1"));
        QCOMPARE(h.at(1).id, QStringLiteral("2"));
    }

    // History is capped at the configured limit, dropping the oldest.
    void historyIsCappedAtLimit()
    {
        FakeBackend backend;
        PlayerOptions opts;
        opts.historyLimit = 2;
        PlayerController controller(&backend, opts);
        controller.play(item(QStringLiteral("1"), QStringLiteral("A"), QStringLiteral("http://s/a")));
        controller.play(item(QStringLiteral("2"), QStringLiteral("B"), QStringLiteral("http://s/b")));
        controller.play(item(QStringLiteral("3"), QStringLiteral("C"), QStringLiteral("http://s/c")));

        const QList<PlayableItem> h = controller.history();
        QCOMPARE(h.size(), 2);
        QCOMPARE(h.at(0).id, QStringLiteral("3"));
        QCOMPARE(h.at(1).id, QStringLiteral("2"));
    }

    // clearHistory() empties the list.
    void clearHistoryEmptiesIt()
    {
        FakeBackend backend;
        PlayerController controller(&backend);
        controller.play(item(QStringLiteral("1"), QStringLiteral("A"), QStringLiteral("http://s/a")));
        controller.clearHistory();
        QCOMPARE(controller.history().size(), 0);
    }
};

QTEST_APPLESS_MAIN(TestPlayerController)
#include "test_player_controller.moc"

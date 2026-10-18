#include <QtTest>

#include <QAbstractListModel>
#include <QFile>
#include <QTemporaryDir>

#include "app/viewmodels/app_controller.h"
#include "app/viewmodels/channel_list_model.h"
#include "core/player/player_backend.h"

using uhf::app::AppController;
using uhf::app::ChannelListModel;
using uhf::core::IPlayerBackend;
using uhf::core::StreamRequest;

namespace {

class FakeBackend : public IPlayerBackend {
public:
    void load(const StreamRequest& r) override { loads.append(r); }
    void play() override { ++playCount; }
    void pause() override {}
    void stop() override {}
    QList<StreamRequest> loads;
    int playCount = 0;
};

} // namespace

class TestAppController : public QObject {
    Q_OBJECT

    QTemporaryDir m_dir;
    QString m_m3u;

private slots:
    void initTestCase()
    {
        QVERIFY(m_dir.isValid());
        m_m3u = m_dir.filePath(QStringLiteral("playlist.m3u"));
        QFile f(m_m3u);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write(
            "#EXTM3U\n"
            "#EXTINF:-1 tvg-id=\"bbc1\" group-title=\"News\",BBC One\n"
            "http://example.com/live/bbc1.ts\n"
            "#EXTINF:-1 group-title=\"Sports\",ESPN\n"
            "http://example.com/live/espn.ts\n");
        f.close();
    }

    void loadingFilePopulatesChannelModel()
    {
        FakeBackend backend;
        AppController controller(&backend);
        controller.loadM3uFile(m_m3u);

        auto* model = qobject_cast<QAbstractListModel*>(controller.channels());
        QVERIFY(model != nullptr);
        QCOMPARE(model->rowCount(), 2);
        QCOMPARE(model->data(model->index(0, 0), ChannelListModel::NameRole).toString(),
                 QStringLiteral("BBC One"));
        QCOMPARE(model->data(model->index(1, 0), ChannelListModel::GroupRole).toString(),
                 QStringLiteral("Sports"));
    }

    void playDrivesBackendWithStreamUrl()
    {
        FakeBackend backend;
        AppController controller(&backend);
        controller.loadM3uFile(m_m3u);
        controller.play(1);

        QCOMPARE(backend.loads.size(), 1);
        QCOMPARE(backend.loads.at(0).url, QUrl(QStringLiteral("http://example.com/live/espn.ts")));
        QCOMPARE(backend.playCount, 1);
        QCOMPARE(controller.nowPlaying(), QStringLiteral("ESPN"));
    }

    void playWithInvalidIndexIsIgnored()
    {
        FakeBackend backend;
        AppController controller(&backend);
        controller.loadM3uFile(m_m3u);
        controller.play(99);
        QCOMPARE(backend.playCount, 0);
    }
};

QTEST_GUILESS_MAIN(TestAppController)
#include "test_app_controller.moc"

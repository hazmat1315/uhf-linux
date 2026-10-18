#include <QtTest>

#include <QElapsedTimer>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>

#include "core/player/player_backend.h"
#include "mpv/mpv_backend.h"

using uhf::core::StreamRequest;
using uhf::mpv::MpvBackend;

class TestMpvBackend : public QObject {
    Q_OBJECT

    QTemporaryDir m_dir;
    QString m_mediaPath;

private slots:
    void initTestCase()
    {
        QVERIFY(m_dir.isValid());
        m_mediaPath = m_dir.filePath(QStringLiteral("tone.wav"));

        // Generate a short audio file with ffmpeg (already installed).
        const QString ffmpeg = QStandardPaths::findExecutable(QStringLiteral("ffmpeg"));
        if (ffmpeg.isEmpty())
            QSKIP("ffmpeg not found; cannot generate test media");

        QProcess p;
        p.start(ffmpeg, {QStringLiteral("-f"), QStringLiteral("lavfi"),
                         QStringLiteral("-i"), QStringLiteral("sine=frequency=440:duration=0.2"),
                         QStringLiteral("-y"), m_mediaPath});
        QVERIFY(p.waitForFinished(15000));
        QVERIFY2(QFile::exists(m_mediaPath), "ffmpeg did not produce test media");
    }

    // libmpv initialises successfully.
    void initialisesCore()
    {
        MpvBackend backend;
        QVERIFY(backend.initialized());
    }

    // Loading + playing a real file drives decode to a known duration.
    void loadsAndDecodesFile()
    {
        MpvBackend backend;
        backend.setHeadless(true);
        QVERIFY(backend.initialized());

        StreamRequest req;
        req.url = QUrl::fromLocalFile(m_mediaPath);
        backend.load(req);
        backend.play();

        // Poll until mpv reports a positive duration (file decoded) or timeout.
        QElapsedTimer timer;
        timer.start();
        double duration = 0.0;
        while (timer.elapsed() < 5000) {
            duration = backend.doubleProperty(QStringLiteral("duration"), 0.0);
            if (duration > 0.0)
                break;
            QTest::qWait(50);
        }
        QVERIFY2(duration > 0.0, "mpv never reported a duration for the loaded file");
    }
};

QTEST_GUILESS_MAIN(TestMpvBackend)
#include "test_mpv_backend.moc"

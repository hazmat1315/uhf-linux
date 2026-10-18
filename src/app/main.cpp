#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "app/viewmodels/app_controller.h"
#include "mpv/mpv_backend.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("UHF Linux"));
    app.setOrganizationName(QStringLiteral("uhf-linux"));

    // Windowed mpv: video plays in mpv's own OS window for this first slice.
    uhf::mpv::MpvBackend backend(uhf::mpv::MpvBackend::Mode::Windowed);
    uhf::app::AppController controller(&backend);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("app"), &controller);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

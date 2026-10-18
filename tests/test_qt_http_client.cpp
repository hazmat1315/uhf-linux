#include <QtTest>

#include <QTcpServer>
#include <QTcpSocket>

#include "net/qt_http_client.h"

using uhf::core::HttpResponse;
using uhf::net::QtHttpClient;

// A trivial loopback HTTP server that answers one canned response, so the real
// QNetworkAccessManager path is exercised without any external network.
class TestQtHttpClient : public QObject {
    Q_OBJECT

    QTcpServer m_server;

private slots:
    void initTestCase()
    {
        QVERIFY(m_server.listen(QHostAddress::LocalHost));
        connect(&m_server, &QTcpServer::newConnection, this, [this] {
            QTcpSocket* sock = m_server.nextPendingConnection();
            connect(sock, &QTcpSocket::readyRead, this, [sock] {
                sock->readAll();
                const QByteArray body = "hello-body";
                QByteArray resp = "HTTP/1.1 200 OK\r\n";
                resp += "Content-Type: text/plain\r\n";
                resp += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
                resp += "Connection: close\r\n\r\n";
                resp += body;
                sock->write(resp);
                sock->flush();
                sock->disconnectFromHost();
            });
        });
    }

    void getReturnsBodyFromServer()
    {
        const QUrl url(QStringLiteral("http://127.0.0.1:%1/x").arg(m_server.serverPort()));
        QtHttpClient client;
        const HttpResponse r = client.get(url);
        QVERIFY2(r.ok, qPrintable(r.error));
        QCOMPARE(r.status, 200);
        QCOMPARE(r.body, QByteArray("hello-body"));
    }

    void getReportsConnectionError()
    {
        // Port 1 is closed; a short timeout bounds the wait so this can never
        // hang even where the OS does not refuse promptly.
        QtHttpClient client(1500);
        const HttpResponse r = client.get(QUrl(QStringLiteral("http://127.0.0.1:1/")));
        QVERIFY(!r.ok);
        QVERIFY(!r.error.isEmpty());
    }
};

QTEST_GUILESS_MAIN(TestQtHttpClient)
#include "test_qt_http_client.moc"

#pragma once

#include <QList>
#include <QPair>
#include <QString>
#include <QUrl>

namespace uhf::core {

// Connection details for an Xtream Codes account.
struct XtreamAccount {
    QString baseUrl;   // scheme://host[:port], no trailing slash
    QString username;
    QString password;
};

// Builds the Xtream Codes player_api.php and stream URLs.
// Pure (no network); safe to unit-test.
class XtreamApi {
public:
    using Param = QPair<QString, QString>;

    explicit XtreamApi(XtreamAccount account);

    QUrl actionUrl(const QString& action, const QList<Param>& params = {}) const;
    QUrl liveStreamUrl(const QString& streamId, const QString& ext) const;
    QUrl vodStreamUrl(const QString& streamId, const QString& ext) const;
    QUrl seriesStreamUrl(const QString& streamId, const QString& ext) const;

private:
    XtreamAccount m_account;
};

} // namespace uhf::core

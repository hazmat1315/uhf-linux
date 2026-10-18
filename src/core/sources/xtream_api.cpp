#include "core/sources/xtream_api.h"

#include <QUrlQuery>

#include <utility>

namespace uhf::core {

namespace {

QString enc(const QString& s)
{
    return QString::fromLatin1(QUrl::toPercentEncoding(s));
}

} // namespace

XtreamApi::XtreamApi(XtreamAccount account) : m_account(std::move(account)) {}

QUrl XtreamApi::actionUrl(const QString& action, const QList<Param>& params) const
{
    QUrl url(m_account.baseUrl + QStringLiteral("/player_api.php"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("username"), m_account.username);
    query.addQueryItem(QStringLiteral("password"), m_account.password);
    query.addQueryItem(QStringLiteral("action"), action);
    for (const Param& p : params)
        query.addQueryItem(p.first, p.second);
    url.setQuery(query);
    return url;
}

QUrl XtreamApi::liveStreamUrl(const QString& streamId, const QString& ext) const
{
    return QUrl(QStringLiteral("%1/live/%2/%3/%4.%5")
                    .arg(m_account.baseUrl, enc(m_account.username),
                         enc(m_account.password), enc(streamId), ext));
}

QUrl XtreamApi::vodStreamUrl(const QString& streamId, const QString& ext) const
{
    return QUrl(QStringLiteral("%1/movie/%2/%3/%4.%5")
                    .arg(m_account.baseUrl, enc(m_account.username),
                         enc(m_account.password), enc(streamId), ext));
}

QUrl XtreamApi::seriesStreamUrl(const QString& streamId, const QString& ext) const
{
    return QUrl(QStringLiteral("%1/series/%2/%3/%4.%5")
                    .arg(m_account.baseUrl, enc(m_account.username),
                         enc(m_account.password), enc(streamId), ext));
}

} // namespace uhf::core

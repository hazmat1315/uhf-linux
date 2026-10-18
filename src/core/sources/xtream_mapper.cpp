#include "core/sources/xtream_mapper.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "core/sources/xtream_api.h"

namespace uhf::core {

namespace {

// Xtream fields are inconsistently typed (id as int or string). Normalise.
QString asString(const QJsonValue& v)
{
    if (v.isString())
        return v.toString();
    if (v.isDouble()) {
        const double d = v.toDouble();
        const qlonglong i = static_cast<qlonglong>(d);
        if (static_cast<double>(i) == d)
            return QString::number(i);
        return QString::number(d);
    }
    return {};
}

} // namespace

QHash<QString, QString> XtreamMapper::parseCategories(const QByteArray& json)
{
    QHash<QString, QString> result;
    const QJsonArray arr = QJsonDocument::fromJson(json).array();
    for (const QJsonValue& v : arr) {
        const QJsonObject o = v.toObject();
        const QString id = asString(o.value(QStringLiteral("category_id")));
        if (!id.isEmpty())
            result.insert(id, o.value(QStringLiteral("category_name")).toString());
    }
    return result;
}

QList<MediaItem> XtreamMapper::parseLiveStreams(const QByteArray& json,
                                                const QHash<QString, QString>& categories,
                                                const XtreamApi& api)
{
    QList<MediaItem> result;
    const QJsonArray arr = QJsonDocument::fromJson(json).array();
    for (const QJsonValue& v : arr) {
        const QJsonObject o = v.toObject();

        MediaItem item;
        item.id = asString(o.value(QStringLiteral("stream_id")));
        item.name = o.value(QStringLiteral("name")).toString();
        item.mediaType = MediaType::Live;
        item.categoryName =
            categories.value(asString(o.value(QStringLiteral("category_id"))));
        item.logo = o.value(QStringLiteral("stream_icon")).toString();
        item.url = api.liveStreamUrl(item.id, QStringLiteral("ts"));
        result.append(item);
    }
    return result;
}

} // namespace uhf::core

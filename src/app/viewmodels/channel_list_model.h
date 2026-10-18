#pragma once

#include <QAbstractListModel>
#include <QList>

#include "core/model/media_item.h"

namespace uhf::app {

// Exposes a list of MediaItems to QML (name / group / url / id roles).
class ChannelListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        GroupRole,
        UrlRole,
        IdRole,
        LogoRole,
    };

    using QAbstractListModel::QAbstractListModel;

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(QList<core::MediaItem> items);
    const core::MediaItem* at(int row) const;

private:
    QList<core::MediaItem> m_items;
};

} // namespace uhf::app

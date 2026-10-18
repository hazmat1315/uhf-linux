#include "app/viewmodels/channel_list_model.h"

#include <utility>

namespace uhf::app {

int ChannelListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_items.size());
}

QVariant ChannelListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_items.size())
        return {};
    const core::MediaItem& item = m_items.at(index.row());
    switch (role) {
    case NameRole:
        return item.name;
    case GroupRole:
        return item.categoryName;
    case UrlRole:
        return item.url.toString();
    case IdRole:
        return item.id;
    case LogoRole:
        return item.logo;
    default:
        return {};
    }
}

QHash<int, QByteArray> ChannelListModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {GroupRole, "group"},
        {UrlRole, "url"},
        {IdRole, "itemId"},
        {LogoRole, "logo"},
    };
}

void ChannelListModel::setItems(QList<core::MediaItem> items)
{
    beginResetModel();
    m_items = std::move(items);
    endResetModel();
}

const core::MediaItem* ChannelListModel::at(int row) const
{
    if (row < 0 || row >= m_items.size())
        return nullptr;
    return &m_items.at(row);
}

} // namespace uhf::app

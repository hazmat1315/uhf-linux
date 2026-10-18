#include <QtTest>

#include "core/library/library_index.h"
#include "core/model/media_item.h"

using uhf::core::LibraryIndex;
using uhf::core::MediaItem;
using uhf::core::MediaType;

namespace {

MediaItem makeItem(const QString& id, const QString& name, MediaType type,
                   const QString& category)
{
    MediaItem i;
    i.id = id;
    i.name = name;
    i.mediaType = type;
    i.categoryName = category;
    return i;
}

QStringList ids(const QList<MediaItem>& items)
{
    QStringList out;
    for (const MediaItem& i : items)
        out << i.id;
    return out;
}

} // namespace

class TestLibraryIndex : public QObject {
    Q_OBJECT

    LibraryIndex makeIndex()
    {
        LibraryIndex index;
        index.setItems({
            makeItem(QStringLiteral("1"), QStringLiteral("BBC One"), MediaType::Live, QStringLiteral("News")),
            makeItem(QStringLiteral("2"), QStringLiteral("CNN International"), MediaType::Live, QStringLiteral("News")),
            makeItem(QStringLiteral("3"), QStringLiteral("ESPN"), MediaType::Live, QStringLiteral("Sports")),
            makeItem(QStringLiteral("4"), QStringLiteral("The Matrix"), MediaType::Movie, QStringLiteral("Action")),
        });
        return index;
    }

private slots:
    // Search is a case-insensitive substring match scoped to a media type.
    void searchIsCaseInsensitiveSubstringWithinType()
    {
        LibraryIndex index = makeIndex();
        const QList<MediaItem> r = index.search(MediaType::Live, QStringLiteral("cnn"));
        QCOMPARE(ids(r), QStringList{QStringLiteral("2")});
    }

    // Search does not cross media types (a Live search ignores movies).
    void searchDoesNotCrossMediaTypes()
    {
        LibraryIndex index = makeIndex();
        const QList<MediaItem> r = index.search(MediaType::Movie, QStringLiteral("e"));
        QCOMPARE(ids(r), QStringList{QStringLiteral("4")});
    }

    // Categories are distinct, in first-seen order, per media type.
    void categoriesAreDistinctInFirstSeenOrder()
    {
        LibraryIndex index = makeIndex();
        QCOMPARE(index.categories(MediaType::Live),
                 (QStringList{QStringLiteral("News"), QStringLiteral("Sports")}));
        QCOMPARE(index.categories(MediaType::Movie),
                 QStringList{QStringLiteral("Action")});
    }

    // inCategory returns items for that category and media type.
    void inCategoryReturnsMatchingItems()
    {
        LibraryIndex index = makeIndex();
        const QList<MediaItem> r = index.inCategory(MediaType::Live, QStringLiteral("News"));
        QCOMPARE(ids(r), (QStringList{QStringLiteral("1"), QStringLiteral("2")}));
    }
};

QTEST_APPLESS_MAIN(TestLibraryIndex)
#include "test_library_index.moc"

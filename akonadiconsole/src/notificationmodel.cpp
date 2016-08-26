/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "notificationmodel.h"

#include <QCoreApplication>
#include <QBuffer>

#include <QLocale>

#include "akonadiconsole_debug.h"
#include <AkonadiCore/ServerManager>

#include <akonadi/private/imapparser_p.h>
#include <akonadi/private/protocol_p.h>
#include <QMetaMethod>

using namespace Akonadi;

class NotificationModel::Item
{
public:
    Item(int type_, NotificationModel::Item *parent)
        : parent(parent)
        , type(type_)
    {}

    virtual ~Item()
    {
        qDeleteAll(nodes);
    }

    virtual QVariant data(int column) const = 0;

    NotificationModel::Item *parent;
    QVector<Item *> nodes;
    int type;
};

class NotificationModel::NotificationEntity: public NotificationModel::Item
{
public:
    NotificationEntity(Protocol::ChangeNotification::Id id,
                       const QString &remoteId, const QString &remoteRevision,
                       const QString &mimeType, NotificationModel::Item *parent)
        : NotificationModel::Item(-1, parent)
        , id(id)
        , remoteId(remoteId)
        , remoteRevision(remoteRevision)
        , mimeType(mimeType)
    {}

    NotificationEntity(NotificationModel::Item *parent)
        : NotificationModel::Item(-1, parent)
        , id(-1)
    {}

    QVariant data(int column) const Q_DECL_OVERRIDE
    {
        switch (column) {
        case 0:
            return id;
        case 1:
            return remoteId;
        case 2:
            return remoteRevision;
        case 3:
            return mimeType;
        default:
            return QVariant();
        }
    }

    Protocol::ChangeNotification::Id id;
    QString remoteId;
    QString remoteRevision;
    QString mimeType;
};

class NotificationModel::ItemNotificationNode: public NotificationModel::Item
{
public:
    ItemNotificationNode(const Protocol::ItemChangeNotification &msg,
                         NotificationModel::Item *parent)
        : NotificationModel::Item(Protocol::Command::ItemChangeNotification, parent)
        , msg(msg)
    {
        Q_FOREACH (const auto &item, msg.items()) {
            nodes << new NotificationEntity(item.id, item.remoteId,
                                            item.remoteRevision, item.mimeType,
                                            this);
        }
    }

    ~ItemNotificationNode()
    {}

    QVariant data(int column) const Q_DECL_OVERRIDE
    {
        switch (column) {
        case 0: {
            switch (msg.operation()) {
            case Protocol::ItemChangeNotification::Add: return QStringLiteral("Add");
            case Protocol::ItemChangeNotification::Modify: return QStringLiteral("Modify");
            case Protocol::ItemChangeNotification::ModifyFlags: return QStringLiteral("ModifyFlags");
            case Protocol::ItemChangeNotification::Move: return QStringLiteral("Move");
            case Protocol::ItemChangeNotification::Remove: return QStringLiteral("Delete");
            case Protocol::ItemChangeNotification::Link: return QStringLiteral("Link");
            case Protocol::ItemChangeNotification::Unlink: return QStringLiteral("Unlink");
            default: return QStringLiteral("Invalid");
            }
        }
        case 1: {
            return QStringLiteral("Items");
        }
        case 2:
            return QString::fromUtf8(msg.sessionId());
        case 3:
            return QString::fromUtf8(msg.resource());
        case 4:
            return QString::fromUtf8(msg.destinationResource());
        case 5:
            return QString::number(msg.parentCollection());
        case 6:
            return QString::number(msg.parentDestCollection());
        case 7:
            return QString::fromUtf8(Akonadi::ImapParser::join(msg.itemParts(), ", "));
        case 8:
            return QString::fromUtf8(Akonadi::ImapParser::join(msg.addedFlags(), ", "));
        case 9:
            return QString::fromUtf8(Akonadi::ImapParser::join(msg.removedFlags(), ", "));
        default:
            return QString();
        }
    }

    Protocol::ItemChangeNotification msg;
};

class NotificationModel::CollectionNotificationNode : public NotificationModel::Item
{
public:
    CollectionNotificationNode(const Protocol::CollectionChangeNotification &msg,
                               NotificationModel::Item *parent)
        : NotificationModel::Item(Protocol::Command::CollectionChangeNotification, parent)
        , msg(msg)
    {
        nodes << new NotificationEntity(msg.id(), msg.remoteId(), msg.remoteRevision(),
                                        QString(), this);
    }

    ~CollectionNotificationNode()
    {}

    QVariant data(int column) const Q_DECL_OVERRIDE
    {
        switch (column) {
        case 0: {
            switch (msg.operation()) {
            case Protocol::CollectionChangeNotification::Add: return QStringLiteral("Add");
            case Protocol::CollectionChangeNotification::Modify: return QStringLiteral("Modify");
            case Protocol::CollectionChangeNotification::Move: return QStringLiteral("Move");
            case Protocol::CollectionChangeNotification::Remove: return QStringLiteral("Delete");
            case Protocol::CollectionChangeNotification::Subscribe: return QStringLiteral("Subscribe");
            case Protocol::CollectionChangeNotification::Unsubscribe: return QStringLiteral("Unsubscribe");
            default: return QStringLiteral("Invalid");
            }
        }
        case 1: {
            return QStringLiteral("Collections");
        }
        case 2:
            return QString::fromUtf8(msg.sessionId());
        case 3:
            return QString::fromUtf8(msg.resource());
        case 4:
            return QString::fromUtf8(msg.destinationResource());
        case 5:
            return QString::number(msg.parentCollection());
        case 6:
            return QString::number(msg.parentDestCollection());
        case 7:
            return QString::fromUtf8(Akonadi::ImapParser::join(msg.changedParts(), ", "));
        default:
            return QString();
        }
    }

    Protocol::CollectionChangeNotification msg;
};

class NotificationModel::TagNotificationNode : public NotificationModel::Item
{
public:
    TagNotificationNode(const Protocol::TagChangeNotification &msg,
                        NotificationModel::Item *parent)
        : NotificationModel::Item(Protocol::Command::TagChangeNotification, parent)
        , msg(msg)
    {
        nodes << new NotificationEntity(msg.id(), msg.remoteId(), QString(),
                                        QString(), this);
    }

    ~TagNotificationNode()
    {}

    QVariant data(int column) const Q_DECL_OVERRIDE
    {
        switch (column) {
        case 0: {
            switch (msg.operation()) {
            case Protocol::TagChangeNotification::Add: return QStringLiteral("Add");
            case Protocol::TagChangeNotification::Modify: return QStringLiteral("Modify");
            case Protocol::TagChangeNotification::Remove: return QStringLiteral("Delete");
            default: return QStringLiteral("Invalid");
            }
        }
        case 1: {
            return QStringLiteral("Tags");
        }
        case 2:
            return QString::fromUtf8(msg.sessionId());
        case 3:
            return QString::fromUtf8(msg.resource());
        default:
            return QString();
        }
    }

    Protocol::TagChangeNotification msg;
};

class NotificationModel::RelationNotificationNode : public NotificationModel::Item
{
public:
    RelationNotificationNode(const Protocol::RelationChangeNotification &msg,
                             NotificationModel::Item *parent)
        : NotificationModel::Item(Protocol::Command::RelationChangeNotification, parent)
        , msg(msg)
    {
    }

    ~RelationNotificationNode()
    {}

    QVariant data(int column) const Q_DECL_OVERRIDE
    {
        switch (column) {
        case 0: {
            switch (msg.operation()) {
            case Protocol::RelationChangeNotification::Add: return QStringLiteral("Add");
            case Protocol::RelationChangeNotification::Remove: return QStringLiteral("Delete");
            default: return QStringLiteral("Invalid");
            }
        }
        case 1: {
            return QStringLiteral("Relation");
        }
        case 2:
            return QString::fromUtf8(msg.sessionId());
        case 3:
            return QString::number(msg.leftItem());
        case 4:
            return QString::number(msg.rightItem());
        case 5:
            return msg.remoteId();
        default:
            return QString();
        }
    }

    Protocol::RelationChangeNotification msg;
};

class NotificationModel::NotificationBlock: public NotificationModel::Item
{
public:
    NotificationBlock(const Akonadi::Protocol::ChangeNotification::List &msgs)
        : NotificationModel::Item(-2, Q_NULLPTR)
    {
        timestamp = QDateTime::currentDateTime();
        Q_FOREACH (const Protocol::ChangeNotification &msg, msgs) {
            switch (msg.type()) {
            case Protocol::Command::ItemChangeNotification:
                nodes << new ItemNotificationNode(msg, this);
                break;
            case Protocol::Command::CollectionChangeNotification:
                nodes << new CollectionNotificationNode(msg, this);
                break;
            case Protocol::Command::TagChangeNotification:
                nodes << new TagNotificationNode(msg, this);
                break;
            case Protocol::Command::RelationChangeNotification:
                nodes << new RelationNotificationNode(msg, this);
                break;
            default:
                qWarning() << "Unknown Notification type" << msg.type();
                break;
            }
        }
    }

    ~NotificationBlock()
    {}

    QVariant data(int column) const Q_DECL_OVERRIDE
    {
        switch (column) {
        case 0:
            return QStringLiteral("%1.%2").arg(QLocale().toString(timestamp.time()))
                   .arg(timestamp.time().msec(), 3, 10, QLatin1Char('0'));
        case 1:
            return nodes.count();
        default:
            return QVariant();
        }
    }

    QDateTime timestamp;
};

NotificationModel::NotificationModel(QObject *parent) :
    QAbstractItemModel(parent),
    m_monitor(Q_NULLPTR)
{
    QString service = QStringLiteral("org.freedesktop.Akonadi");
    if (Akonadi::ServerManager::hasInstanceIdentifier()) {
        service += QLatin1String(".") + Akonadi::ServerManager::instanceIdentifier();
    }
}

NotificationModel::~NotificationModel()
{
    setEnabled(false);
}

int NotificationModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 10;
}

int NotificationModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_data.size();
    }

    Item *parentItem = static_cast<Item *>(parent.internalPointer());
    if (parentItem) {
        return parentItem->nodes.count();
    } else {
        return m_data.size();
    }

    return 0;
}

QModelIndex NotificationModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (row >= m_data.count()) {
            return QModelIndex();
        }
        return createIndex(row, column, m_data.at(row));
    }

    Item *parentItem = static_cast<Item *>(parent.internalPointer());
    if (parentItem) {
        Item *item = parentItem->nodes.at(row);
        return createIndex(row, column, item);
    } else {
        return createIndex(row, column, m_data.at(row));
    }
}

QModelIndex NotificationModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    Item *childItem = static_cast<Item *>(child.internalPointer());
    if (childItem) {
        Item *parent = childItem->parent;
        if (!parent) {
            return QModelIndex();
        }

        const int parentIndex = parent->parent ? parent->parent->nodes.indexOf(childItem) : m_data.indexOf(childItem);

        return createIndex(parentIndex, 0, parent);
    }

    return QModelIndex();
}

QVariant NotificationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Item *item = static_cast<Item *>(index.internalPointer());
    if (!item) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return item->data(index.column());
    }

    return QVariant();
}

QVariant NotificationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QStringLiteral("Operation / ID");
        case 1: return QStringLiteral("Type / RID");
        case 2: return QStringLiteral("Session / REV");
        case 3: return QStringLiteral("Resource / MimeType");
        case 4: return QStringLiteral("Destination Resource");
        case 5: return QStringLiteral("Parent");
        case 6: return QStringLiteral("Destination");
        case 7: return QStringLiteral("Parts");
        case 8: return QStringLiteral("Added Flags");
        case 9: return QStringLiteral("Removed Flags");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

void NotificationModel::slotNotify(const QVector<QByteArray> &data)
{
    Protocol::ChangeNotification::List ntfs;
    ntfs.reserve(data.size());
    Q_FOREACH (const QByteArray &d, data) {
        QBuffer buffer(const_cast<QByteArray *>(&d));
        buffer.open(QIODevice::ReadOnly);
        ntfs << Protocol::deserialize(&buffer);
    }

    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
    m_data.append(new NotificationBlock(ntfs));
    endInsertRows();
}

void NotificationModel::clear()
{
    qDeleteAll(m_data);
    m_data.clear();
    reset();
}

void NotificationModel::setEnabled(bool enable)
{
    if (enable) {
        m_monitor = new Akonadi::Monitor(this);
        m_monitor->setAllMonitored(true);
        m_monitor->setExclusive(true);

    } else {
        m_monitor->deleteLater();
        m_monitor = Q_NULLPTR;
    }
}


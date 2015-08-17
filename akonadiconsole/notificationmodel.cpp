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
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusPendingCall>

#include <KLocale>

#include "akonadiconsole_debug.h"
#include <AkonadiCore/ServerManager>

#include <akonadi/private/imapparser_p.h>
#include <akonadi/private/protocol_p.h>
#include <QMetaMethod>

using namespace Akonadi;

class NotificationModel::Item
{
public:
    Item(int type_): type(type_) {}
    virtual ~Item() {}

    int type;
};

class NotificationModel::NotificationBlock: public NotificationModel::Item
{
public:
    NotificationBlock(const Akonadi::Protocol::ChangeNotification::List &msgs);
    ~NotificationBlock();

    QList<NotificationNode *> nodes;
    QDateTime timestamp;
};

class NotificationModel::NotificationNode: public NotificationModel::Item
{
public:
    NotificationNode(const Protocol::ChangeNotification &msg_, NotificationBlock *parent_);
    ~NotificationNode();

    QByteArray sessionId;
    Protocol::ChangeNotification::Type type;
    Protocol::ChangeNotification::Operation operation;
    QByteArray resource;
    QByteArray destResource;
    Protocol::ChangeNotification::Id parentCollection;
    Protocol::ChangeNotification::Id destCollection;
    QSet<QByteArray> parts;
    QSet<QByteArray> addedFlags;
    QSet<QByteArray> removedFlags;
    QList<NotificationEntity *> entities;
    NotificationBlock *parent;
};

class NotificationModel::NotificationEntity: public NotificationModel::Item
{
public:
    NotificationEntity(const Protocol::ChangeNotification::Entity &entity, NotificationNode *parent_);

    Protocol::ChangeNotification::Id id;
    QString remoteId;
    QString remoteRevision;
    QString mimeType;
    NotificationNode *parent;
};

NotificationModel::NotificationBlock::NotificationBlock(const Protocol::ChangeNotification::List &msgs) :
    Item(0)
{
    timestamp = QDateTime::currentDateTime();
    Q_FOREACH (const Protocol::ChangeNotification &msg, msgs) {
        nodes << new NotificationNode(msg, this);
    }
}

NotificationModel::NotificationBlock::~NotificationBlock()
{
    qDeleteAll(nodes);
}

NotificationModel::NotificationNode::NotificationNode(const Protocol::ChangeNotification &msg, NotificationModel::NotificationBlock *parent_):
    Item(1),
    sessionId(msg.sessionId()),
    type(msg.type()),
    operation(msg.operation()),
    resource(msg.resource()),
    destResource(msg.destinationResource()),
    parentCollection(msg.parentCollection()),
    destCollection(msg.parentDestCollection()),
    parts(msg.itemParts()),
    addedFlags(msg.addedFlags()),
    removedFlags(msg.removedFlags()),
    parent(parent_)
{
    Q_FOREACH (const Protocol::ChangeNotification::Entity &entity, msg.entities()) {
        entities << new NotificationEntity(entity, this);
    }
}

NotificationModel::NotificationNode::~NotificationNode()
{
    qDeleteAll(entities);
}

NotificationModel::NotificationEntity::NotificationEntity(const Protocol::ChangeNotification::Entity &entity, NotificationModel::NotificationNode *parent_):
    Item(2),
    id(entity.id),
    remoteId(entity.remoteId),
    remoteRevision(entity.remoteRevision),
    mimeType(entity.mimeType),
    parent(parent_)
{
}

NotificationModel::NotificationModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    QString service = QStringLiteral("org.freedesktop.Akonadi");
    if (Akonadi::ServerManager::hasInstanceIdentifier()) {
        service += QLatin1String(".") + Akonadi::ServerManager::instanceIdentifier();
    }
    m_manager = new QDBusInterface(service, QStringLiteral("/notifications/debug"),
                                   QStringLiteral("org.freedesktop.Akonadi.NotificationManager"),
                                   QDBusConnection::sessionBus(),
                                   this);
    if (!m_manager->isValid()) {
        qCWarning(AKONADICONSOLE_LOG) << "Unable to connect to notification manager";
        delete m_manager;
        m_manager = 0;
    } else {
        connect(m_manager, SIGNAL(debugNotify(QVector<QByteArray>)),
                this, SLOT(slotNotify(QVector<QByteArray>)));
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
        if (parentItem->type == 0) {
            return static_cast<NotificationBlock *>(parentItem)->nodes.count();
        } else if (parentItem->type == 1) {
            return static_cast<NotificationNode *>(parentItem)->entities.count();
        }
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
        if (parentItem ->type == 0) {
            NotificationBlock *parentBlock = static_cast<NotificationBlock *>(parentItem);
            return createIndex(row, column, parentBlock->nodes.at(row));
        } else if (parentItem->type == 1) {
            NotificationNode *parentNode = static_cast<NotificationNode *>(parentItem);
            return createIndex(row, column, parentNode->entities.at(row));
        }
    }

    return QModelIndex();
}

QModelIndex NotificationModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    Item *childItem = static_cast<Item *>(child.internalPointer());
    if (childItem) {
        if (childItem->type == 0) {
            return QModelIndex();
        } else if (childItem->type == 1) {
            NotificationNode *childNode = static_cast<NotificationNode *>(childItem);
            return createIndex(m_data.indexOf(childNode->parent), 0, childNode->parent);
        } else if (childItem->type == 2) {
            NotificationEntity *childEntity = static_cast<NotificationEntity *>(childItem);
            NotificationNode *parentNode = childEntity->parent;
            return createIndex(parentNode->parent->nodes.indexOf(parentNode), 0, childEntity->parent);
        }
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

    if (item->type == 0) {
        NotificationBlock *block = static_cast<NotificationBlock *>(item);
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0:
                return QString(KLocale::global()->formatTime(block->timestamp.time(), true) +
                               QStringLiteral(".%1").arg(block->timestamp.time().msec(), 3, 10, QLatin1Char('0')));
            case 1:
                return block->nodes.count();
            }
        }
    } else if (item->type == 1) {
        NotificationNode *node = static_cast<NotificationNode *>(item);
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0: {
                switch (node->operation) {
                case Protocol::ChangeNotification::Add: return QStringLiteral("Add");
                case Protocol::ChangeNotification::Modify: return QStringLiteral("Modify");
                case Protocol::ChangeNotification::ModifyFlags: return QStringLiteral("ModifyFlags");
                case Protocol::ChangeNotification::ModifyTags: return QStringLiteral("ModifyTags");
                case Protocol::ChangeNotification::Move: return QStringLiteral("Move");
                case Protocol::ChangeNotification::Remove: return QStringLiteral("Delete");
                case Protocol::ChangeNotification::Link: return QStringLiteral("Link");
                case Protocol::ChangeNotification::Unlink: return QStringLiteral("Unlink");
                case Protocol::ChangeNotification::Subscribe: return QStringLiteral("Subscribe");
                case Protocol::ChangeNotification::Unsubscribe: return QStringLiteral("Unsubscribe");
                default: return QStringLiteral("Invalid");
                }
            }
            case 1: {
                switch (node->type) {
                case Protocol::ChangeNotification::Collections: return QStringLiteral("Collections");
                case Protocol::ChangeNotification::Items: return QStringLiteral("Items");
                case Protocol::ChangeNotification::Tags: return QStringLiteral("Tags");
                default: return QStringLiteral("Invalid");
                }
            }
            case 2:
                return QString::fromUtf8(node->sessionId);
            case 3:
                return QString::fromUtf8(node->resource);
            case 4:
                return QString::fromUtf8(node->destResource);
            case 5:
                return QString::number(node->parentCollection);
            case 6:
                return QString::number(node->destCollection);
            case 7:
                return QString::fromUtf8(Akonadi::ImapParser::join(node->parts, ", "));
            case 8:
                return QString::fromUtf8(Akonadi::ImapParser::join(node->addedFlags, ", "));
            case 9:
                return QString::fromUtf8(Akonadi::ImapParser::join(node->removedFlags, ", "));
            }
        }
    } else if (item->type == 2) {
        NotificationEntity *entity = static_cast<NotificationEntity *>(item);
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0:
                return entity->id;
            case 1:
                return entity->remoteId;
            case 2:
                return entity->remoteRevision;
            case 3:
                return entity->mimeType;
            }
        }
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
    if (m_manager) {
        m_manager->asyncCall(QStringLiteral("enableDebug"), enable);
    }
}


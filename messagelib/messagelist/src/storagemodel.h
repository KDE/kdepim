/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef __MESSAGELIST_STORAGEMODEL_H__
#define __MESSAGELIST_STORAGEMODEL_H__

#include <messagelist_export.h>
#include <messagelist/storagemodelbase.h>

#include <collection.h>
#include <item.h>

#include <kmime/kmime_message.h>

class QAbstractItemModel;
class QItemSelectionModel;

namespace Akonadi
{
class Item;
}

namespace MessageList
{

namespace Core
{
class MessageItem;
}

/**
 * The Akonadi specific implementation of the Core::StorageModel.
 */
class MESSAGELIST_EXPORT StorageModel : public MessageList::Core::StorageModel
{
    Q_OBJECT

public:
    /**
    * Create a StorageModel wrapping the specified folder.
    */
    explicit StorageModel(QAbstractItemModel *model, QItemSelectionModel *selectionModel, QObject *parent = Q_NULLPTR);
    ~StorageModel();

    Akonadi::Collection::List displayedCollections() const;

    QString id() const Q_DECL_OVERRIDE;
    bool containsOutboundMessages() const Q_DECL_OVERRIDE;

    virtual bool isOutBoundFolder(const Akonadi::Collection &c) const;

    int initialUnreadRowCountGuess() const Q_DECL_OVERRIDE;
    bool initializeMessageItem(MessageList::Core::MessageItem *mi, int row, bool bUseReceiver) const Q_DECL_OVERRIDE;
    void fillMessageItemThreadingData(MessageList::Core::MessageItem *mi, int row, ThreadingDataSubset subset) const Q_DECL_OVERRIDE;
    void updateMessageItemData(MessageList::Core::MessageItem *mi, int row) const Q_DECL_OVERRIDE;
    void setMessageItemStatus(MessageList::Core::MessageItem *mi, int row, Akonadi::MessageStatus status) Q_DECL_OVERRIDE;

    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QMimeData *mimeData(const QList< MessageList::Core::MessageItem * > &) const Q_DECL_OVERRIDE;
    using MessageList::Core::StorageModel::mimeData;

    void prepareForScan() Q_DECL_OVERRIDE;

    Akonadi::Item itemForRow(int row) const;
    Akonadi::Collection parentCollectionForRow(int row) const;
    KMime::Message::Ptr messageForRow(int row) const;

    void resetModelStorage();

private:
    Q_PRIVATE_SLOT(d, void onSourceDataChanged(const QModelIndex &, const QModelIndex &))
    Q_PRIVATE_SLOT(d, void onSelectionChanged())
    Q_PRIVATE_SLOT(d, void loadSettings())

    class Private;
    Private *const d;
};

} // namespace MessageList

#endif //!__MESSAGELIST_STORAGEMODEL_H__

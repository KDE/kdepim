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

#include <messagelist/core/storagemodelbase.h>

#include <collection.h>
#include <item.h>

#include <QColor>
#include <QFont>

#include <KMime/kmime_message.h>

#include <messagelist/messagelist_export.h>

class QAbstractItemModel;
class QItemSelectionModel;

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
    explicit StorageModel( QAbstractItemModel *model, QItemSelectionModel *selectionModel, QObject *parent = 0 );
    ~StorageModel();

    Akonadi::Collection::List displayedCollections() const;

    virtual QString id() const;
    virtual bool containsOutboundMessages() const;

    virtual bool isOutBoundFolder( const Akonadi::Collection& c ) const;

    virtual int initialUnreadRowCountGuess() const;
    virtual bool initializeMessageItem( MessageList::Core::MessageItem *mi, int row, bool bUseReceiver ) const;
    virtual void fillMessageItemThreadingData( MessageList::Core::MessageItem *mi, int row, ThreadingDataSubset subset ) const;
    virtual void updateMessageItemData( MessageList::Core::MessageItem *mi, int row ) const;
    virtual void setMessageItemStatus( MessageList::Core::MessageItem *mi, int row, const Akonadi::MessageStatus &status );

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex &index ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QMimeData* mimeData( const QList< MessageList::Core::MessageItem* >& ) const;
    using MessageList::Core::StorageModel::mimeData;

    virtual void prepareForScan();

    Akonadi::Item itemForRow( int row ) const;
    KMime::Message::Ptr messageForRow( int row ) const;

    void resetModelStorage();

private:
    Q_PRIVATE_SLOT(d, void onSourceDataChanged( const QModelIndex&, const QModelIndex& ))
    Q_PRIVATE_SLOT(d, void onSelectionChanged())
    Q_PRIVATE_SLOT(d, void loadSettings())

    class Private;
    Private * const d;
};

} // namespace MessageList

#endif //!__MESSAGELIST_STORAGEMODEL_H__

/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "snippetsmodel_p.h"

using namespace MailCommon;

class MailCommon::SnippetItem
{
  public:
    SnippetItem( bool isGroup = false, SnippetItem *parent = 0 );
    ~SnippetItem();

    bool isGroup() const;

    void setName( const QString &name );
    QString name() const;

    void setText( const QString &text );
    QString text() const;

    void setKeySequence( const QString &sequence );
    QString keySequence() const;

    void appendChild( SnippetItem *child );
    void removeChild( SnippetItem *child );
    SnippetItem *child( int row ) const;
    int childCount() const;
    int row() const;
    SnippetItem *parent() const;

  private:
    QList<SnippetItem*> mChildItems;
    SnippetItem *mParentItem;

    bool mIsGroup;
    QString mName;
    QString mText;
    QString mKeySequence;
};

SnippetItem::SnippetItem( bool isGroup, SnippetItem *parent )
  : mParentItem( parent ), mIsGroup( isGroup )
{
}

SnippetItem::~SnippetItem()
{
  qDeleteAll( mChildItems );
}

bool SnippetItem::isGroup() const
{
  return mIsGroup;
}

void SnippetItem::setName( const QString &name )
{
  mName = name;
}

QString SnippetItem::name() const
{
  return mName;
}

void SnippetItem::setText( const QString &text )
{
  mText = text;
}

QString SnippetItem::text() const
{
  return mText;
}

void SnippetItem::setKeySequence( const QString &sequence )
{
  mKeySequence = sequence;
}

QString SnippetItem::keySequence() const
{
  return mKeySequence;
}

void SnippetItem::appendChild( SnippetItem *item )
{
  mChildItems.append( item );
}

void SnippetItem::removeChild( SnippetItem *item )
{
  mChildItems.removeAll( item );
  delete item;
}

SnippetItem *SnippetItem::child( int row ) const
{
  return mChildItems.value( row );
}

int SnippetItem::childCount() const
{
  return mChildItems.count();
}

SnippetItem *SnippetItem::parent() const
{
  return mParentItem;
}

int SnippetItem::row() const
{
  if ( mParentItem )
    return mParentItem->mChildItems.indexOf( const_cast<SnippetItem*>( this ) );

  return 0;
}


SnippetsModel::SnippetsModel( QObject *parent )
  : QAbstractItemModel( parent )
{
  mRootItem = new SnippetItem( true );
  mRootItem->setText( "Root" );
}

SnippetsModel::~SnippetsModel()
{
  delete mRootItem;
}

int SnippetsModel::columnCount( const QModelIndex& ) const
{
  return 1;
}

bool SnippetsModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  SnippetItem *item = static_cast<SnippetItem*>( index.internalPointer() );
  Q_ASSERT( item );

  switch ( role ) {
    case NameRole:
      item->setName( value.toString() );
      emit dataChanged( index, index );
      return true;
      break;
    case TextRole:
      item->setText( value.toString() );
      emit dataChanged( index, index );
      return true;
      break;
    case KeySequenceRole:
      item->setKeySequence( value.toString() );
      emit dataChanged( index, index );
      return true;
      break;
    default:
      return false;
      break;
  }

  return false;
}

QVariant SnippetsModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  SnippetItem *item = static_cast<SnippetItem*>( index.internalPointer() );

  switch ( role ) {
    case IsGroupRole:
      return item->isGroup();
      break;
    case NameRole:
      return item->name();
      break;
    case TextRole:
      return item->text();
      break;
    case KeySequenceRole:
      return item->keySequence();
      break;
  }

  return QVariant();
}

Qt::ItemFlags SnippetsModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return 0;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex SnippetsModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  SnippetItem *parentItem;

  if ( !parent.isValid() )
    parentItem = mRootItem;
  else
    parentItem = static_cast<SnippetItem*>( parent.internalPointer() );

  SnippetItem *childItem = parentItem->child( row );
  if (childItem)
    return createIndex( row, column, childItem );
  else
    return QModelIndex();
}

QModelIndex SnippetsModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  SnippetItem *childItem = static_cast<SnippetItem*>( index.internalPointer() );
  SnippetItem *parentItem = childItem->parent();

  if ( parentItem == mRootItem )
    return QModelIndex();

  return createIndex( parentItem->row(), 0, parentItem );
}

int SnippetsModel::rowCount( const QModelIndex &parent ) const
{
  SnippetItem *parentItem;
  if ( parent.column() > 0 )
    return 0;

  if ( !parent.isValid() )
    parentItem = mRootItem;
  else
    parentItem = static_cast<SnippetItem*>( parent.internalPointer() );

  return parentItem->childCount();
}

bool SnippetsModel::insertRows( int row, int count, const QModelIndex &parent )
{
  SnippetItem *parentItem;

  if ( !parent.isValid() )
    parentItem = mRootItem;
  else
    parentItem = static_cast<SnippetItem*>( parent.internalPointer() );

  beginInsertRows( parent, row, row + count - 1 );
  for ( int i = 0; i < count; ++i ) {
    new SnippetItem( !parent.isValid(), parentItem );
  }
  endInsertRows();

  return true;
}

bool SnippetsModel::removeRows( int row, int count, const QModelIndex &parent )
{
  SnippetItem *parentItem;

  if ( !parent.isValid() )
    parentItem = mRootItem;
  else
    parentItem = static_cast<SnippetItem*>( parent.internalPointer() );

  beginRemoveRows( parent, row, row + count - 1 );
  for ( int i = 0; i < count; ++i )
    parentItem->removeChild( parentItem->child( row ) );
  endRemoveRows();

  return true;
}

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

#include "collectionfetchwatcher.h"

#include <AkonadiCore/collectionstatistics.h>
#include <AkonadiCore/entitytreemodel.h>

#include <QtCore/QAbstractItemModel>

using namespace AkonadiFuture;

class CollectionFetchWatcher::Private
{
  public:
    Private( CollectionFetchWatcher *qq, const QModelIndex &index, const QAbstractItemModel *model )
      : q( qq ), mIndex( index ), mModel( model )
    {
    }

    void dataChanged( const QModelIndex &index, const QModelIndex& )
    {
      if ( index != mIndex )
        return;

      if ( mIndex.data( Akonadi::EntityTreeModel::FetchStateRole ).toInt() == Akonadi::EntityTreeModel::IdleState ) {
        q->disconnect( mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                       q, SLOT(dataChanged(QModelIndex,QModelIndex)) );

        emit q->collectionFetched( mIndex );

        q->deleteLater();
      }
    }

    CollectionFetchWatcher *q;
    QPersistentModelIndex mIndex;
    const QAbstractItemModel *mModel;
};

CollectionFetchWatcher::CollectionFetchWatcher( const QModelIndex &index, const QAbstractItemModel *model, QObject *parent )
  : QObject( parent ), d( new Private( this, index, model ) )
{
  Q_ASSERT( d->mIndex.model() == d->mModel ); // make sure we work on the right indexes/model
}

CollectionFetchWatcher::~CollectionFetchWatcher()
{
  delete d;
}

void CollectionFetchWatcher::start()
{
  const Akonadi::Collection collection = d->mIndex.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
  Q_ASSERT( collection.isValid() );

  if ( collection.statistics().count() == 0 ) {
    // no reason to wait, this collection does not contain any items
    emit collectionFetched( d->mIndex );
    deleteLater();
    return;
  }

  // check if the loading has been finished already
  if ( d->mIndex.data( Akonadi::EntityTreeModel::FetchStateRole ).toInt() == Akonadi::EntityTreeModel::IdleState ) {
    emit collectionFetched( d->mIndex );
    deleteLater();
    return;
  }

  // start our work
  connect( d->mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
           this, SLOT(dataChanged(QModelIndex,QModelIndex)) );
}

#include "moc_collectionfetchwatcher.cpp"

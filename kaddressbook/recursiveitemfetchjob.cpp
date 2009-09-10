/*
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "recursiveitemfetchjob.h"

#include <akonadi/collectionfetchjob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>

#include <QtCore/QVariant>

using namespace Akonadi;

class RecursiveItemFetchJob::Private
{
  public:
    Private( const Collection &collection, RecursiveItemFetchJob *parent )
      : mParent( parent ), mCollection( collection ), mFetchCount( 0 )
    {
    }

    void collectionFetchResult( KJob *job )
    {
      if ( job->error() ) {
        mParent->emitResult();
        return;
      }

      const CollectionFetchJob *fetchJob = qobject_cast<CollectionFetchJob*>( job );

      Collection::List collections = fetchJob->collections();
      collections.prepend( mCollection );

      foreach ( const Collection &collection, collections ) {
        ItemFetchJob *itemFetchJob = new ItemFetchJob( collection, mParent );
        itemFetchJob->setFetchScope( mFetchScope );
        mParent->connect( itemFetchJob, SIGNAL( result( KJob* ) ),
                          mParent, SLOT( itemFetchResult( KJob* ) ) );

        mFetchCount++;
      }
    }

    void itemFetchResult( KJob *job )
    {
      if ( !job->error() ) {
        const ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
        mItems << fetchJob->items();
      }

      mFetchCount--;

      if ( mFetchCount == 0 )
        mParent->emitResult();
    }

    RecursiveItemFetchJob *mParent;
    Collection mCollection;
    Item::List mItems;
    ItemFetchScope mFetchScope;

    int mFetchCount;
};

RecursiveItemFetchJob::RecursiveItemFetchJob( const Collection &collection, QObject * parent )
  : KJob( parent ), d( new Private( collection, this ) )
{
}

RecursiveItemFetchJob::~RecursiveItemFetchJob()
{
  delete d;
}

void RecursiveItemFetchJob::setFetchScope( const ItemFetchScope &fetchScope )
{
  d->mFetchScope = fetchScope;
}

ItemFetchScope &RecursiveItemFetchJob::fetchScope()
{
  return d->mFetchScope;
}

void RecursiveItemFetchJob::start()
{
  CollectionFetchJob *job = new CollectionFetchJob( d->mCollection, CollectionFetchJob::Recursive, this );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( collectionFetchResult( KJob* ) ) );
}

Akonadi::Item::List RecursiveItemFetchJob::items() const
{
  return d->mItems;
}

#include "recursiveitemfetchjob.moc"

/*
    Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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


#include "entityupdateadapter.h"

#include <akonadi/session.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionmodifyjob.h>
#include <akonadi/collectiondeletejob.h>
#include <akonadi/collectioncreatejob.h>
#include <akonadi/collectioncopyjob.h>
#include <akonadi/collectionstatisticsjob.h>
#include <akonadi/collectionstatistics.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemmovejob.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/transactionsequence.h>
#include <akonadi/entitydisplayattribute.h>
#include "collectionchildorderattribute.h"

#include <QVariant>
#include <QStringList>

#include <kdebug.h>

using namespace Akonadi;

EntityUpdateAdapter::EntityUpdateAdapter( Session *s, ItemFetchScope scope, QObject *parent, int includeUnsubscribed )
    : QObject( parent ), m_session( s ), m_job_parent( s ), m_itemFetchScope( scope )
{

  m_includeUnsubscribed = ( includeUnsubscribed == IncludeUnsubscribed ) ? true : false;


  m_session->setParent(this);

}

EntityUpdateAdapter::~EntityUpdateAdapter()
{
  delete m_job_parent;
}

void EntityUpdateAdapter::addEntities( Item::List newItems, Collection::List newCollections, Collection parent, int row )
{

}

void EntityUpdateAdapter::beginTransaction()
{
  m_job_parent = new TransactionSequence(m_session);
}

void EntityUpdateAdapter::endTransaction()
{
  m_job_parent = m_session;
}

void EntityUpdateAdapter::fetchCollections( Collection col, CollectionFetchJob::Type type )
{
  CollectionFetchJob *job = new CollectionFetchJob( col, type, m_job_parent );
  job->includeUnsubscribed( m_includeUnsubscribed );
  connect( job, SIGNAL( collectionsReceived( Akonadi::Collection::List ) ),
           this, SIGNAL( collectionsReceived( Akonadi::Collection::List ) ) );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( listJobDone( KJob* ) ) );

}

void EntityUpdateAdapter::fetchItems( Item::List items )
{


}

void EntityUpdateAdapter::fetchItems( Collection parent )
{
  Akonadi::ItemFetchJob *itemJob = new Akonadi::ItemFetchJob( parent, m_job_parent );
  itemJob->setFetchScope( m_itemFetchScope );

  // ### HACK: itemsAdded needs to know which collection items were added to.
  // That is not provided by akonadi, so we attach it in a property.
  itemJob->setProperty( ItemFetchCollectionId(), QVariant( parent.id() ) );

  connect( itemJob, SIGNAL( itemsReceived( Akonadi::Item::List ) ),
           this, SLOT( itemsReceivedFromJob( Akonadi::Item::List ) ) );
  connect( itemJob, SIGNAL( result( KJob* ) ),
           this, SLOT( listJobDone( KJob* ) ) );
}

void EntityUpdateAdapter::itemsReceivedFromJob( Akonadi::Item::List list )
{
  QObject *job = this->sender();
  kDebug() << job;
  if ( job ) {
    Collection::Id colId = job->property( ItemFetchCollectionId() ).value<Collection::Id>();
    kDebug() << colId;
    emit itemsReceived( list, colId );
  }
}

void EntityUpdateAdapter::moveEntities( Item::List movedItems, Collection::List movedCollections, Collection src, Collection dst, int row )
{
  kDebug() << movedItems.size() << movedCollections.size() << src.remoteId() << dst.remoteId();

  // TODO: Use row to update the CollectionChildOrderAttribute correctly.

  TransactionSequence *transaction = new TransactionSequence( m_session );

  foreach( Collection col, movedCollections ) {
    // This is effectively a collectionmovejob.
    col.setParent( dst.id() );
    Akonadi::CollectionModifyJob *modifyJob = new Akonadi::CollectionModifyJob( col, transaction );
//     Akonadi::CollectionModifyJob *modifyJob = new Akonadi::CollectionModifyJob(col, m_job_parent);
  }
  foreach( Item item, movedItems ) {
    // This doesn't have any effect on the resource.
    Akonadi::ItemMoveJob *itemMoveJob = new Akonadi::ItemMoveJob( item, dst, transaction );
//     Akonadi::ItemMoveJob *itemMoveJob = new Akonadi::ItemMoveJob(item, dst, m_job_parent);
  }

  if ( src.hasAttribute<CollectionChildOrderAttribute>() ) {
    // Update the source collection...
    CollectionChildOrderAttribute *srcCcoa = src.attribute<CollectionChildOrderAttribute>();
    QStringList srcl = srcCcoa->orderList();
    kDebug() << srcl;
    foreach( Collection col, movedCollections ) {
      srcl.removeOne( col.remoteId() );
    }
    foreach( Item item, movedItems ) {
      srcl.removeOne( item.remoteId() );
    }
    kDebug() << srcl;
    srcCcoa->setOrderList( srcl );
    src.addAttribute( srcCcoa );

    // ... And the destination collection.
    CollectionChildOrderAttribute *dstCcoa = dst.attribute<CollectionChildOrderAttribute>();
    QStringList dstl = dstCcoa->orderList();
    kDebug() << dstl;
    foreach( Collection col, movedCollections ) {
      dstl.append( col.remoteId() );
    }
    foreach( Item item, movedItems ) {
      dstl.append( item.remoteId() );
    }
    kDebug() << dstl;
    dstCcoa->setOrderList( dstl );
    dst.addAttribute( dstCcoa );

    Akonadi::CollectionModifyJob *srcModifyJob = new Akonadi::CollectionModifyJob( src, transaction );
//     Akonadi::CollectionModifyJob *srcModifyJob = new Akonadi::CollectionModifyJob( src, m_job_parent );
    if ( dst != src ) {
      Akonadi::CollectionModifyJob *dstModifyJob = new Akonadi::CollectionModifyJob( dst, transaction );
//       Akonadi::CollectionModifyJob *dstModifyJob = new Akonadi::CollectionModifyJob( dst, m_job_parent );
    }
  }
}

void EntityUpdateAdapter::removeEntities( Item::List removedItems, Collection::List removedCollections, Collection parent )
{

}

void EntityUpdateAdapter::updateEntities( Collection::List updatedCollections, Item::List updatedItems )
{

}

void EntityUpdateAdapter::updateEntities( Collection::List updatedCollections )
{

}

void EntityUpdateAdapter::updateEntities( Item::List updatedItems )
{
  foreach( Item item, updatedItems ) {
    Akonadi::ItemModifyJob *itemModifyJob = new Akonadi::ItemModifyJob( item, m_job_parent );
  }
}

void EntityUpdateAdapter::listJobDone( KJob *job )
{
  if ( job->error() ) {
    kWarning( 5250 ) << "Job error: " << job->errorString() << endl;
  }
}

void EntityUpdateAdapter::updateJobDone( KJob *job )
{
  if ( job->error() ) {
    // TODO: handle job errors
    kWarning( 5250 ) << "Job error:" << job->errorString();
  } else {
    // TODO: Is this trying to do the job of collectionstatisticschanged?
//     CollectionStatisticsJob *csjob = static_cast<CollectionStatisticsJob*>( job );
//     Collection result = csjob->collection();
//     collectionStatisticsChanged( result.id(), csjob->statistics() );
  }
}

// #include "entityupdateadapter.moc"

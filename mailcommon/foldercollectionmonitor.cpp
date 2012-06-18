/*
  Copyright (c) 2009, 2010, 2011 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "foldercollectionmonitor.h"
#include "mailutil.h"
#include "foldercollection.h"
#include "expirecollectionattribute.h"

#include <Akonadi/ChangeRecorder>
#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/CollectionModel>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/Item>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/KMime/MessageParts>

#include <KMime/KMimeMessage>

namespace MailCommon {

FolderCollectionMonitor::FolderCollectionMonitor( QObject *parent )
  : QObject( parent )
{
  // monitor collection changes
  mMonitor = new Akonadi::ChangeRecorder( this );
  mMonitor->setCollectionMonitored( Akonadi::Collection::root() );
  mMonitor->fetchCollectionStatistics( true );
  mMonitor->collectionFetchScope().setIncludeStatistics( true );
  mMonitor->fetchCollection( true );
  mMonitor->setAllMonitored( true );
  mMonitor->setMimeTypeMonitored( KMime::Message::mimeType() );
  mMonitor->setMimeTypeMonitored(QString::fromLatin1( "inode/directory" ));
  mMonitor->setResourceMonitored( "akonadi_search_resource", true );
#ifndef KDEPIM_NO_NEPOMUK
  mMonitor->setResourceMonitored( "akonadi_nepomuktag_resource", true );
#endif
  mMonitor->itemFetchScope().fetchPayloadPart( Akonadi::MessagePart::Envelope );
}

FolderCollectionMonitor::~FolderCollectionMonitor()
{
}

Akonadi::ChangeRecorder *FolderCollectionMonitor::monitor() const
{
  return mMonitor;
}

void FolderCollectionMonitor::expireAllFolders( bool immediate,
                                                QAbstractItemModel *collectionModel )
{
  if ( collectionModel ) {
    expireAllCollection( collectionModel, immediate );
  }
}

void FolderCollectionMonitor::expireAllCollection( const QAbstractItemModel *model,
                                                   bool immediate,
                                                   const QModelIndex &parentIndex )
{
  const int rowCount = model->rowCount( parentIndex );
  for ( int row = 0; row < rowCount; ++row ) {
    const QModelIndex index = model->index( row, 0, parentIndex );
    const Akonadi::Collection collection =
      model->data(
        index, Akonadi::CollectionModel::CollectionRole ).value<Akonadi::Collection>();

    if ( !collection.isValid() || Util::isVirtualCollection( collection ) ) {
      continue;
    }

    bool mustDeleteExpirationAttribute = false;
    MailCommon::ExpireCollectionAttribute *attr =
      MailCommon::ExpireCollectionAttribute::expirationCollectionAttribute(
        collection, mustDeleteExpirationAttribute );

    if ( attr->isAutoExpire() ) {
       MailCommon::Util::expireOldMessages( collection, immediate );
    }

    if ( model->rowCount( index ) > 0 ) {
      expireAllCollection( model, immediate, index );
    }

    if ( mustDeleteExpirationAttribute ) {
      delete attr;
    }
  }
}

void FolderCollectionMonitor::expunge( const Akonadi::Collection & col, bool sync )
{
  if ( col.isValid() ) {
    Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob( col, this );
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotDeleteJob(KJob*)) );
    if ( sync ) {
      job->exec();
    }
  } else {
    kDebug() << " Try to expunge an invalid collection :" << col;
  }
}

void FolderCollectionMonitor::slotDeleteJob( KJob *job )
{
  Util::showJobErrorMessage( job );
}

}

#include "foldercollectionmonitor.moc"

#include "collectiondumpjob.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QHash>

using namespace Akonadi;

CollectionDumpJob::CollectionDumpJob( const Collection &collection, QDir dumpPath, QObject *parent ) :
    KJob( parent ), m_collection( collection ), m_path( dumpPath ), m_subjobs( 0 )
{
}

void CollectionDumpJob::start()
{
  if ( !m_path.exists() ) {
    setError( 1 );
    setErrorText( QString( "Path %1 does not exist" ).arg(m_path.absolutePath() ) );
    emitResult();
    return;
  }

  // record collection attributes
  m_config = KSharedConfig::openConfig( m_path.absoluteFilePath( "collectioninfo" ), KSharedConfig::SimpleConfig );
  KConfigGroup configGroup( m_config, "General" );
  configGroup.writeEntry( "name", m_collection.name() );
  m_config->sync();

  dumpItems();

  // dump subcollections
  CollectionFetchJob *job = new CollectionFetchJob( m_collection, CollectionFetchJob::FirstLevel, this );
  job->fetchScope().setResource( m_collection.resource() );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( fetchResult(KJob*) ) );
  job->start();
}

void CollectionDumpJob::fetchResult( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    emitResult();
    return;
  }

  const CollectionFetchJob *fetchJob = static_cast< CollectionFetchJob* >( job );
  m_subjobs = fetchJob->collections().size();
  if ( m_subjobs > 0 ) {
    foreach ( const Collection &collection, fetchJob->collections() ) {
      QString subName = QString::number( collection.id() );
      m_path.mkdir( subName ); // will be checked later...
      QDir dir( QString( "%1/%2" ).arg( m_path.absolutePath() ).arg( subName ) );
      CollectionDumpJob *dumpJob = new CollectionDumpJob( collection, dir, this );
      connect( dumpJob, SIGNAL( result( KJob* ) ), this, SLOT( dumpResult( KJob* ) ) );
      dumpJob->start();
    }
  }
  else {
    emitResult();
  }
}

void CollectionDumpJob::dumpResult( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    emitResult();
    return;
  }

  if ( --m_subjobs == 0 )
    emitResult();
}

void CollectionDumpJob::dumpItems()
{
  ItemFetchJob *job = new ItemFetchJob( m_collection, this );
  job->fetchScope().fetchFullPayload();
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( itemFetchResult( KJob* ) ) );
  job->start();
}

void CollectionDumpJob::itemFetchResult( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    emitResult();
    return;
  }

  const ItemFetchJob *fetchJob = static_cast< ItemFetchJob* >( job );
  KConfigGroup itemsGroup( m_config, "Items" );
  foreach ( const Item &item, fetchJob->items() ) {
    KConfigGroup itemGroup( &itemsGroup, QString::number( item.id() ) );
    itemGroup.writeEntry( "flags", item.flags().toList() );
    itemGroup.writeEntry( "mimeType", item.mimeType() );
    itemGroup.writeEntry( "modificationTime", item.modificationTime() );
    itemGroup.writeEntry( "payloadData", item.payloadData() );
  }
  m_config->sync();
}

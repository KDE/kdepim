#include "collectiondumpjob.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>

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
  }

  // record collection attributes
  KConfig config( m_path.absolutePath() + "/" + "collectioninfo" );
  KConfigGroup configGroup( &config, "General" );
  configGroup.writeEntry( "name", m_collection.name() );
  configGroup.writeEntry( "contentMimeTypes", m_collection.contentMimeTypes() );
  config.sync();

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
  }

  if ( --m_subjobs == 0 )
    emitResult();
}

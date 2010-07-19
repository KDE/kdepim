#include "resourcedumpjob.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/agenttype.h>
#include "collectiondumpjob.h"

using namespace Akonadi;

ResourceDumpJob::ResourceDumpJob( const AgentInstance &instance, const QDir &dumpPath, QObject *parent) :
    KJob( parent ), m_instance( instance ), m_path( dumpPath ), m_collectionsNo( 0 )
{
}

void ResourceDumpJob::start()
{
  if ( !m_path.exists() ) {
    setError( 1 );
    setErrorText( QString( "Directory %1 does not exist" ).arg(m_path.absolutePath() ) );
    emitResult();
  }

  // record resource data
  KConfig config( QString( "%1/%2" ).arg( m_path.absolutePath() ).arg( "resurceinfo" ) );
  KConfigGroup configGroup( &config, "General" );
  configGroup.writeEntry( "type", m_instance.type().identifier() );
  configGroup.writeEntry( "name", m_instance.name() );
  config.sync();

  // copy resource configuration file if there is one
  KStandardDirs stdDirs;
  QString configPath = stdDirs.findResource( "config", QString( "%1rc" ).arg( m_instance.identifier() ) );
  if ( !configPath.isEmpty() ) {
    QFile file( configPath );
    QString configDest = QString( "%1/%2" ).arg( m_path.absolutePath() ).arg( "resourcerc" );
    if ( !file.copy( configDest ) ) {
      setError( 2 );
      setErrorText( QString( "Unable to copy %1 to %2" ).arg( file.fileName() ).arg( configDest ) );
      emitResult();
    }
  }

  // dump collections
  CollectionFetchJob *job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::FirstLevel, this);
  job->fetchScope().setResource( m_instance.identifier() );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( fetchResult( KJob* ) ) );
  job->start();
}

void ResourceDumpJob::fetchResult( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    emitResult();
  }

  // dump all found collections, if any
  const CollectionFetchJob *fetchJob = static_cast< CollectionFetchJob* >( job );
  m_collectionsNo = fetchJob->collections().size();
  if ( m_collectionsNo > 0 ) {
    foreach ( const Collection& collection, fetchJob->collections() ) {
      QString name = QString::number( collection.id() );
      m_path.mkdir( name ); // will be checked later
      QDir dir( QString( "%1/%2" ).arg( m_path.absolutePath() ).arg( name ) );
      CollectionDumpJob *dumpJob = new CollectionDumpJob( collection, dir, this );
      connect( dumpJob, SIGNAL( result( KJob* ) ), this, SLOT( dumpResult( KJob* ) ) );
      dumpJob->start();
    }
  }
  else {
    emitResult();
  }
}

void ResourceDumpJob::dumpResult( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    emitResult();
  }

  if ( --m_collectionsNo == 0 )
    emitResult();
}

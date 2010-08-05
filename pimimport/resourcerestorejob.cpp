#include "resourcerestorejob.h"

#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <QtCore/QFile>
#include <QtCore/QTimer>

using namespace Akonadi;

ResourceRestoreJob::ResourceRestoreJob( const QDir &path, QObject *parent)  :
    KJob( parent ), m_path( path )
{
}

void ResourceRestoreJob::start()
{
  // check config file
  QDir configFile( m_path.absoluteFilePath( "resourceinfo" ) );
  if ( !configFile.exists() ) {
    setError( 1 );
    setErrorText( QString( "File %1 does not exist" ).arg( configFile.absolutePath() ) );
  }

  KConfig config( configFile.absolutePath() );
  KConfigGroup configGroup( &config, "General" );
  QString type = configGroup.readEntry( "type", QString() );

  AgentInstanceCreateJob *job = new AgentInstanceCreateJob( type, this );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( createResult( KJob* ) ) );
  job->start();
}

void ResourceRestoreJob::createResult( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    emitResult();
  }

  const AgentInstanceCreateJob *createJob = static_cast< AgentInstanceCreateJob* >( job );
  m_instance = createJob->instance();

  // copy resource's rc file if there is one
  QString rcPath = m_path.absoluteFilePath( "resourcerc" );
  QFile rcFile( rcPath );
  if ( rcFile.exists() ) {
    KStandardDirs dirs;
    QString configDir = dirs.saveLocation( "config" );
    QString dest = QString( "%1/%2%3" ).arg( configDir ).arg( m_instance.identifier() ).arg( "rc" );
    bool result = rcFile.copy( dest );
    if ( !result ) {
      setError( 1 );
      setErrorText( QString( "Unable to copy file %1 to %2" ).arg( rcPath ).arg( dest ) );
      emitResult();
    }
  }

  // reconfigure resource and restore main collection
  m_instance.reconfigure();
  restoreMainCollection();
}

void ResourceRestoreJob::restoreMainCollection()
{
  CollectionFetchJob *job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::FirstLevel, this );
  job->fetchScope().setResource( m_instance.identifier() );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( mainFetchResult( KJob* ) ) );
  job->start();
}

void ResourceRestoreJob::mainFetchResult( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    emitResult();
  }

  const CollectionFetchJob *fetchJob = static_cast< CollectionFetchJob* >( job );
  if ( fetchJob->collections().isEmpty() ) {
    QTimer::singleShot( 50, this, SLOT( restoreMainCollection() ) );
    return;
  }
  else {
    foreach ( const Collection &collection, fetchJob->collections() )
      kError() << collection.id() << collection.name();
  }
}

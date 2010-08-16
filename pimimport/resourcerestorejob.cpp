#include "resourcerestorejob.h"

#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/collectiondeletejob.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/session.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <QtCore/QFile>
#include <QtCore/QTimer>

#include "collectionrestorejob.h"

using namespace Akonadi;

ResourceRestoreJob::ResourceRestoreJob( const QDir &path, QObject *parent)  :
    KJob( parent ), m_path( path )
{
}

void ResourceRestoreJob::start()
{
  // check config file
  if ( !m_path.exists( "resourceinfo" ) ) {
    setError( 1 );
    setErrorText( QString( "File %1 does not exist" ).arg( m_path.absoluteFilePath( "resourceinfo" ) ) );
    emitResult();
    return;
  }

  KConfig config( m_path.absoluteFilePath( "resourceinfo" ) );
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
    kError() << job->errorString();
    emitResult();
    return;
  }

  const AgentInstanceCreateJob *createJob = static_cast< AgentInstanceCreateJob* >( job );
  m_instance = createJob->instance();

  // dump directory should have only *one* subdirectory containing main collection
  QStringList subcollections = m_path.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
  Session *session = new Session( m_instance.identifier().toLocal8Bit(), this );
  CollectionRestoreJob *restoreJob = new CollectionRestoreJob( Collection::root(),
                                                               m_path.absoluteFilePath( subcollections[0] ),
                                                               session );
  connect( restoreJob, SIGNAL( result( KJob* ) ), this, SLOT( restoreResult( KJob* ) ) );
  restoreJob->start();
}

void ResourceRestoreJob::restoreResult( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    kError() << job->errorString();
    emitResult();
    return;
  }

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
      return;
    }
  }

  // restart resource and emit result
  kError() << "Restored resource: " << m_instance.identifier();
  m_instance.restart();
  emitResult();
}

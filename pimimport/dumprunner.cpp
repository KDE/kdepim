#include "dumprunner.h"

#include <akonadi/agentmanager.h>

#include "resourcedumpjob.h"
#include "resourcerestorejob.h"

DumpRunner::DumpRunner( const QDir &path, QObject *parent ) :
    QObject( parent ), m_remainingResources( 0 ), m_path( path )
{
}

void DumpRunner::dump()
{
  QList< ResourceDumpJob* > jobs;
  Akonadi::AgentManager *mgr = Akonadi::AgentManager::self();
  foreach ( const Akonadi::AgentInstance &instance, mgr->instances() ) {
    // check if instance is a resource
    if ( !instance.type().capabilities().contains( "Resource" ) )
      continue;

    // setup directory for resource
    QString resPath = m_path.absoluteFilePath( instance.identifier() );
    QDir resDir;
    resDir.mkpath( resPath );
    resDir.setPath( resPath );

    ResourceDumpJob *job = new ResourceDumpJob( instance, resDir, this );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( resourceFinished() ) );
    jobs.append( job );
  }

  m_remainingResources = jobs.size();
  if ( m_remainingResources ) {
    foreach ( KJob *job, jobs )
      job->start();
  }
  else {
    emit finished();
  }
}

void DumpRunner::restore()
{
  QList< ResourceRestoreJob* > jobs;
  foreach ( const QString &resource, m_path.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) ) {
    ResourceRestoreJob *job = new ResourceRestoreJob( m_path.absoluteFilePath( resource ), this );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( resourceFinished() ) );
    jobs.append( job );
  }

  m_remainingResources = jobs.size();
  if ( m_remainingResources ) {
    foreach ( ResourceRestoreJob *job, jobs )
      job->start();
  }
  else {
    emit finished();
  }
}

void DumpRunner::resourceFinished()
{
  if ( --m_remainingResources == 0)
    emit finished();
}

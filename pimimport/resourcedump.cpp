#include "resourcedump.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <akonadi/agenttype.h>
#include <akonadi/agentinstancecreatejob.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kconfiggroup.h>

ResourceDump::ResourceDump( QString path, QObject *parent ) :
    AbstractDump( path, parent ), m_instance()
{
  m_name = path.split("/").last();
}

ResourceDump::ResourceDump( QString path, Akonadi::AgentInstance instance, QObject *parent ) :
    AbstractDump( path, parent ), m_instance( instance )
{
  m_name = path.split("/").last();
}

Akonadi::AgentInstance ResourceDump::instance() const
{
  return m_instance;
}

void ResourceDump::dump()
{
  // check if m_instance is a valid instance
  if ( !m_instance.isValid() ) {
    kError() << "ResourceDump::dump(): m_instance is invalid";
    return ;
  }

  // copy resource configuration file if there is one
  KStandardDirs stdDirs;
  QString configPath = stdDirs.findResource( "config", QString( "%1rc" ).arg( m_instance.identifier() ) );
  if ( !configPath.isEmpty() ) {
    QFile file( configPath );
    QString configDest = QString( "%1/%2" ).arg( path() ).arg( "resourcerc" );
    if ( !file.copy( configDest ) ) {
      kError() << "ResourceDump::dump(): unable to copy file " << file.fileName()
          << " to " << configDest;
      return;
    }
  }

  // write resourece type to info file
  KConfig config( QString( "%1/%2" ).arg( path() ).arg( "resourceinfo" ), KConfig::SimpleConfig );
  KConfigGroup cfgGroup( &config, "General" );
  cfgGroup.writeEntry( "type", m_instance.type().identifier() );
  config.sync();

  emit finished();
}

void ResourceDump::restore()
{
  // get resource type from info file
  KConfig config( QString( "%1/%2" ).arg( path() ).arg( "resourceinfo" ), KConfig::SimpleConfig );
  KConfigGroup cfgGroup( &config, "General" );
  QString type = cfgGroup.readEntry( "type", QString() );

  // create resource
  Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob( type, this);
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( resourceCreated( KJob* ) ) );
  job->start();
}

void ResourceDump::resourceCreated( KJob *job )
{
  if ( job->error() ) {
    kError() << "restoring " << m_name << " failed: " << job->errorText();
    return;
  }

  m_instance = static_cast< Akonadi::AgentInstanceCreateJob* >( job )->instance();
  restoreResource();
}

void ResourceDump::restoreResource()
{
  // copy resource's config file if there is one
  QFile cfgFile( QString( "%1/%2" ).arg( path() ).arg( "resourcerc" ) );
  if ( cfgFile.exists() ) {
    KStandardDirs dirs;
    QString configDir = dirs.saveLocation( "config" );
    QString dest = QString( "%1/%2%3" ).arg( configDir ).arg(m_instance.identifier() ).arg( "rc" );
    bool result = cfgFile.copy( dest );
    if ( !result )
      kError() << "ResourceDump::restore(): copying file failed: " << cfgFile.fileName() << " to "
          << dest;
    m_instance.reconfigure();
  }

  kError() << "restored " << m_name;

  emit finished();
}

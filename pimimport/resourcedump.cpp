#include "resourcedump.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <akonadi/agenttype.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kconfiggroup.h>

ResourceDump::ResourceDump( QString path, QObject *parent ) :
    AbstractDump( path, parent ), m_instance()
{
}

ResourceDump::ResourceDump( QString path, Akonadi::AgentInstance instance, QObject *parent ) :
    AbstractDump( path, parent ), m_instance( instance )
{
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

}

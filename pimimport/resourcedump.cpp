#include "resourcedump.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <akonadi/agenttype.h>
#include <kdebug.h>
#include <kstandarddirs.h>

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

  QFile resTypeFile( QString( "%1/%2" ).arg( path() ).arg( "resourcetype" ) );
  if ( !resTypeFile.open(QIODevice::WriteOnly) ) {
    kError() << "ResourceDump::dump(): unable to open file for writing: " << resTypeFile.fileName();
    return;
  }
  QTextStream resTypeStream( &resTypeFile );
  resTypeStream << m_instance.type().identifier();
  resTypeStream.flush();
  resTypeFile.close();

  emit finished();
}

void ResourceDump::restore()
{

}

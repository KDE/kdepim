#include "akonadidumper.h"

#include <QtCore/QDir>
#include <akonadi/agentinstance.h>
#include <akonadi/agentmanager.h>
#include <akonadi/control.h>
#include <kdebug.h>
#include "resourcedump.h"

using namespace Akonadi;

AkonadiDump::AkonadiDump( const QDir &path, QObject *parent ) :
    AbstractDump( path, parent ), m_remainingResources( 0 )
{
}

void AkonadiDump::dump()
{
  initializeResources( action_dump );
  foreach ( AbstractDump *dump, m_resources )
    dump->dump();

  emit finished();
}

void AkonadiDump::restore()
{
  initializeResources( action_restore );
  if ( m_resources.size() == 0 ) {
    emit finished();
    return;
  }
  else {
    foreach ( AbstractDump *dump, m_resources )
      dump->restore();
  }
}

void AkonadiDump::initializeResources( AbstractDump::Action action )
{
  if ( action == AbstractDump::action_dump ) {
    AgentManager *mgr = AgentManager::self();
    foreach ( const AgentInstance &instance, mgr->instances() ) {
      // check if instance is a resource
      if ( !instance.type().capabilities().contains( "Resource" ) )
        continue;

      // setup directory for resource
      QString resPath = QString( "%1/%2" ).arg( path().absolutePath() ).arg( instance.identifier() );
      QDir resDir;
      resDir.mkpath( resPath );
      resDir.setPath( resPath );

      ResourceDump *res = new ResourceDump( resDir.absolutePath(), instance, this );
      m_resources.append( res );
    }
  }
  else if ( action == AbstractDump::action_restore ) {
    QDir dir( path() );
    foreach ( const QString &resource, dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) ) {
      ResourceDump *res = new ResourceDump( QString("%1/%2").arg( dir.absolutePath() ).arg( resource ),
                                            this );
      connect( res, SIGNAL( finished() ), SLOT( resourceRestored() ) );
      m_resources.append( res );
    }

  }
}

void AkonadiDump::resourceRestored()
{
  if ( --m_remainingResources == 0)
    emit finished();
}

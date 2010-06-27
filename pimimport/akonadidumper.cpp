#include "akonadidumper.h"

#include <QtCore/QDir>
#include <akonadi/agentinstance.h>
#include <akonadi/agentmanager.h>
#include <akonadi/control.h>
#include <kdebug.h>
#include "resourcedump.h"

using namespace Akonadi;

AkonadiDump::AkonadiDump( QString path, QObject *parent ) :
    AbstractDump( path, parent )
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
}

void AkonadiDump::initializeResources( AbstractDump::Action action )
{
  if ( action == AbstractDump::action_dump ) {
    AgentManager *mgr = AgentManager::self();
    foreach ( AgentInstance instance, mgr->instances() ) {
      // setup directory for resource
      QString resPath = QString( "%1/%2" ).arg( path() ).arg( instance.identifier() );
      QDir resDir;
      resDir.mkpath( resPath );
      resDir.setPath( resPath );

      ResourceDump *res = new ResourceDump( resDir.absolutePath(), instance, this );
      m_resources.append( res );
    }
  }
}

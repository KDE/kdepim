/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "agentprogressmonitor.h"

#include <Akonadi/AgentManager>

#include <KDebug>

#include <QWeakPointer>

using namespace Akonadi;
using namespace KPIM;

class AgentProgressMonitor::Private
{
  public:
    Private( AgentProgressMonitor *qq, const AgentInstance &agnt, ProgressItem *itm )
      : q( qq )
      , agent( agnt )
      , item ( itm )
    {
    }

    // slots:
    void abort();
    void instanceProgressChanged( const AgentInstance &instance );
    void instanceStatusChanged( const AgentInstance &instance );

    AgentProgressMonitor *const q;
    AgentInstance agent;
    QWeakPointer<ProgressItem> const item;
};

void AgentProgressMonitor::Private::abort()
{
  agent.abortCurrentTask();
}

void AgentProgressMonitor::Private::instanceProgressChanged( const AgentInstance &instance )
{
  if ( !item.data() )
    return;

  if ( agent == instance ) {
    agent = instance;
    if ( agent.progress() >= 0 ) {
      item.data()->setProgress( agent.progress() );
    }
  }
}

void AgentProgressMonitor::Private::instanceStatusChanged( const AgentInstance &instance )
{
  if ( !item.data() )
    return;

  if ( agent == instance ) {
    agent = instance;
    item.data()->setStatus( agent.statusMessage() );
    switch ( agent.status() ) {
      case AgentInstance::Idle:
        if( item.data() )	      
          item.data()->setComplete();
        break;
      case AgentInstance::Running:
        break;
      case AgentInstance::Broken: 
        item.data()->disconnect( q ); // avoid abort call
        item.data()->cancel();
	if( item.data() )
           item.data()->setComplete();
        break;
      default:
        Q_ASSERT( false );
    }
  }
}



AgentProgressMonitor::AgentProgressMonitor( const AgentInstance &agent,
    ProgressItem *item )
  : QObject( item )
  , d( new Private( this, agent, item ) )
{
  connect( AgentManager::self(), SIGNAL(instanceProgressChanged(Akonadi::AgentInstance)),
      this, SLOT(instanceProgressChanged(Akonadi::AgentInstance)) );
  connect( AgentManager::self(), SIGNAL(instanceStatusChanged(Akonadi::AgentInstance)),
      this, SLOT(instanceStatusChanged(Akonadi::AgentInstance)) );
  // TODO connect to instanceError, instanceNameChanged, instanceWarning, instanceOnline,
  // instanceRemoved?  and do what?

  connect( item, SIGNAL(progressItemCanceled(KPIM::ProgressItem*)),
      this, SLOT(abort()) );

  // TODO what about usesCrypto?

  // TODO handle offline case
}

AgentProgressMonitor::~AgentProgressMonitor()
{
  delete d;
}

#include "agentprogressmonitor.moc"

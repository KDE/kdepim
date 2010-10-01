/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include "agentstatusmonitor.h"

#include <Akonadi/AgentManager>
#include <KDebug>

using namespace Akonadi;

Q_DECLARE_METATYPE( AgentStatusMonitor::AgentStatus )

AgentStatusMonitor::AgentStatusMonitor(QObject* parent): QObject(parent), m_status( Offline )
{
  qRegisterMetaType<AgentStatusMonitor::AgentStatus>();
  connect( AgentManager::self(), SIGNAL(instanceAdded(Akonadi::AgentInstance)), SLOT(updateStatus()) );
  connect( AgentManager::self(), SIGNAL(instanceRemoved(Akonadi::AgentInstance)), SLOT(updateStatus()) );
  connect( AgentManager::self(), SIGNAL(instanceOnline(Akonadi::AgentInstance,bool)), SLOT(updateStatus()) );
  connect( AgentManager::self(), SIGNAL(instanceStatusChanged(Akonadi::AgentInstance)), SLOT(updateStatus()) );
  updateStatus();
}

AgentStatusMonitor::AgentStatus AgentStatusMonitor::status() const
{
  return m_status;
}

void AgentStatusMonitor::updateStatus()
{
  AgentStatus oldStatus = m_status;
  m_status = Offline;
  // TODO: apply mimetype filtering
  foreach ( const AgentInstance &instance, AgentManager::self()->instances() ) {
    if ( instance.isOnline() )
      m_status |= Online;
    if ( instance.type().identifier() == QLatin1String( "akonadi_maildispatcher_agent" ) ) {
      if ( instance.status() == AgentInstance::Running )
        m_status |= Sending;
    } else {
      if ( instance.status() == AgentInstance::Running )
        m_status |= Receiving;
    }
  }

  kDebug() << m_status << oldStatus;
  if ( m_status != oldStatus )
    emit statusChanged();
}

#include "agentstatusmonitor.moc"

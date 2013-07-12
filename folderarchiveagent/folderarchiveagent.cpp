/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "folderarchiveagent.h"

#include "folderarchiveagentadaptor.h"
#include "folderarchiveagentsettings.h"

#include <akonadi/dbusconnectionpool.h>


FolderArchiveAgent::FolderArchiveAgent(const QString &id)
    : Akonadi::AgentBase( id )
{
    new FolderArchiveAgentAdaptor( this );
    Akonadi::DBusConnectionPool::threadConnection().registerObject( QLatin1String( "/FolderArchiveAgent" ), this, QDBusConnection::ExportAdaptors );
    Akonadi::DBusConnectionPool::threadConnection().registerService( QLatin1String( "org.freedesktop.Akonadi.FolderArchiveAgent" ) );
}

FolderArchiveAgent::~FolderArchiveAgent()
{
}

void FolderArchiveAgent::showConfigureDialog(qlonglong windowId)
{
    //TODO
}

void FolderArchiveAgent::setEnableAgent(bool b)
{
    FolderArchiveAgentSettings::setEnabled(b);
    FolderArchiveAgentSettings::self()->writeConfig();
}

bool FolderArchiveAgent::enabledAgent() const
{
    return FolderArchiveAgentSettings::enabled();
}


AKONADI_AGENT_MAIN( FolderArchiveAgent )

#include "folderarchiveagent.moc"

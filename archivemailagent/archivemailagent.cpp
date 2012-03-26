/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "archivemailagent.h"
#include "archivemailkernel.h"

#include <mailcommon/mailkernel.h>
#include <akonadi/dbusconnectionpool.h>


ArchiveMailAgent::ArchiveMailAgent( const QString &id )
  : Akonadi::AgentBase( id )
{
  ArchiveMailKernel *kernel = new ArchiveMailKernel( this );
  CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
  CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget

  Akonadi::DBusConnectionPool::threadConnection().registerObject( QLatin1String( "/ArchiveMailAgent" ), this, QDBusConnection::ExportAdaptors );
  Akonadi::DBusConnectionPool::threadConnection().registerService( QLatin1String( "org.freedesktop.Akonadi.ArchiveMailAgent" ) );

}

ArchiveMailAgent::~ArchiveMailAgent()
{
}

AKONADI_AGENT_MAIN( ArchiveMailAgent )

#include "archivemailagent.moc"

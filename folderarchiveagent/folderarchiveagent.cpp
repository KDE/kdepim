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
#include "folderarchiveconfiguredialog.h"

#include "folderarchiveagentadaptor.h"
#include "folderarchiveagentsettings.h"
#include "folderarchivemanager.h"

#include <KWindowSystem>

#include <akonadi/dbusconnectionpool.h>

#include <QPointer>

FolderArchiveAgent::FolderArchiveAgent(const QString &id)
    : Akonadi::AgentBase( id )
{
    mFolderArchiveManager = new FolderArchiveManager(this);
    new FolderArchiveAgentAdaptor( this );
    Akonadi::DBusConnectionPool::threadConnection().registerObject( QLatin1String( "/FolderArchiveAgent" ), this, QDBusConnection::ExportAdaptors );
    Akonadi::DBusConnectionPool::threadConnection().registerService( QLatin1String( "org.freedesktop.Akonadi.FolderArchiveAgent" ) );

    mFolderArchiveManager->load();
}

FolderArchiveAgent::~FolderArchiveAgent()
{
}

void FolderArchiveAgent::archiveItems( const QList<qint64> &itemIds )
{
    //TODO
}

void FolderArchiveAgent::showConfigureDialog(qlonglong windowId)
{
    QPointer<FolderArchiveConfigureDialog> dialog = new FolderArchiveConfigureDialog();
    if (windowId) {
#ifndef Q_WS_WIN
        KWindowSystem::setMainWindow( dialog, windowId );
#else
        KWindowSystem::setMainWindow( dialog, (HWND)windowId );
#endif
    }
    if (dialog->exec()) {
        mFolderArchiveManager->load();
    }
    delete dialog;
}

void FolderArchiveAgent::configure( WId windowId )
{
    showConfigureDialog((qulonglong)windowId);
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

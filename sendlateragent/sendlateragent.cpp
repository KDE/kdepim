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

#include "sendlateragent.h"
#include "sendlatermanager.h"
#include "sendlaterconfiguredialog.h"
#include "sendlateragentadaptor.h"

#include <akonadi/dbusconnectionpool.h>

#include <KWindowSystem>
#include <KLocale>

#include <QPointer>

SendLaterAgent::SendLaterAgent(const QString &id)
    : Akonadi::AgentBase( id )
{
    mManager = new SendLaterManager(this);
    KGlobal::locale()->insertCatalog( QLatin1String("akonadi_sendlater_agent") );
    new SendLaterAgentAdaptor( this );
    Akonadi::DBusConnectionPool::threadConnection().registerObject( QLatin1String( "/SendLaterAgent" ), this, QDBusConnection::ExportAdaptors );
    Akonadi::DBusConnectionPool::threadConnection().registerService( QLatin1String( "org.freedesktop.Akonadi.SendLaterAgent" ) );
    mManager->load();
}

SendLaterAgent::~SendLaterAgent()
{
}

void SendLaterAgent::configure( WId windowId )
{
    showConfigureDialog(windowId);
}

void SendLaterAgent::showConfigureDialog(qlonglong windowId)
{
    QPointer<SendLaterConfigureDialog> dialog = new SendLaterConfigureDialog();
    if (windowId) {
#ifndef Q_WS_WIN
        KWindowSystem::setMainWindow( dialog, windowId );
#else
        KWindowSystem::setMainWindow( dialog, (HWND)windowId );
#endif
    }
    if (dialog->exec()) {
        mManager->load();
    }
    delete dialog;
}


AKONADI_AGENT_MAIN( SendLaterAgent )

#include "sendlateragent.moc"

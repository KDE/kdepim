/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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


#include "followupreminderagent.h"
//#include "followupreminderadaptor.h"
//#include "followupreminderconfiguredialog.h"
//#include "followupreminderagentsettings.h"
#include <KWindowSystem>
#include <KLocale>

#include <akonadi/dbusconnectionpool.h>

#include <QPointer>
#include <QDebug>

//#define DEBUG_FOLLOWUPREMINDERAGENT 1

FollowUpReminderAgent::FollowUpReminderAgent(const QString &id)
    : Akonadi::AgentBase( id )
{
    KGlobal::locale()->insertCatalog( QLatin1String("akonadi_followupreminder_agent") );
    //new FollowUpReminderAgentAdaptor(this);
    //Akonadi::DBusConnectionPool::threadConnection().registerObject( QLatin1String( "/FollowUpReminder" ), this, QDBusConnection::ExportAdaptors );
    //Akonadi::DBusConnectionPool::threadConnection().registerService( QLatin1String( "org.freedesktop.Akonadi.FollowUpReminder" ) );
    //if (FollowUpReminderAgentSettings::enabled()) {
#ifdef DEBUG_FOLLOWUPREMINDERAGENT
        //QTimer::singleShot(1000, mManager, SLOT(load()));
#else
        //QTimer::singleShot(1000*60*4, mManager, SLOT(load()));
#endif
    //}
}

FollowUpReminderAgent::~FollowUpReminderAgent()
{
}

void FollowUpReminderAgent::setEnableAgent(bool b)
{
    //FollowUpReminderAgentSettings::self()->setEnabled(b);
}

bool FollowUpReminderAgent::enabledAgent() const
{
    //return FollowUpReminderAgentSettings::self()->enabled();
    return false;
}

void FollowUpReminderAgent::showConfigureDialog(qlonglong windowId)
{
    //TODO
}

void FollowUpReminderAgent::configure( WId windowId )
{
    //showConfigureDialog((qulonglong)windowId);
}


AKONADI_AGENT_MAIN( FollowUpReminderAgent )

#include "followupreminderagent.moc"

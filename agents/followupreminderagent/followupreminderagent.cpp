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
#include "followupremindermanager.h"
#include "followupreminderadaptor.h"
#include "followupreminderinfodialog.h"
#include "followupreminderagentsettings.h"
#include <KWindowSystem>
#include <KMime/Message>

#include <AkonadiCore/ChangeRecorder>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/dbusconnectionpool.h>

#include <QPointer>
#include <QDebug>
#include <QTimer>

FollowUpReminderAgent::FollowUpReminderAgent(const QString &id)
    : Akonadi::AgentBase( id )
{
    new FollowUpReminderAgentAdaptor(this);
    Akonadi::DBusConnectionPool::threadConnection().registerObject( QLatin1String( "/FollowUpReminder" ), this, QDBusConnection::ExportAdaptors );
    Akonadi::DBusConnectionPool::threadConnection().registerService( QLatin1String( "org.freedesktop.Akonadi.FollowUpReminder" ) );
    mManager = new FollowUpReminderManager(this);
    changeRecorder()->itemFetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
    changeRecorder()->itemFetchScope().setCacheOnly(true);
    changeRecorder()->fetchCollection( true );
    if (FollowUpReminderAgentSettings::enabled()) {
        mManager->load();
    }
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(reload()));
    //Reload all each 24hours
    mTimer->start(24*60*60*1000);
}

FollowUpReminderAgent::~FollowUpReminderAgent()
{
}

void FollowUpReminderAgent::setEnableAgent(bool enabled)
{
    if (FollowUpReminderAgentSettings::self()->enabled() == enabled)
        return;

    FollowUpReminderAgentSettings::self()->setEnabled(enabled);
    FollowUpReminderAgentSettings::self()->save();
    if (enabled) {
        mManager->load();
    }
}

bool FollowUpReminderAgent::enabledAgent() const
{
    return FollowUpReminderAgentSettings::self()->enabled();
}

void FollowUpReminderAgent::showConfigureDialog(qlonglong windowId)
{
    QPointer<FollowUpReminderInfoDialog> dialog = new FollowUpReminderInfoDialog();
    if (windowId) {
#ifndef Q_WS_WIN
        KWindowSystem::setMainWindow( dialog, windowId );
#else
        KWindowSystem::setMainWindow( dialog, (HWND)windowId );
#endif
    }
    if (dialog->exec()) {
        //TODO
        //TODO save result
    }
    delete dialog;
}

void FollowUpReminderAgent::configure( WId windowId )
{
    showConfigureDialog((qulonglong)windowId);
}

void FollowUpReminderAgent::itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection )
{
    if (!enabledAgent())
        return;

    if ( item.mimeType() != KMime::Message::mimeType() ) {
        qDebug() << "FollowUpReminderAgent::itemAdded called for a non-message item!";
        return;
    }
    mManager->checkFollowUp(item, collection);
}

void FollowUpReminderAgent::reload()
{
    if (enabledAgent()) {
        mManager->load();
        mTimer->start();
    }
}



AKONADI_AGENT_MAIN( FollowUpReminderAgent )

#include "followupreminderagent.moc"

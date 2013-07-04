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
#include "sendlaterinfo.h"
#include "sendlateragentadaptor.h"
#include "sendlateragentsettings.h"

#include <akonadi/dbusconnectionpool.h>
#include <akonadi/changerecorder.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/session.h>
#include <Akonadi/AttributeFactory>
#include <Akonadi/CollectionFetchScope>
#include <KMime/Message>

#include <KWindowSystem>
#include <KLocale>

#include <QPointer>

#define DEBUG_SENDLATERAGENT 1

SendLaterAgent::SendLaterAgent(const QString &id)
    : Akonadi::AgentBase( id )
{
    mManager = new SendLaterManager(this);
    connect(mManager, SIGNAL(needUpdateConfigDialogBox()), SIGNAL(needUpdateConfigDialogBox()));
    KGlobal::locale()->insertCatalog( QLatin1String("akonadi_sendlater_agent") );
    new SendLaterAgentAdaptor( this );
    Akonadi::DBusConnectionPool::threadConnection().registerObject( QLatin1String( "/SendLaterAgent" ), this, QDBusConnection::ExportAdaptors );
    Akonadi::DBusConnectionPool::threadConnection().registerService( QLatin1String( "org.freedesktop.Akonadi.SendLaterAgent" ) );

    changeRecorder()->setMimeTypeMonitored( KMime::Message::mimeType() );
    changeRecorder()->itemFetchScope().setCacheOnly( true );
    changeRecorder()->itemFetchScope().setFetchModificationTime( false );
    changeRecorder()->fetchCollection( true );
    changeRecorder()->setChangeRecordingEnabled( false );
    changeRecorder()->setAllMonitored(true);
    changeRecorder()->ignoreSession( Akonadi::Session::defaultSession() );
    changeRecorder()->collectionFetchScope().setAncestorRetrieval( Akonadi::CollectionFetchScope::All );
    changeRecorder()->setCollectionMonitored(Akonadi::Collection::root(), true);


    if (SendLaterAgentSettings::enabled()) {
#ifdef DEBUG_SENDLATERAGENT
        QTimer::singleShot(1000, mManager, SLOT(load()));
#else
        QTimer::singleShot(1000*60*4, mManager, SLOT(load()));
#endif
    }
}

SendLaterAgent::~SendLaterAgent()
{
}

void SendLaterAgent::reload()
{
    if (SendLaterAgentSettings::enabled())
        mManager->load(true);
}

void SendLaterAgent::setEnableAgent(bool enabled)
{
    if (SendLaterAgentSettings::enabled() == enabled)
        return;

    SendLaterAgentSettings::setEnabled(enabled);
    SendLaterAgentSettings::self()->writeConfig();
    if (enabled) {
        mManager->load();
    } else {
        mManager->stopAll();
    }
}

bool SendLaterAgent::enabledAgent() const
{
    return SendLaterAgentSettings::enabled();
}

SendLater::SendLaterDialog::SendLaterAction SendLaterAgent::addSendLaterItem(qlonglong itemId, qlonglong windowId)
{
    //TODO 4.12: use it and fix it :)
    SendLater::SendLaterInfo *info = new SendLater::SendLaterInfo;
    info->setItemId(itemId);
    QPointer<SendLater::SendLaterDialog> dialog = new SendLater::SendLaterDialog(info);
    SendLater::SendLaterDialog::SendLaterAction action = SendLater::SendLaterDialog::Canceled;
    if (windowId) {
#ifndef Q_WS_WIN
        KWindowSystem::setMainWindow( dialog, windowId );
#else
        KWindowSystem::setMainWindow( dialog, (HWND)windowId );
#endif
    }
    if (dialog->exec()) {
        info = dialog->info();
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group = config->group(QString::fromLatin1("SendLaterItem %1").arg(info->itemId()));
        info->writeConfig(group);
        action = dialog->action();
        //TODO
        config->sync();
        config->reparseConfiguration();
        mManager->load();
    }
    delete info;
    delete dialog;
    return action;
}

void SendLaterAgent::configure( WId windowId )
{
    showConfigureDialog(windowId);
}

void SendLaterAgent::slotSendNow(Akonadi::Item::Id id)
{
    mManager->sendNow(id);
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
    connect(this, SIGNAL(needUpdateConfigDialogBox()), dialog, SLOT(slotNeedToReloadConfig()));
    connect(dialog, SIGNAL(sendNow(Akonadi::Item::Id)), this, SLOT(slotSendNow(Akonadi::Item::Id)));
    if (dialog->exec()) {
        mManager->load();
    }
    delete dialog;
}

void SendLaterAgent::itemRemoved( const Akonadi::Item &item )
{
    mManager->itemRemoved(item.id());
}

void SendLaterAgent::printDebugInfo()
{
    mManager->printDebugInfo();
}

AKONADI_AGENT_MAIN( SendLaterAgent )

#include "sendlateragent.moc"

/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include "sendlaterutil.h"
#include "sendlateragentadaptor.h"
#include "sendlateragentsettings.h"
#include "sendlaterremovemessagejob.h"

#include <Akonadi/KMime/SpecialMailCollections>
#include <AgentInstance>
#include <AgentManager>
#include <kdbusconnectionpool.h>
#include <changerecorder.h>
#include <itemfetchscope.h>
#include <AkonadiCore/session.h>
#include <AttributeFactory>
#include <CollectionFetchScope>
#include <KMime/Message>

#include <KWindowSystem>
#include <Kdelibs4ConfigMigrator>

#include <QPointer>

//#define DEBUG_SENDLATERAGENT 1

SendLaterAgent::SendLaterAgent(const QString &id)
    : Akonadi::AgentBase(id)
{
    Kdelibs4ConfigMigrator migrate(QLatin1String("sendlateragent"));
    migrate.setConfigFiles(QStringList() << QLatin1String("akonadi_sendlater_agentrc") << QLatin1String("akonadi_sendlater_agent.notifyrc"));
    migrate.migrate();

    mManager = new SendLaterManager(this);
    connect(mManager, &SendLaterManager::needUpdateConfigDialogBox, this, &SendLaterAgent::needUpdateConfigDialogBox);
    new SendLaterAgentAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject(QLatin1String("/SendLaterAgent"), this, QDBusConnection::ExportAdaptors);
    KDBusConnectionPool::threadConnection().registerService(QLatin1String("org.freedesktop.Akonadi.SendLaterAgent"));

    changeRecorder()->setMimeTypeMonitored(KMime::Message::mimeType());
    changeRecorder()->itemFetchScope().setCacheOnly(true);
    changeRecorder()->itemFetchScope().setFetchModificationTime(false);
    changeRecorder()->setChangeRecordingEnabled(false);
    changeRecorder()->ignoreSession(Akonadi::Session::defaultSession());
    setNeedsNetwork(true);

    if (SendLaterAgentSettings::enabled()) {
#ifdef DEBUG_SENDLATERAGENT
        QTimer::singleShot(1000, mManager, SLOT(load()));
#else
        QTimer::singleShot(1000 * 60 * 4, mManager, SLOT(load()));
#endif
    }
}

SendLaterAgent::~SendLaterAgent()
{
}

void SendLaterAgent::doSetOnline(bool online)
{
    if (online) {
        reload();
    } else {
        mManager->stopAll();
    }
}

void SendLaterAgent::reload()
{
    if (SendLaterAgentSettings::enabled()) {
        mManager->load(true);
    }
}

void SendLaterAgent::setEnableAgent(bool enabled)
{
    if (SendLaterAgentSettings::enabled() == enabled) {
        return;
    }

    SendLaterAgentSettings::setEnabled(enabled);
    SendLaterAgentSettings::self()->save();
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

void SendLaterAgent::configure(WId windowId)
{
    showConfigureDialog((qlonglong)windowId);
}

void SendLaterAgent::slotSendNow(Akonadi::Item::Id id)
{
    mManager->sendNow(id);
}

void SendLaterAgent::showConfigureDialog(qlonglong windowId)
{
    QPointer<SendLaterConfigureDialog> dialog = new SendLaterConfigureDialog();
    if (windowId) {
#ifndef Q_OS_WIN
        KWindowSystem::setMainWindow(dialog, windowId);
#else
        KWindowSystem::setMainWindow(dialog, (HWND)windowId);
#endif
    }
    connect(this, &SendLaterAgent::needUpdateConfigDialogBox, dialog.data(), &SendLaterConfigureDialog::slotNeedToReloadConfig);
    connect(dialog.data(), &SendLaterConfigureDialog::sendNow, this, &SendLaterAgent::slotSendNow);
    if (dialog->exec()) {
        mManager->load();
        QList<Akonadi::Item::Id> listMessage = dialog->messagesToRemove();
        if (!listMessage.isEmpty()) {
            //Will delete in specific job when done.
            SendLaterRemoveMessageJob *sendlaterremovejob = new SendLaterRemoveMessageJob(listMessage, this);
            sendlaterremovejob->start();
        }
    }
    delete dialog;
}

void SendLaterAgent::itemsRemoved(const Akonadi::Item::List &items)
{
    Q_FOREACH (const Akonadi::Item &item, items) {
        mManager->itemRemoved(item.id());
    }
}

void SendLaterAgent::itemsMoved(const Akonadi::Item::List &items, const Akonadi::Collection &/*sourceCollection*/, const Akonadi::Collection &destinationCollection)
{
    if (Akonadi::SpecialMailCollections::self()->specialCollectionType(destinationCollection) != Akonadi::SpecialMailCollections::Trash) {
        return;
    }
    itemsRemoved(items);
}

QString SendLaterAgent::printDebugInfo()
{
    return mManager->printDebugInfo();
}

AKONADI_AGENT_MAIN(SendLaterAgent)


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

#include "sendlaterjob.h"
#include "sendlaterinfo.h"

#include "messagecomposer/sender/akonadisender.h"
#include "messagecore/helpers/messagehelpers.h"
#include "messagecore/utils/stringutil.h"

#include <MailTransport/mailtransport/transportattribute.h>
#include <MailTransport/mailtransport/sentbehaviourattribute.h>
#include <MailTransport/mailtransport/messagequeuejob.h>
#include <MailTransport/mailtransport/transport.h>
#include <MailTransport/mailtransport/transportmanager.h>

#include <ItemFetchJob>
#include <ItemDeleteJob>

#include <KNotification>
#include <KLocalizedString>
#include <QIcon>
#include <KIconLoader>
#include <QDebug>

SendLaterJob::SendLaterJob(SendLaterManager *manager, SendLater::SendLaterInfo *info, QObject *parent)
    : QObject(parent),
      mManager(manager),
      mInfo(info)
{
    qDebug() << " SendLaterJob::SendLaterJob" << this;
}

SendLaterJob::~SendLaterJob()
{
    qDebug() << " SendLaterJob::~SendLaterJob()" << this;
}

void SendLaterJob::start()
{
    if (mInfo) {
        if (mInfo->itemId() > -1) {
            const Akonadi::Item item = Akonadi::Item(mInfo->itemId());
            Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob(item, this);
            mFetchScope.fetchAttribute<MailTransport::TransportAttribute>();
            mFetchScope.fetchAttribute<MailTransport::SentBehaviourAttribute>();
            mFetchScope.setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
            mFetchScope.fetchFullPayload(true);
            fetch->setFetchScope(mFetchScope);
            connect(fetch, &Akonadi::ItemFetchJob::itemsReceived, this, &SendLaterJob::slotMessageTransfered);
            connect(fetch, &Akonadi::ItemFetchJob::result, this, &SendLaterJob::slotJobFinished);
            fetch->start();
        } else {
            sendError(i18n("Not message found."), SendLaterManager::ItemNotFound);
        }
    } else {
        sendError(i18n("Not message found."), SendLaterManager::UnknownError);
    }
}

void SendLaterJob::slotMessageTransfered(const Akonadi::Item::List &items)
{
    if (items.isEmpty()) {
        sendError(i18n("No message found."), SendLaterManager::ItemNotFound);
        return;
    } else if (items.count() == 1) {
        //Success
        mItem = items.first();
        return;
    }
    qDebug() << "Error during fetching message.";
    sendError(i18n("Error during fetching message."), SendLaterManager::TooManyItemFound);
}

void SendLaterJob::slotJobFinished(KJob *job)
{
    if (job->error()) {
        sendError(i18n("Cannot fetch message. %1", job->errorString()), SendLaterManager::CanNotFetchItem);
        return;
    }
    if (!MailTransport::TransportManager::self()->showTransportCreationDialog(0, MailTransport::TransportManager::IfNoTransportExists)) {
        qDebug() << " we can't create transport ";
        sendError(i18n("We can't create transport"), SendLaterManager::CanNotCreateTransport);
        return;
    }

    if (mItem.isValid()) {
        const KMime::Message::Ptr msg = MessageCore::Util::message(mItem);
        if (!msg) {
            sendError(i18n("Message is not a real message"), SendLaterManager::CanNotFetchItem);
            return;
        }
        updateAndCleanMessageBeforeSending(msg);

        if (!mManager->sender()->send(msg, MessageComposer::MessageSender::SendImmediate)) {
            sendError(i18n("Cannot send message."), SendLaterManager::MailDispatchDoesntWork);
        } else {
            if (!mInfo->isRecurrence()) {
                Akonadi::ItemDeleteJob *fetch = new Akonadi::ItemDeleteJob(mItem, this);
                connect(fetch, &Akonadi::ItemDeleteJob::result, this, &SendLaterJob::slotDeleteItem);
            } else {
                sendDone();
            }
        }
    }
}

void SendLaterJob::updateAndCleanMessageBeforeSending(const KMime::Message::Ptr &msg)
{
    msg->date()->setDateTime(QDateTime::currentDateTime());
    MessageCore::StringUtil::removePrivateHeaderFields(msg, true);
    msg->removeHeader("X-KMail-SignatureActionEnabled");
    msg->removeHeader("X-KMail-EncryptActionEnabled");
    msg->removeHeader("X-KMail-CryptoMessageFormat");
    msg->assemble();
}

void SendLaterJob::slotDeleteItem(KJob *job)
{
    if (job->error()) {
        qDebug() << " void SendLaterJob::slotDeleteItem( KJob *job ) :" << job->errorString();
    }
    sendDone();
}

void SendLaterJob::sendDone()
{
    const QPixmap pixmap = QIcon::fromTheme(QLatin1String("kmail")).pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    KNotification::event(QLatin1String("mailsend"),
                         i18n("Message sent"),
                         pixmap,
                         0,
                         KNotification::CloseOnTimeout,
                         QLatin1String("akonadi_sendlater_agent"));
    mManager->sendDone(mInfo);
    deleteLater();
}

void SendLaterJob::sendError(const QString &error, SendLaterManager::ErrorType type)
{
    const QPixmap pixmap = QIcon::fromTheme(QLatin1String("kmail")).pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    KNotification::event(QLatin1String("mailsendfailed"),
                         error,
                         pixmap,
                         0,
                         KNotification::CloseOnTimeout,
                         QLatin1String("akonadi_sendlater_agent"));
    mManager->sendError(mInfo, type);
    deleteLater();
}


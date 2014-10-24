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

#include "followupreminderjob.h"

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/KMime/MessageParts>

#include <KMime/Message>

#include <KDebug>

FollowUpReminderJob::FollowUpReminderJob(QObject *parent)
    : QObject(parent)
{
}

FollowUpReminderJob::~FollowUpReminderJob()
{

}

void FollowUpReminderJob::start()
{
    if (!mItem.isValid()) {
        qDebug()<<" item is not valid";
        deleteLater();
        return;
    }
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(mItem);
    job->fetchScope().fetchPayloadPart( Akonadi::MessagePart::Envelope, true );
    job->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );

    connect( job, SIGNAL(result(KJob*)), SLOT(slotItemFetchJobDone(KJob*)) );
}

void FollowUpReminderJob::setItem(const Akonadi::Item &item)
{
    mItem = item;
}

void FollowUpReminderJob::slotItemFetchJobDone(KJob* job)
{
    if ( job->error() ) {
        kError() << "Error while fetching item. " << job->error() << job->errorString();
        deleteLater();
        return;
    }

    const Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );

    const Akonadi::Item::List items = fetchJob->items();
    if ( items.isEmpty() ) {
        kError() << "Error while fetching item: item not found";
        deleteLater();
        return;
    }
    const Akonadi::Item item = items.at(0);
    if ( !item.hasPayload<KMime::Message::Ptr>() ) {
        kError() << "Item has not payload";
        deleteLater();
        return;
    }
    const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
    if (msg) {
        KMime::Headers::InReplyTo *replyTo = msg->inReplyTo(false);
        if (replyTo) {
            const QString replyToIdStr = replyTo->asUnicodeString();
            qDebug()<<"Reply to"<<replyToIdStr;
            Q_EMIT finished(replyToIdStr, item.id());
        }
    }
    deleteLater();
}

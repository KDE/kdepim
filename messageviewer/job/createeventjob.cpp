/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "createeventjob.h"
#include "messageviewer_debug.h"
#include <Akonadi/KMime/MessageParts>
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/ItemCreateJob>

#include <KMime/Message>

using namespace MessageViewer;

CreateEventJob::CreateEventJob(const KCalCore::Event::Ptr &eventPtr, const Akonadi::Collection &collection, const Akonadi::Item &item, QObject *parent)
    : KJob(parent),
      mItem(item),
      mCollection(collection),
      mEventPtr(eventPtr)
{
}

CreateEventJob::~CreateEventJob()
{
    qCDebug(MESSAGEVIEWER_LOG) << " CreateEventJob::~CreateEventJob()";
}

void CreateEventJob::start()
{
    // We need the full payload to attach the mail to the incidence
    if (!mItem.loadedPayloadParts().contains(Akonadi::MessagePart::Body)) {
        Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(mItem);
        job->fetchScope().fetchFullPayload();
        connect(job, &Akonadi::ItemFetchJob::result, this, &CreateEventJob::slotFetchDone);
    } else {
        createEvent();
    }
}

void CreateEventJob::slotFetchDone(KJob *job)
{
    qCDebug(MESSAGEVIEWER_LOG) << " void CreateEventJob::slotFetchDone(KJob *job)";
    Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob *>(job);
    if (fetchJob->items().count() == 1) {
        mItem = fetchJob->items().first();
    } else {
        qCDebug(MESSAGEVIEWER_LOG) << " createTodo Error during fetch: " << job->errorString();
        Q_EMIT emitResult();
        return;
    }
    createEvent();
}

void CreateEventJob::createEvent()
{
    if (!mItem.hasPayload<KMime::Message::Ptr>()) {
        qCDebug(MESSAGEVIEWER_LOG) << " item has not payload";
        Q_EMIT emitResult();
        return;
    }
    KMime::Message::Ptr msg =  mItem.payload<KMime::Message::Ptr>();

    KCalCore::Attachment::Ptr attachmentPtr(new KCalCore::Attachment(msg->encodedContent().toBase64(), KMime::Message::mimeType()));
    const KMime::Headers::Subject *const subject = msg->subject(false);
    if (subject) {
        attachmentPtr->setLabel(subject->asUnicodeString());
    }
    mEventPtr->addAttachment(attachmentPtr);

    Akonadi::Item newEventItem;
    newEventItem.setMimeType(KCalCore::Event::eventMimeType());
    newEventItem.setPayload<KCalCore::Event::Ptr>(mEventPtr);

    Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(newEventItem, mCollection);
    connect(createJob, &Akonadi::ItemCreateJob::result, this, &CreateEventJob::slotCreateNewEvent);
}

void CreateEventJob::slotCreateNewEvent(KJob *job)
{
    if (job->error()) {
        qCDebug(MESSAGEVIEWER_LOG) << "Error during create new Todo " << job->errorString();
    }
    Q_EMIT emitResult();
}

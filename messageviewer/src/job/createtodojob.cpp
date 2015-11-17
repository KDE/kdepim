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

#include "createtodojob.h"
#include "messageviewer_debug.h"
#include <Akonadi/KMime/MessageParts>
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/ItemCreateJob>
#include <AkonadiCore/RelationCreateJob>

#include <KMime/Message>

using namespace MessageViewer;

CreateTodoJob::CreateTodoJob(const KCalCore::Todo::Ptr &todoPtr, const Akonadi::Collection &collection, const Akonadi::Item &item, QObject *parent)
    : KJob(parent),
      mItem(item),
      mCollection(collection),
      mTodoPtr(todoPtr)
{
}

CreateTodoJob::~CreateTodoJob()
{
    qCDebug(MESSAGEVIEWER_LOG) << " CreateTodoJob::~CreateTodoJob()";
}

void CreateTodoJob::start()
{
    // We need the full payload to attach the mail to the incidence
    if (!mItem.loadedPayloadParts().contains(Akonadi::MessagePart::Body)) {
        Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(mItem);
        job->fetchScope().fetchFullPayload();
        connect(job, &Akonadi::ItemFetchJob::result, this, &CreateTodoJob::slotFetchDone);
    } else {
        createTodo();
    }
}

void CreateTodoJob::slotFetchDone(KJob *job)
{
    Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob *>(job);
    if (fetchJob->items().count() == 1) {
        mItem = fetchJob->items().at(0);
    } else {
        qCDebug(MESSAGEVIEWER_LOG) << " createTodo Error during fetch: " << job->errorString();
        Q_EMIT emitResult();
        return;
    }
    createTodo();
}

void CreateTodoJob::createTodo()
{
    if (!mItem.hasPayload<KMime::Message::Ptr>()) {
        qCDebug(MESSAGEVIEWER_LOG) << " item has not payload";
        Q_EMIT emitResult();
        return;
    }

    Akonadi::Item newTodoItem;
    newTodoItem.setMimeType(KCalCore::Todo::todoMimeType());
    newTodoItem.setPayload<KCalCore::Todo::Ptr>(mTodoPtr);

    Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(newTodoItem, mCollection);
    connect(createJob, &Akonadi::ItemCreateJob::result, this, &CreateTodoJob::todoCreated);
}

void CreateTodoJob::todoCreated(KJob *job)
{
    if (job->error()) {
        qCDebug(MESSAGEVIEWER_LOG) << "Error during create new Todo " << job->errorString();
        setError(job->error());
        setErrorText(job->errorText());
        Q_EMIT emitResult();
    } else {
        Akonadi::ItemCreateJob *createJob = static_cast<Akonadi::ItemCreateJob *>(job);
        Akonadi::Relation relation(Akonadi::Relation::GENERIC, mItem, createJob->item());
        Akonadi::RelationCreateJob *job = new Akonadi::RelationCreateJob(relation);
    }
}

void CreateTodoJob::relationCreated(KJob *job)
{
    if (job->error()) {
        qCDebug(MESSAGEVIEWER_LOG) << "Error during create new Todo " << job->errorString();
    }
    Q_EMIT emitResult();
}

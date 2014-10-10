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

#include "createeventjob.h"

#include <Akonadi/KMime/MessageParts>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/RelationCreateJob>

#include <KMime/Message>
#include <QDebug>

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
}

void CreateEventJob::start()
{
    // We need the full payload to attach the mail to the incidence
    if ( !mItem.loadedPayloadParts().contains( Akonadi::MessagePart::Body ) ) {
        Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( mItem );
        job->fetchScope().fetchFullPayload();
        connect( job, SIGNAL(result(KJob*)), this, SLOT(slotFetchDone(KJob*)) );
    } else {
        createEvent();
    }
}

void CreateEventJob::slotFetchDone(KJob *job)
{
    Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob *>(job);
    if ( fetchJob->items().count() == 1 ) {
        mItem = fetchJob->items().first();
    } else {
        Q_EMIT emitResult();
        return;
    }
    createEvent();
}

void CreateEventJob::createEvent()
{
    if ( !mItem.hasPayload<KMime::Message::Ptr>() ) {
        qDebug()<<" item has not payload";
        Q_EMIT emitResult();
        return;
    }

    Akonadi::Item newTodoItem;
    newTodoItem.setMimeType( KCalCore::Event::eventMimeType() );
    newTodoItem.setPayload<KCalCore::Event::Ptr>( mEventPtr );

    Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(newTodoItem, mCollection);
    connect(createJob, SIGNAL(result(KJob*)), this, SLOT(eventCreated(KJob*)));
}

void CreateEventJob::eventCreated(KJob *job)
{
    if ( job->error() ) {
        qDebug() << "Error during create new Event "<<job->errorString();
        setError( job->error() );
        setErrorText( job->errorText() );
        emitResult();
    } else {
        Akonadi::ItemCreateJob *createJob = static_cast<Akonadi::ItemCreateJob *> ( job );
        Akonadi::Relation relation( Akonadi::Relation::GENERIC, mItem, createJob->item() );
        Akonadi::RelationCreateJob *job = new Akonadi::RelationCreateJob( relation );
        connect( job, SIGNAL( result( KJob * ) ), this, SLOT( relationCreated( KJob * ) ) );
    }
}

void CreateEventJob::relationCreated(KJob *job)
{
   Q_UNUSED(job)
   emitResult();
}

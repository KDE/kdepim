/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#include "createnotejob.h"

#include <Akonadi/KMime/MessageParts>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemCreateJob>

#include <KMime/Message>
#include <QApplication>
#include <QDebug>

using namespace MessageViewer;

CreateNoteJob::CreateNoteJob(const KMime::Message::Ptr &notePtr, const Akonadi::Collection &collection, const Akonadi::Item &item, QObject *parent)
    : KJob(parent),
      mItem(item),
      mCollection(collection),
      mNote(notePtr)
{
}

CreateNoteJob::~CreateNoteJob()
{
    qDebug()<<" CreateNoteJob::~CreateNoteJob()";
}

void CreateNoteJob::start()
{
    // We need the full payload to attach the mail
    if ( !mItem.loadedPayloadParts().contains( Akonadi::MessagePart::Body ) ) {
        Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( mItem );
        job->fetchScope().fetchFullPayload();
        connect( job, SIGNAL(result(KJob*)), this, SLOT(slotFetchDone(KJob*)) );

        if ( job->exec() ) {
            if ( job->items().count() == 1 ) {
                mItem = job->items().first();
            }
        } else {
            qDebug()<<" createNote: Error during fetch: "<<job->errorString();
        }
    } else {
        createNote();
    }
}

void CreateNoteJob::slotFetchDone(KJob *job)
{
    qDebug()<<" void CreateNoteJob::slotFetchDone(KJob *job)";
    Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob *>(job);
    if ( fetchJob->items().count() == 1 ) {
        mItem = fetchJob->items().first();
    } else {
        Q_EMIT emitResult();
        return;
    }
    createNote();
}

void CreateNoteJob::createNote()
{
    if ( !mItem.hasPayload<KMime::Message::Ptr>() ) {
        qDebug()<<" item has not payload";
        Q_EMIT emitResult();
        return;
    }
    KMime::Message::Ptr msg =  mItem.payload<KMime::Message::Ptr>();

    Akonadi::NoteUtils::Attachment attachment(msg->encodedContent(), msg->mimeType());
    const KMime::Headers::Subject * const subject = msg->subject(false);
    if (subject)
        attachment.setLabel(subject->asUnicodeString());
    mNote.attachments().append(attachment);

    mNote.setFrom(QCoreApplication::applicationName()+QCoreApplication::applicationVersion());
    mNote.setLastModifiedDate(KDateTime::currentUtcDateTime());

    Akonadi::Item newNoteItem;
    newNoteItem.setMimeType( Akonadi::NoteUtils::noteMimeType() );
    newNoteItem.setPayload( mNote.message() );

    Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(newNoteItem, mCollection);
    connect(createJob, SIGNAL(result(KJob*)), this, SLOT(slotCreateNewNote(KJob*)));
}

void CreateNoteJob::slotCreateNewNote(KJob *job)
{
    if ( job->error() ) {
        qDebug() << "Error during create new Note "<<job->errorString();
    }
    Q_EMIT emitResult();
}

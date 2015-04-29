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
#include <Akonadi/RelationCreateJob>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/Relation>

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
}

void CreateNoteJob::start()
{
    mNote.setFrom(QCoreApplication::applicationName() + QCoreApplication::applicationVersion());
    mNote.setLastModifiedDate(KDateTime::currentUtcDateTime());
    if (!mItem.relations().isEmpty())  {
        Akonadi::Relation relation;
        foreach (const Akonadi::Relation &r, mItem.relations()) {
            // assuming that GENERIC relations to emails are notes is a pretty horirific hack imo - aseigo
            if (r.type() == Akonadi::Relation::GENERIC && r.right().mimeType() == Akonadi::NoteUtils::noteMimeType() ) {
                relation = r;
                break;
            }
        }

        if (relation.isValid()) {
            Akonadi::Item item = relation.right();
            item.setMimeType(Akonadi::NoteUtils::noteMimeType());
            item.setPayload(mNote.message());
            Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(item);
            connect(modifyJob, SIGNAL(result(KJob*)), this, SLOT(noteUpdated(KJob*)));
            return;
        }
    }

    Akonadi::Item newNoteItem;
    newNoteItem.setMimeType( Akonadi::NoteUtils::noteMimeType() );
    newNoteItem.setPayload( mNote.message() );
    Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(newNoteItem, mCollection);
    connect(createJob, SIGNAL(result(KJob*)), this, SLOT(noteCreated(KJob*)));
}

void CreateNoteJob::noteCreated(KJob *job)
{
    if ( job->error() ) {
        qDebug() << "Error during create new Note "<<job->errorString();
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

void CreateNoteJob::noteUpdated(KJob *job)
{
    if ( job->error() ) {
        setError( job->error() );
        setErrorText( job->errorText() );
    }

    emitResult();
}

void CreateNoteJob::relationCreated(KJob *job)
{
   Q_UNUSED(job)
   emitResult();
}


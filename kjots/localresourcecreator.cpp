/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "localresourcecreator.h"

#include "akonadi_next/note.h"

#include <KDebug>
#include <KLocalizedString>
#include <KRandom>
#include <AkonadiCore/CollectionFetchJob>
#include <Akonadi/AgentInstance>
#include <Akonadi/AgentManager>
#include <Akonadi/CollectionCreateJob>
#include <AkonadiCore/ItemCreateJob>
#include <AkonadiCore/item.h>
#include <KMime/KMimeMessage>
#include <AkonadiCore/EntityDisplayAttribute>

LocalResourceCreator::LocalResourceCreator(QObject* parent)
  : NoteShared::LocalResourceCreator(parent)
{

}

void LocalResourceCreator::finishCreateResource()
{
    Akonadi::CollectionFetchJob *collectionFetchJob = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::FirstLevel, this );
    connect( collectionFetchJob, SIGNAL(result(KJob*)), SLOT(rootFetchFinished(KJob*)) );
}

void LocalResourceCreator::rootFetchFinished(KJob* job)
{
  if (job->error()) {
    kWarning() << job->errorString();
    deleteLater();
    return;
  }

  Akonadi::CollectionFetchJob *lastCollectionFetchJob = qobject_cast<Akonadi::CollectionFetchJob*>(job);
  if (!lastCollectionFetchJob) {
    deleteLater();
    return;
  }

  Akonadi::Collection::List list = lastCollectionFetchJob->collections();

  if (list.isEmpty())
  {
    kWarning() << "Couldn't find new collection in resource";
    deleteLater();
    return;
  }

  foreach (const Akonadi::Collection &col, list)
  {
    Akonadi::AgentInstance instance = Akonadi::AgentManager::self()->instance(col.resource());
    if (instance.type().identifier() == akonadiNotesInstanceName())
    {
      Akonadi::CollectionFetchJob *collectionFetchJob = new Akonadi::CollectionFetchJob( col, Akonadi::CollectionFetchJob::FirstLevel, this );
      collectionFetchJob->setProperty("FetchedCollection", col.id());
      connect( collectionFetchJob, SIGNAL(result(KJob*)), SLOT(topLevelFetchFinished(KJob*)) );
      return;
    }
  }
  Q_ASSERT(!"Couldn't find new collection");
  deleteLater();
}

void LocalResourceCreator::topLevelFetchFinished(KJob* job)
{
  if (job->error()) {
    kWarning() << job->errorString();
    deleteLater();
    return;
  }

  Akonadi::CollectionFetchJob *lastCollectionFetchJob = qobject_cast<Akonadi::CollectionFetchJob*>(job);
  if (!lastCollectionFetchJob) {
    deleteLater();
    return;
  }

  Akonadi::Collection::List list = lastCollectionFetchJob->collections();

  if (!list.isEmpty())
  {
    deleteLater();
    return;
  }

  Akonadi::Collection::Id id = lastCollectionFetchJob->property("FetchedCollection").toLongLong();

  Akonadi::Collection collection;
  collection.setParentCollection( Akonadi::Collection(id) );
  QString title = i18nc( "The default name for new books.", "New Book" );
  collection.setName( KRandom::randomString( 10 ) );
  collection.setContentMimeTypes( QStringList() << Akonadi::Collection::mimeType() << Akonotes::Note::mimeType() );

  Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();
  eda->setIconName( QLatin1String("x-office-address-book") );
  eda->setDisplayName( title );
  collection.addAttribute(eda);

  Akonadi::CollectionCreateJob *createJob = new Akonadi::CollectionCreateJob( collection, this );
  connect( createJob, SIGNAL(result(KJob*)), this, SLOT(createFinished(KJob*)) );

}

void LocalResourceCreator::createFinished(KJob* job)
{
  if (job->error()) {
    kWarning() << job->errorString();
    deleteLater();
    return;
  }

  Akonadi::CollectionCreateJob* collectionCreateJob = qobject_cast<Akonadi::CollectionCreateJob*>(job);
  if (!collectionCreateJob)
  {
    deleteLater();
    return;
  }

  Akonadi::Item item;
  item.setParentCollection(collectionCreateJob->collection());
  item.setMimeType( Akonotes::Note::mimeType() );

  KMime::Message::Ptr note( new KMime::Message() );

  QString title = i18nc( "The default name for new pages.", "New Page" );
  QByteArray encoding( "utf-8" );

  note->subject( true )->fromUnicodeString( title, encoding );
  note->contentType( true )->setMimeType( "text/plain" );
  note->date( true )->setDateTime( KDateTime::currentLocalDateTime() );
  note->from( true )->fromUnicodeString( QLatin1String("Kjots@kde4"), encoding );
  // Need a non-empty body part so that the serializer regards this as a valid message.
  note->mainBodyPart()->fromUnicodeString( QLatin1String(" ") );

  note->assemble();

  item.setPayload(note);
  Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();
  eda->setIconName( QLatin1String("text-plain") );
  item.addAttribute(eda);

  Akonadi::ItemCreateJob *itemCreateJob = new Akonadi::ItemCreateJob( item,  collectionCreateJob->collection(), this);
  connect( itemCreateJob, SIGNAL(result(KJob*)), SLOT(itemCreateFinished(KJob*)) );
}

void LocalResourceCreator::itemCreateFinished(KJob* job)
{
  if (job->error()) {
    kWarning() << job->errorString();
  }
  deleteLater();
}







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

#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>
#include "maildirsettings.h"

#include "akonadi_next/note.h"

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KRandom>
#include <KStandardDirs>
#include <akonadi/resourcesynchronizationjob.h>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionCreateJob>
#include <Akonadi/ItemCreateJob>
#include <akonadi/item.h>
#include <KMime/KMimeMessage>
#include <Akonadi/EntityDisplayAttribute>

static const char * akonadi_notes_instance_name = "akonadi_akonotes_resource";

LocalResourceCreator::LocalResourceCreator(QObject* parent)
  : QObject(parent)
{

}

void LocalResourceCreator::createIfMissing()
{
  Akonadi::AgentManager *manager = Akonadi::AgentManager::self();

  Akonadi::AgentInstance::List instances = manager->instances();
  bool found = false;
  foreach ( const Akonadi::AgentInstance& instance, instances ) {
    if (instance.type().identifier() == akonadi_notes_instance_name)
    {
      found = true;
      break;
    }
  }
  if (found)
  {
    deleteLater();
    return;
  }
  createInstance();
}

void LocalResourceCreator::createInstance()
{
  Akonadi::AgentType notesType = Akonadi::AgentManager::self()->type( akonadi_notes_instance_name );

  Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob( notesType );
  connect( job, SIGNAL(result(KJob*)),
      this, SLOT(instanceCreated(KJob*)) );

  job->start();
}

void LocalResourceCreator::instanceCreated( KJob *job )
{
  if (job->error()) {
    kWarning() << job->errorString();
    deleteLater();
    return;
  }

  Akonadi::AgentInstanceCreateJob *createJob = qobject_cast<Akonadi::AgentInstanceCreateJob*>(job);
  Akonadi::AgentInstance instance = createJob->instance();

  instance.setName( i18nc( "Default name for resource holding notes", "Local Notes" ) );

  OrgKdeAkonadiMaildirSettingsInterface *iface = new OrgKdeAkonadiMaildirSettingsInterface(
    "org.freedesktop.Akonadi.Resource." + instance.identifier(),
    "/Settings", QDBusConnection::sessionBus(), this );

  // TODO: Make errors user-visible.
  if (!iface->isValid() ) {
    kWarning() << "Failed to obtain D-Bus interface for remote configuration.";
    delete iface;
    deleteLater();
    return;
  }

  QDBusPendingReply<void> response = iface->setPath( KGlobal::dirs()->localxdgdatadir() + "/notes/" + KRandom::randomString( 10 ) );

  instance.reconfigure();

  Akonadi::ResourceSynchronizationJob *syncJob = new Akonadi::ResourceSynchronizationJob(instance, this);
  connect( syncJob, SIGNAL(result(KJob*)), SLOT(syncDone(KJob*)));
  syncJob->start();
}

void LocalResourceCreator::syncDone(KJob* job)
{
  if ( job->error() ) {
    kWarning() << "Synchronizing the resource failed:" << job->errorString();
    deleteLater();
    return;
  }

  kWarning() << "Instance synchronized";

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
    if (instance.type().identifier() == akonadi_notes_instance_name)
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
  eda->setIconName( "x-office-address-book" );
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
  note->from( true )->fromUnicodeString( "Kjots@kde4", encoding );
  // Need a non-empty body part so that the serializer regards this as a valid message.
  note->mainBodyPart()->fromUnicodeString( " " );

  note->assemble();

  item.setPayload(note);
  Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();
  eda->setIconName( "text-plain" );
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






#include "localresourcecreator.moc"

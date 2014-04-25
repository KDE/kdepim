/*
  Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "backuper.h"

#include <calendarsupport/utils.h>

#include <KCalCore/Incidence>
#include <KCalCore/FileStorage>

#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>

#include <KLocale>
#include <KJob>
#include <KDebug>

#include <QCoreApplication>

static void print(const QString &message)
{
    QTextStream out(stdout);
    out << message << "\n";
}

void Backuper::emitFinished(bool success, const QString &message)
{
    if (success) {
        print(QLatin1Char('\n') + i18np("Backup was successful. %1 incidence was saved.", "Backup was successful. %1 incidences were saved.", m_calendar->incidences().count()));
    } else {
        print(message);
    }

    m_calendar.clear();

    emit finished(success, message);
    qApp->exit(success ? 0 : -1); // TODO: If we move this class to kdepimlibs, remove this
}

Backuper::Backuper(QObject *parent) : QObject(parent), m_backupInProgress(false)
{
}

void Backuper::backup(const QString &filename, const QList<Akonadi::Entity::Id> &collectionIds)
{
    if (filename.isEmpty()) {
        emitFinished(false, i18n("File is empty."));
        return;
    }

    if (m_backupInProgress) {
        emitFinished(false, i18n("A backup is already in progress."));
        return;
    }
    print(i18n("Backing up your calendar data..."));
    m_calendar = KCalCore::MemoryCalendar::Ptr(new KCalCore::MemoryCalendar(KDateTime::LocalZone));
    m_requestedCollectionIds = collectionIds;
    m_backupInProgress = true;
    m_filename = filename;

    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
                                                                       Akonadi::CollectionFetchJob::Recursive);

    job->fetchScope().setContentMimeTypes(KCalCore::Incidence::mimeTypes());
    connect(job, SIGNAL(result(KJob*)), SLOT(onCollectionsFetched(KJob*)));
    job->start();
}

void Backuper::onCollectionsFetched(KJob *job)
{
    if (job->error() == 0) {
        QSet<QString> mimeTypeSet = KCalCore::Incidence::mimeTypes().toSet();
        Akonadi::CollectionFetchJob *cfj = qobject_cast<Akonadi::CollectionFetchJob*>(job);
        foreach(const Akonadi::Collection &collection, cfj->collections()) {
            if (!m_requestedCollectionIds.isEmpty() && !m_requestedCollectionIds.contains(collection.id()))
                continue;
            if (!mimeTypeSet.intersect(collection.contentMimeTypes().toSet()).isEmpty()) {
                m_collections << collection;
                loadCollection(collection);
            }
        }

        if (m_collections.isEmpty()) {
            emitFinished(false, i18n("No data to backup."));
        }
    } else {
        kError() << job->errorString();
        m_backupInProgress = false;
        emitFinished(false, job->errorString());
    }
}

void Backuper::loadCollection(const Akonadi::Collection &collection)
{
    print(i18n("Processing collection %1 (id=%2)...", collection.displayName(), collection.id()));
    Akonadi::ItemFetchJob *ifj = new Akonadi::ItemFetchJob(collection, this);
    ifj->setProperty("collectionId", collection.id());
    ifj->fetchScope().fetchFullPayload(true);
    connect(ifj, SIGNAL(result(KJob*)), SLOT(onCollectionLoaded(KJob*)));
    m_pendingCollections << collection.id();
}

void Backuper::onCollectionLoaded(KJob *job)
{
    if (job->error()) {
        m_backupInProgress = false;
        m_calendar.clear();
        emitFinished(false, job->errorString());
    } else {
        Akonadi::ItemFetchJob *ifj = qobject_cast<Akonadi::ItemFetchJob *>(job);
        Akonadi::Collection::Id id = ifj->property("collectionId").toInt();
        Q_ASSERT(id != -1);
        Akonadi::Item::List items = ifj->items();
        m_pendingCollections.removeAll(id);

        foreach (const Akonadi::Item &item, items) {
            KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
            Q_ASSERT(incidence);
            m_calendar->addIncidence(incidence);
        }

        if (m_pendingCollections.isEmpty()) { // We're done
            KCalCore::FileStorage storage(m_calendar, m_filename);
            bool success = storage.save();
            QString message = success ? QString() : i18n("An error occurred");
            emitFinished(success, message);
        }
    }
}

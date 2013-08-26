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

#include "collectionloader.h"

#include <KCalCore/Incidence>

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <QString>
#include <QSet>


CollectionLoader::CollectionLoader(QObject *parent) :
    QObject(parent)
{
}

void CollectionLoader::load()
{
    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
                                                                       Akonadi::CollectionFetchJob::Recursive);

    job->fetchScope().setContentMimeTypes(KCalCore::Incidence::mimeTypes());
    connect(job, SIGNAL(result(KJob*)), SLOT(onCollectionsLoaded(KJob *)));
    job->start();
}

Akonadi::Collection::List CollectionLoader::collections() const
{
    return m_collections;
}

void CollectionLoader::onCollectionsLoaded(KJob *job)
{
    if (job->error() == 0) {
        Akonadi::CollectionFetchJob *cfj = qobject_cast<Akonadi::CollectionFetchJob*>(job);
        Q_ASSERT(cfj);
        foreach(const Akonadi::Collection &collection, cfj->collections()) {
            QSet<QString> mimeTypeSet = KCalCore::Incidence::mimeTypes().toSet();
            if (!mimeTypeSet.intersect(collection.contentMimeTypes().toSet()).isEmpty()) {
                m_collections << collection;
            }
        }
        emit loaded(true);
    } else {
        kError() << job->errorString();
        emit loaded(false);
    }
}

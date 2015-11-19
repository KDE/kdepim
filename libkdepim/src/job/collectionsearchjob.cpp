/*
Copyright (C) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include "collectionsearchjob.h"
#include "libkdepim_debug.h"

#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>
#include <AkonadiSearch/PIM/collectionquery.h>

using namespace KPIM;
class KPIM::CollectionSearchJobPrivate
{
public:
    CollectionSearchJobPrivate(const QString &searchString, const QStringList &mimetypeFilter)
        : mSearchString(searchString),
          mMimeTypeFilter(mimetypeFilter)
    {
    }
    QString mSearchString;
    QStringList mMimeTypeFilter;
    Akonadi::Collection::List mMatchingCollections;
    Akonadi::Collection::List mAncestors;
};

CollectionSearchJob::CollectionSearchJob(const QString &searchString, const QStringList &mimetypeFilter, QObject *parent)
    : KJob(parent),
      d(new KPIM::CollectionSearchJobPrivate(searchString, mimetypeFilter))
{
}

CollectionSearchJob::~CollectionSearchJob()
{
    delete d;
}

void CollectionSearchJob::start()
{
    Akonadi::Search::PIM::CollectionQuery query;
    if (d->mSearchString == QLatin1String("*")) {
        query.setNamespace(QStringList() << QStringLiteral(""));
    } else {
        //We exclude the other users namespace
        query.setNamespace(QStringList() << QStringLiteral("shared") << QStringLiteral(""));
        query.pathMatches(d->mSearchString);
    }
    query.setMimetype(d->mMimeTypeFilter);
    query.setLimit(200);
    Akonadi::Search::PIM::ResultIterator it = query.exec();
    Akonadi::Collection::List collections;
    while (it.next()) {
        collections << Akonadi::Collection(it.id());
    }
    qCDebug(LIBKDEPIM_LOG) << "Found collections " << collections.size();

    if (collections.isEmpty()) {
        //We didn't find anything
        emitResult();
        return;
    }

    Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(collections, Akonadi::CollectionFetchJob::Base, this);
    fetchJob->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
    fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
    fetchJob->fetchScope().setIgnoreRetrievalErrors(true);
    connect(fetchJob, &Akonadi::CollectionFetchJob::collectionsReceived, this, &CollectionSearchJob::onCollectionsReceived);
    connect(fetchJob, &Akonadi::CollectionFetchJob::result, this, &CollectionSearchJob::onCollectionsFetched);
}

void CollectionSearchJob::onCollectionsReceived(const Akonadi::Collection::List &list)
{
    Q_FOREACH (const Akonadi::Collection &col, list) {
        d->mMatchingCollections << col;
        Akonadi::Collection ancestor = col.parentCollection();
        while (ancestor.isValid() && (ancestor != Akonadi::Collection::root())) {
            if (!d->mAncestors.contains(ancestor)) {
                d->mAncestors << ancestor;
            }
            ancestor = ancestor.parentCollection();
        }
    }
}

void CollectionSearchJob::onCollectionsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(LIBKDEPIM_LOG) << job->errorString();
        emitResult();
        return;
    }
    if (!d->mAncestors.isEmpty()) {
        Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(d->mAncestors, Akonadi::CollectionFetchJob::Base, this);
        fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
        connect(fetchJob, &Akonadi::CollectionFetchJob::result, this, &CollectionSearchJob::onAncestorsFetched);
    } else {
        //We didn't find anything
        emitResult();
    }
}

static Akonadi::Collection replaceParent(Akonadi::Collection col, const Akonadi::Collection::List &ancestors)
{
    if (!col.isValid()) {
        return col;
    }
    const Akonadi::Collection parent = replaceParent(col.parentCollection(), ancestors);
    Q_FOREACH (const Akonadi::Collection &c, ancestors) {
        if (col == c) {
            col = c;
            break;
        }
    }
    col.setParentCollection(parent);
    return col;
}

void CollectionSearchJob::onAncestorsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(LIBKDEPIM_LOG) << job->errorString();
        emitResult();
        return;
    }
    Akonadi::CollectionFetchJob *fetchJob = static_cast<Akonadi::CollectionFetchJob *>(job);
    Akonadi::Collection::List matchingCollections;
    matchingCollections.reserve(d->mMatchingCollections.count());
    Q_FOREACH (const Akonadi::Collection &c, d->mMatchingCollections) {
        //We need to replace the parents with the version that contains the name, so we can display it accordingly
        matchingCollections << replaceParent(c, fetchJob->collections());
    }
    d->mMatchingCollections = matchingCollections;
    emitResult();
}

Akonadi::Collection::List CollectionSearchJob::matchingCollections() const
{
    return d->mMatchingCollections;
}


/*
 * Copyright (c) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "searchcollectionindexingwarning.h"

#include <akonadi/persistentsearchattribute.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/collectionstatistics.h>
#include <akonadi/entityhiddenattribute.h>
#include <akonadi/cachepolicy.h>

#include <pimcommon/util/indexerutils.h>

#include <QDBusInterface>

#include <KLocalizedString>

SearchCollectionIndexingWarning::SearchCollectionIndexingWarning(QWidget *parent)
    : KMessageWidget(parent)
{
    setVisible(false);
    setWordWrap(true);
    setText(i18n("Some of the search folders in this query are still being indexed "
                 "or are excluded from indexing completely. The results below may be incomplete."));
    setCloseButtonVisible(true);
    setMessageType(Information);
}

SearchCollectionIndexingWarning::~SearchCollectionIndexingWarning()
{
}

Akonadi::CollectionFetchJob *SearchCollectionIndexingWarning::fetchCollections(const Akonadi::Collection::List& cols,
                                                                               bool recursive)
{
    const Akonadi::CollectionFetchJob::Type type = recursive ?
                                                Akonadi::CollectionFetchJob::Recursive :
                                                Akonadi::CollectionFetchJob::Base;
    Akonadi::CollectionFetchJob *fetch = new Akonadi::CollectionFetchJob(cols, type, this);
    fetch->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::None);
    fetch->fetchScope().setContentMimeTypes(QStringList() << Akonadi::Collection::mimeType()
                                                          << QLatin1String("message/rfc822"));
    fetch->fetchScope().setIncludeStatistics(true);
    return fetch;
}


void SearchCollectionIndexingWarning::setCollection(const Akonadi::Collection &collection)
{
    if (collection == mCollection) {
        return;
    }

    animatedHide();

    mCollection = collection;
    mCollections.clear();

    // Not a search collection?
    if (!collection.hasAttribute<Akonadi::PersistentSearchAttribute>()) {
        return;
    }

    Akonadi::PersistentSearchAttribute *attr = collection.attribute<Akonadi::PersistentSearchAttribute>();
    Akonadi::Collection::List cols;
    Q_FOREACH (qint64 col, attr->queryCollections()) {
        cols.push_back(Akonadi::Collection(col));
    }

    // First retrieve the top-level collections
    Akonadi::CollectionFetchJob *fetch = fetchCollections(cols, false);
    fetch->setProperty("recursiveQuery", attr->isRecursive());
    connect(fetch, SIGNAL(finished(KJob*)), this, SLOT(queryRootCollectionFetchFinished(KJob*)));
}

void SearchCollectionIndexingWarning::queryRootCollectionFetchFinished(KJob* job)
{
    if (job->error()) {
        kWarning() << job->errorString();
        return;
    }

    // Store the root collections
    mCollections = qobject_cast<Akonadi::CollectionFetchJob*>(job)->collections();

    if (job->property("recursiveQuery").toBool()) {
        // Fetch all descendants, if necessary
        Akonadi::CollectionFetchJob *fetch = fetchCollections(mCollections, true);
        connect(fetch, SIGNAL(finished(KJob*)), this, SLOT(queryCollectionFetchFinished(KJob*)));
    } else {
        queryIndexerStatus();
    }
}

void SearchCollectionIndexingWarning::queryCollectionFetchFinished(KJob *job)
{
    if (job->error()) {
        kWarning() << job->errorString();
        return;
    }

    mCollections += qobject_cast<Akonadi::CollectionFetchJob*>(job)->collections();
    queryIndexerStatus();
}

void SearchCollectionIndexingWarning::queryIndexerStatus()
{
    PimCommon::CollectionIndexStatusJob *indexerJob = new PimCommon::CollectionIndexStatusJob(mCollections, this);
    connect(indexerJob, SIGNAL(finished(KJob*)), this, SLOT(indexerStatsFetchFinished(KJob*)));
    indexerJob->start();
}

void SearchCollectionIndexingWarning::indexerStatsFetchFinished(KJob* job)
{
    if (job->error()) {
        kWarning() << job->errorString();
        return;
    }

    bool allFullyIndexed = true;
    QMap<qint64, qint64> stats = qobject_cast<PimCommon::CollectionIndexStatusJob*>(job)->resultStats();
    Q_FOREACH (const Akonadi::Collection &col, mCollections) {
        if (col.hasAttribute<Akonadi::EntityHiddenAttribute>() || !col.cachePolicy().localParts().contains(QLatin1String("RFC822"))) {
            continue;
        }
        kDebug() << "Collection:" << col.displayName() << "(" << col.id() << "), count:" << col.statistics().count() << ", index:" << stats.value(col.id());
        if (col.statistics().count() != stats.value(col.id())) {
            allFullyIndexed = false;
            break;
        }
    }

    if (!allFullyIndexed) {
        animatedShow();
    }
}


/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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

#include "fetchrecursivecollectionsjob.h"
#include <kdebug.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>

using namespace PimCommon;

FetchRecursiveCollectionsJob::FetchRecursiveCollectionsJob(QObject *parent)
    : QObject(parent)
{

}

FetchRecursiveCollectionsJob::~FetchRecursiveCollectionsJob()
{

}

void FetchRecursiveCollectionsJob::start()
{
    if (!mTopCollection.isValid()) {
        kWarning() << "Any collection is defined";
        Q_EMIT fetchCollectionFailed();
        deleteLater();
        return;
    }
    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(mTopCollection, Akonadi::CollectionFetchJob::Recursive);
    job->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotInitialCollectionFetchingDone(KJob*)));
}

void FetchRecursiveCollectionsJob::setTopCollection(const Akonadi::Collection &col)
{
    mTopCollection = col;
}

void FetchRecursiveCollectionsJob::slotInitialCollectionFetchingDone(KJob *job)
{
    if (job->error()) {
        kWarning() << job->errorString();
        Q_EMIT fetchCollectionFailed();
        deleteLater();
        return;
    }
    Akonadi::CollectionFetchJob *fetchJob = qobject_cast<Akonadi::CollectionFetchJob *>(job);
    Q_EMIT fetchCollectionFinished(fetchJob->collections() << mTopCollection);
    deleteLater();
}


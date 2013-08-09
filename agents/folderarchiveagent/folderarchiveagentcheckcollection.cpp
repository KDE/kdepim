/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "folderarchiveagentcheckcollection.h"
#include "folderarchiveaccountinfo.h"

#include <Akonadi/CollectionFetchJob>

FolderArchiveAgentCheckCollection::FolderArchiveAgentCheckCollection(FolderArchiveAccountInfo *info, QObject *parent)
    : QObject(parent),
      mInfo(info)
{
    Akonadi::Collection col(info->archiveTopLevel());
    if (info->keepExistingStructure()) {
        Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(col, Akonadi::CollectionFetchJob::Recursive);
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotInitialCollectionFetchingDone(KJob*)) );
    } else {
        Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(col, Akonadi::CollectionFetchJob::FirstLevel);
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotInitialCollectionFetchingDone(KJob*)) );
    }
}

FolderArchiveAgentCheckCollection::~FolderArchiveAgentCheckCollection()
{

}

void FolderArchiveAgentCheckCollection::slotInitialCollectionFetchingDone(KJob *job)
{
    if ( job->error() ) {
        qWarning() << job->errorString();
        Q_EMIT checkFailed();
        return;
    }
}

#include "folderarchiveagentcheckcollection.moc"

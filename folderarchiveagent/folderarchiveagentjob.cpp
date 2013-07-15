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

#include "folderarchiveagentjob.h"
#include "folderarchiveaccountinfo.h"
#include "folderarchivemanager.h"
#include <Akonadi/ItemMoveJob>
#include <Akonadi/CollectionFetchJob>

FolderArchiveAgentJob::FolderArchiveAgentJob(FolderArchiveManager *manager, FolderArchiveAccountInfo *info, const Akonadi::Item::List &lstItem, QObject *parent)
    : QObject(parent),
      mLstItem(lstItem),
      mManager(manager),
      mInfo(info)
{
}

FolderArchiveAgentJob::~FolderArchiveAgentJob()
{
}

void FolderArchiveAgentJob::start()
{
    Akonadi::CollectionFetchJob *saveMessageJob = new Akonadi::CollectionFetchJob( Akonadi::Collection(mInfo->archiveTopLevel()), Akonadi::CollectionFetchJob::Base );
    connect( saveMessageJob, SIGNAL(result(KJob*)), this, SLOT(slotFetchCollection(KJob*)));
}

void FolderArchiveAgentJob::slotFetchCollection(KJob*)
{
    //TODO
}

#include "folderarchiveagentjob.moc"

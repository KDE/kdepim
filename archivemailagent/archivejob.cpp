/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
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

#include "archivejob.h"
#include "archivemailinfo.h"
#include <mailcommon/backupjob.h>

ArchiveJob::ArchiveJob(ArchiveMailInfo *info, const Akonadi::Collection &folder, bool immediate )
  : MailCommon::ScheduledJob( folder, immediate ),mInfo(info)
{
}

ArchiveJob::~ArchiveJob()
{
  delete mInfo;
}

void ArchiveJob::execute()
{
  if(mInfo) {
    MailCommon::BackupJob *backupJob = new MailCommon::BackupJob();
    backupJob->setRootFolder( Akonadi::Collection(mInfo->saveCollectionId()) );
    backupJob->setSaveLocation( mInfo->url() );//TODO fix me
    backupJob->setArchiveType( mInfo->archiveType() );
    backupJob->setDeleteFoldersAfterCompletion( false );
    backupJob->setRecursive( mInfo->saveSubCollection() );
    connect(backupJob,SIGNAL(backupDone()),this,SLOT(slotBackupDone()));
    backupJob->start();
  }
}

void ArchiveJob::slotBackupDone()
{
  if(mInfo) {
    mInfo->setLastDateSaved(QDate::currentDate());
    //FIXME
    //mInfo->writeConfig();
  }
}

void ArchiveJob::kill()
{
  ScheduledJob::kill();
}

MailCommon::ScheduledJob *ScheduledArchiveTask::run()
{
  return folder().isValid() ? new ArchiveJob( mInfo, folder(), isImmediate() ) : 0;
}


#include "archivejob.moc"

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
#include "archivemailmanager.h"

#include "mailcommon/mailutil.h"

#include <mailcommon/backupjob.h>

#include <KNotification>
#include <KLocale>


ArchiveJob::ArchiveJob(ArchiveMailManager *manager, ArchiveMailInfo *info, const Akonadi::Collection &folder, bool immediate )
  : MailCommon::ScheduledJob( folder, immediate )
  ,mInfo(info)
  ,mManager(manager)
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
    Akonadi::Collection collection(mInfo->saveCollectionId());
    backupJob->setRootFolder( collection );
    backupJob->setSaveLocation( mInfo->realUrl(collection.name()) );
    backupJob->setArchiveType( mInfo->archiveType() );
    backupJob->setDeleteFoldersAfterCompletion( false );
    backupJob->setRecursive( mInfo->saveSubCollection() );
    connect(backupJob,SIGNAL(backupDone()),this,SLOT(slotBackupDone()));
    backupJob->start();
    const QString summary = i18n("Start to archive %1",MailCommon::Util::fullCollectionPath(collection) );
    KNotification::event( "",
                          summary,
                          QPixmap(),
                          0,
                          KNotification::CloseOnTimeout);
  }
}

void ArchiveJob::slotBackupDone()
{
  Akonadi::Collection collection(mInfo->saveCollectionId());
  const QString summary = i18n("Archive done for %1",MailCommon::Util::fullCollectionPath(collection) );
  KNotification::event( "",
                        summary,
                        QPixmap(),
                        0,
                        KNotification::CloseOnTimeout);
  mManager->backupDone(mInfo);
}

void ArchiveJob::kill()
{
  //TODO fix kill program.
  ScheduledJob::kill();
}

MailCommon::ScheduledJob *ScheduledArchiveTask::run()
{
  return folder().isValid() ? new ArchiveJob( mManager, mInfo, folder(), isImmediate() ) : 0;
}


#include "archivejob.moc"

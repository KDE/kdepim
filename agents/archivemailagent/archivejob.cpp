/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "mailcommon/util/mailutil.h"

#include <mailcommon/job/backupjob.h>

#include <KNotification>
#include <KLocale>
#include <KGlobal>
#include <KIcon>
#include <KIconLoader>


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
    if (mInfo) {

        Akonadi::Collection collection(mInfo->saveCollectionId());
        const QString realPath = MailCommon::Util::fullCollectionPath(collection);
        if (realPath.isEmpty()) {
            qDebug()<<" We cannot find real path, collection doesn't exist";
            mManager->collectionDoesntExist(mInfo);
            deleteLater();
            return;
        }
        if (mInfo->url().isEmpty()) {
            qDebug()<<" Path is empty";
            mManager->collectionDoesntExist(mInfo);
            deleteLater();
            return;
        }

        MailCommon::BackupJob *backupJob = new MailCommon::BackupJob();
        backupJob->setRootFolder( MailCommon::Util::updatedCollection(collection) );

        backupJob->setSaveLocation( mInfo->realUrl(realPath) );
        backupJob->setArchiveType( mInfo->archiveType() );
        backupJob->setDeleteFoldersAfterCompletion( false );
        backupJob->setRecursive( mInfo->saveSubCollection() );
        backupJob->setDisplayMessageBox(false);
        backupJob->setRealPath(realPath);
        const QString summary = i18n("Start to archive %1",realPath );
        const QPixmap pixmap = KIcon( QLatin1String("kmail") ).pixmap( KIconLoader::SizeSmall, KIconLoader::SizeSmall );
        KNotification::event( QLatin1String("archivemailstarted"),
                              summary,
                              pixmap,
                              0,
                              KNotification::CloseOnTimeout,
                              KGlobal::mainComponent());
        connect(backupJob, SIGNAL(backupDone(QString)), this, SLOT(slotBackupDone(QString)));
        connect(backupJob, SIGNAL(error(QString)), this, SLOT(slotError(QString)));
        backupJob->start();
    }
}

void ArchiveJob::slotError(const QString &error)
{
    const QPixmap pixmap = KIcon( QLatin1String("kmail") ).pixmap( KIconLoader::SizeSmall, KIconLoader::SizeSmall );

    KNotification::event( QLatin1String("archivemailerror"),
                          error,
                          pixmap,
                          0,
                          KNotification::CloseOnTimeout,
                          KGlobal::mainComponent());
    mManager->backupDone(mInfo);
    deleteLater();
}

void ArchiveJob::slotBackupDone(const QString &info)
{
    const QPixmap pixmap = KIcon( QLatin1String("kmail") ).pixmap( KIconLoader::SizeSmall, KIconLoader::SizeSmall );

    KNotification::event( QLatin1String("archivemailfinished"),
                          info,
                          pixmap,
                          0,
                          KNotification::CloseOnTimeout,
                          KGlobal::mainComponent());
    mManager->backupDone(mInfo);
    deleteLater();
}

void ArchiveJob::kill()
{
    ScheduledJob::kill();
}

MailCommon::ScheduledJob *ScheduledArchiveTask::run()
{
    return folder().isValid() ? new ArchiveJob( mManager, mInfo, folder(), isImmediate() ) : 0;
}



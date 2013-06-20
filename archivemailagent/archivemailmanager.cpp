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

#include "archivemailmanager.h"
#include "archivemailinfo.h"
#include "archivejob.h"
#include "archivemailkernel.h"
#include "archivemailagentutil.h"

#include <mailcommon/kernel/mailkernel.h>
#include <mailcommon/util/mailutil.h>

#include <Akonadi/Collection>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KGlobal>

#include <QDate>
#include <QFile>
#include <QDir>

ArchiveMailManager::ArchiveMailManager(QObject *parent)
    : QObject( parent )
{
    mArchiveMailKernel = new ArchiveMailKernel( this );
    CommonKernel->registerKernelIf( mArchiveMailKernel ); //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf( mArchiveMailKernel ); //SettingsIf is used in FolderTreeWidget
    mConfig = KGlobal::config();
}

ArchiveMailManager::~ArchiveMailManager()
{
    qDeleteAll(mListArchiveInfo);
}

void ArchiveMailManager::slotArchiveNow(ArchiveMailInfo *info)
{
    if (!info)
        return;
    ArchiveMailInfo *stockInfo = new ArchiveMailInfo(*info);
    mListArchiveInfo.append(stockInfo);
    ScheduledArchiveTask *task = new ScheduledArchiveTask( this, stockInfo,Akonadi::Collection(stockInfo->saveCollectionId()), true /*immediat*/ );
    mArchiveMailKernel->jobScheduler()->registerTask( task );
}

void ArchiveMailManager::load()
{
    qDeleteAll(mListArchiveInfo);
    mListArchiveInfo.clear();


    const QStringList collectionList = mConfig->groupList().filter( QRegExp( QLatin1String("ArchiveMailCollection \\d+") ) );
    const int numberOfCollection = collectionList.count();
    for (int i = 0 ; i < numberOfCollection; ++i) {
        KConfigGroup group = mConfig->group(collectionList.at(i));
        ArchiveMailInfo *info = new ArchiveMailInfo(group);

        if (ArchiveMailAgentUtil::needToArchive(info)) {
            Q_FOREACH(ArchiveMailInfo*oldInfo,mListArchiveInfo) {
                if (oldInfo->saveCollectionId() == info->saveCollectionId()) {
                    //already in jobscheduler
                    delete info;
                    info = 0;
                    break;
                }
            }
            if (info) {
                //Store task started
                mListArchiveInfo.append(info);
                ScheduledArchiveTask *task = new ScheduledArchiveTask( this, info,Akonadi::Collection(info->saveCollectionId()), /*immediate*/false );
                mArchiveMailKernel->jobScheduler()->registerTask( task );
            }
        } else {
            delete info;
        }
    }
}

void ArchiveMailManager::removeCollection(const Akonadi::Collection &collection)
{
    const QString groupname = ArchiveMailAgentUtil::archivePattern.arg(collection.id());
    if (mConfig->hasGroup(groupname)) {
        KConfigGroup group = mConfig->group(groupname);
        group.deleteGroup();
        mConfig->sync();
        mConfig->reparseConfiguration();
        Q_FOREACH(ArchiveMailInfo *info, mListArchiveInfo) {
            if (info->saveCollectionId() == collection.id()) {
                mListArchiveInfo.removeAll(info);
            }
        }
    }
}

void ArchiveMailManager::backupDone(ArchiveMailInfo *info)
{
    info->setLastDateSaved(QDate::currentDate());
    const QString groupname = ArchiveMailAgentUtil::archivePattern.arg(info->saveCollectionId());
    //Don't store it if we removed this task
    if (mConfig->hasGroup(groupname)) {
        KConfigGroup group = mConfig->group(groupname);
        info->writeConfig(group);
    }
    Akonadi::Collection collection(info->saveCollectionId());
    const QString realPath = MailCommon::Util::fullCollectionPath(collection);
    const QStringList lst = info->listOfArchive(realPath);

    if (info->maximumArchiveCount() != 0) {
        if (lst.count() > info->maximumArchiveCount()) {
            const int diff = (lst.count() - info->maximumArchiveCount());
            for (int i = 0; i < diff; ++i) {
                const QString fileToRemove(info->url().path() + QDir::separator() + lst.at(i));
                kDebug()<<" file to remove "<<fileToRemove;
                QFile::remove(fileToRemove);
            }
        }
    }
    mListArchiveInfo.removeAll(info);
}

void ArchiveMailManager::pause()
{
    mArchiveMailKernel->jobScheduler()->pause();
}

void ArchiveMailManager::resume()
{
    mArchiveMailKernel->jobScheduler()->resume();
}

void ArchiveMailManager::printArchiveListInfo()
{
    Q_FOREACH (ArchiveMailInfo *info, mListArchiveInfo) {
        kDebug()<<"info: collectionId:"<<info->saveCollectionId()
                <<" saveSubCollection ?"<<info->saveSubCollection()
                <<" lastDateSaved:"<<info->lastDateSaved()
                <<" number of archive:"<<info->maximumArchiveCount()
                <<" directory"<<info->url()
                <<" enabled ?"<<info->isEnabled();
    }
}

#include "archivemailmanager.moc"

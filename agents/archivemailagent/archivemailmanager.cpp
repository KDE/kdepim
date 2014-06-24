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

#include <KConfigGroup>
#include <KSharedConfig>
#include <QDebug>

#include <QDate>
#include <QFile>
#include <QDir>

ArchiveMailManager::ArchiveMailManager(QObject *parent)
    : QObject( parent )
{
    mArchiveMailKernel = new ArchiveMailKernel( this );
    CommonKernel->registerKernelIf( mArchiveMailKernel ); //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf( mArchiveMailKernel ); //SettingsIf is used in FolderTreeWidget
    mConfig = KSharedConfig::openConfig();
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
    removeCollectionId(collection.id());
}

void ArchiveMailManager::removeCollectionId(Akonadi::Collection::Id id)
{
    const QString groupname = ArchiveMailAgentUtil::archivePattern.arg(id);
    if (mConfig->hasGroup(groupname)) {
        KConfigGroup group = mConfig->group(groupname);
        group.deleteGroup();
        mConfig->sync();
        mConfig->reparseConfiguration();
        Q_FOREACH(ArchiveMailInfo *info, mListArchiveInfo) {
            if (info->saveCollectionId() == id) {
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
                qDebug()<<" file to remove "<<fileToRemove;
                QFile::remove(fileToRemove);
            }
        }
    }
    mListArchiveInfo.removeAll(info);

    Q_EMIT needUpdateConfigDialogBox();
}

void ArchiveMailManager::collectionDoesntExist(ArchiveMailInfo *info)
{
    removeCollectionId(info->saveCollectionId());
    mListArchiveInfo.removeAll(info);
    Q_EMIT needUpdateConfigDialogBox();
}

void ArchiveMailManager::pause()
{
    mArchiveMailKernel->jobScheduler()->pause();
}

void ArchiveMailManager::resume()
{
    mArchiveMailKernel->jobScheduler()->resume();
}

QString ArchiveMailManager::printCurrentListInfo()
{
    QString infoStr;
    Q_FOREACH (ArchiveMailInfo *info, mListArchiveInfo) {
        if (!infoStr.isEmpty())
            infoStr += QLatin1Char('\n');
        infoStr += infoToStr(info);
    }
    return infoStr;
}

QString ArchiveMailManager::infoToStr(ArchiveMailInfo *info) const
{
    QString infoStr = QLatin1String("collectionId: ") + QString::number(info->saveCollectionId()) + QLatin1Char('\n');
    infoStr += QLatin1String("save sub collection: ") + (info->saveSubCollection() ? QLatin1String("true") : QLatin1String("false")) + QLatin1Char('\n');
    infoStr += QLatin1String("last Date Saved: ") + info->lastDateSaved().toString() + QLatin1Char('\n');
    infoStr += QLatin1String("maximum achive number: ") + QString::number(info->maximumArchiveCount()) + QLatin1Char('\n');
    infoStr += QLatin1String("directory: ") + info->url().pathOrUrl() + QLatin1Char('\n');
    infoStr += QLatin1String("Enabled: ") + (info->isEnabled() ? QLatin1String("true") : QLatin1String("false"));
    return infoStr;
}

QString ArchiveMailManager::printArchiveListInfo()
{
    QString infoStr;
    const QStringList collectionList = mConfig->groupList().filter( QRegExp( QLatin1String("ArchiveMailCollection \\d+") ) );
    const int numberOfCollection = collectionList.count();
    for (int i = 0 ; i < numberOfCollection; ++i) {
        KConfigGroup group = mConfig->group(collectionList.at(i));
        ArchiveMailInfo info(group);
        if (!infoStr.isEmpty())
            infoStr += QLatin1Char('\n');
        infoStr += infoToStr(&info);
    }
    return infoStr;
}

void ArchiveMailManager::archiveFolder(const QString &path, Akonadi::Collection::Id collectionId)
{
    ArchiveMailInfo *info = new ArchiveMailInfo;
    info->setSaveCollectionId(collectionId);
    info->setUrl(KUrl(path));
    slotArchiveNow(info);
    delete info;
}


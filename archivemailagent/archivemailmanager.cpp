/*
  Copyright (c) 2012-2013 Montel Laurent <montel.org>

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

#include <mailcommon/mailkernel.h>
#include <mailcommon/mailutil.h>

#include <Akonadi/Collection>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KGlobal>

#include <QDate>
#include <QFile>
#include <QDir>

static QString archivePattern = QLatin1String("ArchiveMailCollection %1");

ArchiveMailManager::ArchiveMailManager(QObject *parent)
  : QObject( parent )
{
  mArchiveMailKernel = new ArchiveMailKernel( this );
  CommonKernel->registerKernelIf( mArchiveMailKernel ); //register KernelIf early, it is used by the Filter classes
  CommonKernel->registerSettingsIf( mArchiveMailKernel ); //SettingsIf is used in FolderTreeWidget
}

ArchiveMailManager::~ArchiveMailManager()
{
  qDeleteAll(mListArchiveInfo);
}

void ArchiveMailManager::load()
{
  qDeleteAll(mListArchiveInfo);
  mListArchiveInfo.clear();

  KSharedConfig::Ptr config = KGlobal::config();
  const QStringList collectionList = config->groupList().filter( QRegExp( "ArchiveMailCollection \\d+" ) );
  const int numberOfCollection = collectionList.count();
  for(int i = 0 ; i < numberOfCollection; ++i) {
    KConfigGroup group = config->group(collectionList.at(i));
    ArchiveMailInfo *info = new ArchiveMailInfo(group);

    const QDate diffDate = ArchiveMailAgentUtil::diffDate(info);
    if(QDate::currentDate() >= diffDate) {
      Q_FOREACH(ArchiveMailInfo*oldInfo,mListArchiveInfo) {
        if(oldInfo->saveCollectionId() == info->saveCollectionId()) {
          //already in jobscheduler
          delete info;
          info = 0;
          break;
        }
      }
      if(info) {
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

void ArchiveMailManager::removeCollection(const Akonadi::Collection& collection)
{
  KSharedConfig::Ptr config = KGlobal::config();
  const QString groupname = archivePattern.arg(collection.id());
  if(config->hasGroup(groupname)) {
    KConfigGroup group = config->group(groupname);
    group.deleteGroup();
    config->sync();
    Q_FOREACH(ArchiveMailInfo *info, mListArchiveInfo) {
      if(info->saveCollectionId() == collection.id()) {
        mListArchiveInfo.removeAll(info);
      }
    }
  }
}

void ArchiveMailManager::backupDone(ArchiveMailInfo *info)
{
  info->setLastDateSaved(QDate::currentDate());
  KSharedConfig::Ptr config = KGlobal::config();
  const QString groupname = archivePattern.arg(info->saveCollectionId());
  //Don't store it if we removed this task
  if(config->hasGroup(groupname)) {
    KConfigGroup group = config->group(groupname);
    info->writeConfig(group);
  }
  Akonadi::Collection collection(info->saveCollectionId());
  const QString realPath = MailCommon::Util::fullCollectionPath(collection);
  const QStringList lst = info->listOfArchive(realPath);

  if(info->maximumArchiveCount() != 0) {
    if(lst.count() > info->maximumArchiveCount()) {
      const int diff = (lst.count() - info->maximumArchiveCount());
      for(int i = 0; i < diff; ++i) {
        const QString fileToRemove(info->url().path() + QDir::separator() + lst.at(i));
        qDebug()<<" file to remove "<<fileToRemove;
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


#include "archivemailmanager.moc"

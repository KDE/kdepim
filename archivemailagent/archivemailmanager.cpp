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

#include "archivemailmanager.h"
#include "archivemailinfo.h"
#include "archivejob.h"
#include "archivemailkernel.h"

#include <mailcommon/mailkernel.h>

#include <Akonadi/Collection>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KGlobal>

#include <QDate>

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
  KSharedConfig::Ptr config = KGlobal::config();
  const QStringList collectionList = config->groupList().filter( QRegExp( "ArchiveMailCollection \\d+" ) );
  const int numberOfCollection = collectionList.count();
  for(int i = 0 ; i < numberOfCollection; ++i) {
    KConfigGroup group = config->group(collectionList.at(i));
    ArchiveMailInfo *info = new ArchiveMailInfo(group);

    QDate diffDate(info->lastDateSaved());
    switch(info->archiveUnit()) {
      case ArchiveMailInfo::ArchiveDays:
        diffDate = diffDate.addDays(info->archiveAge());
        break;
      case ArchiveMailInfo::ArchiveWeeks:
        diffDate = diffDate.addDays(info->archiveAge()*7);
        break;
      case ArchiveMailInfo::ArchiveMonths:
        diffDate = diffDate.addMonths(info->archiveAge());
        break;
      default:
        qDebug()<<"archiveUnit not defined :"<<info->archiveUnit();
        break;
    }
    if(QDate::currentDate() > diffDate) {
      //Store task started
      mListArchiveInfo.append(info);
      ScheduledArchiveTask *task = new ScheduledArchiveTask( this, info,Akonadi::Collection(info->saveCollectionId()), /*immediate*/false );
      mArchiveMailKernel->jobScheduler()->registerTask( task );
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
    Q_FOREACH(ArchiveMailInfo *info, mListArchiveInfo) {
      if(info->saveCollectionId() == collection.id()) {
        //TODO stop task
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
  KConfigGroup group = config->group(groupname);
  info->writeConfig(group);
  mListArchiveInfo.removeAll(info);
}

#include "archivemailmanager.moc"

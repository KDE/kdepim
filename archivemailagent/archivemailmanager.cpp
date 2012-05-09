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

#include <KConfigGroup>
#include <KSharedConfig>
#include <KGlobal>

#include <QDate>

ArchiveMailManager::ArchiveMailManager(QObject *parent)
  : QObject( parent )
{
  mArchiveMailKernel = new ArchiveMailKernel( this );
  CommonKernel->registerKernelIf( mArchiveMailKernel ); //register KernelIf early, it is used by the Filter classes
  CommonKernel->registerSettingsIf( mArchiveMailKernel ); //SettingsIf is used in FolderTreeWidget
}

ArchiveMailManager::~ArchiveMailManager()
{
}

void ArchiveMailManager::load()
{
  KSharedConfig::Ptr config = KGlobal::config();
  const QStringList collectionList = config->groupList().filter( QRegExp( "ArchiveMailCollection \\d+" ) );
  const int numberOfCollection = collectionList.count();
  for(int i = 0 ; i < numberOfCollection; ++i) {
    KConfigGroup group = config->group(collectionList.at(i));
    ArchiveMailInfo *info = new ArchiveMailInfo(group);
    if(QDate::currentDate() > (info->lastDateSaved().addDays(info->archiveAge()))) {//TODO use unit
      ScheduledArchiveTask *task = new ScheduledArchiveTask( info,Akonadi::Collection(info->saveCollectionId()), /*immediate*/false );
      mArchiveMailKernel->jobScheduler()->registerTask( task );
    } else {
      delete info;
    }
  }
}

#include "archivemailmanager.moc"

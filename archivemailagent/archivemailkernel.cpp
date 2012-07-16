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

#include "archivemailkernel.h"

#include <KConfigGroup>
#include <kglobal.h>
#include <kpimidentities/identitymanager.h>
#include <mailcommon/foldercollectionmonitor.h>
#include <mailcommon/jobscheduler.h>
#include <messagecomposer/akonadisender.h>
#include <akonadi/session.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/changerecorder.h>

ArchiveMailKernel::ArchiveMailKernel( QObject *parent )
  : QObject( parent )
{
  mIdentityManager = new KPIMIdentities::IdentityManager( false, this );
  mFolderCollectionMonitor = new MailCommon::FolderCollectionMonitor( this );

  Akonadi::Session *session = new Akonadi::Session( "Archive Mail Kernel ETM", this );
  folderCollectionMonitor()->setSession( session );
  mEntityTreeModel = new Akonadi::EntityTreeModel( folderCollectionMonitor(), this );
  mEntityTreeModel->setIncludeUnsubscribed( false );
  mEntityTreeModel->setItemPopulationStrategy( Akonadi::EntityTreeModel::LazyPopulation );

  mCollectionModel = new Akonadi::EntityMimeTypeFilterModel( this );
  mCollectionModel->setSourceModel( mEntityTreeModel );
  mCollectionModel->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
  mCollectionModel->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );
  mCollectionModel->setDynamicSortFilter( true );
  mCollectionModel->setSortCaseSensitivity( Qt::CaseInsensitive );
  mJobScheduler = new MailCommon::JobScheduler(this);
}

KPIMIdentities::IdentityManager *ArchiveMailKernel::identityManager()
{
  return mIdentityManager;
}

MessageSender *ArchiveMailKernel::msgSender()
{
  return 0;
}

Akonadi::EntityMimeTypeFilterModel *ArchiveMailKernel::collectionModel() const
{
  return mCollectionModel;
}

KSharedConfig::Ptr ArchiveMailKernel::config()
{
  return KGlobal::config();
}

void ArchiveMailKernel::syncConfig()
{
  Q_ASSERT( false );
}

MailCommon::JobScheduler* ArchiveMailKernel::jobScheduler() const
{
  return mJobScheduler;
}

Akonadi::ChangeRecorder *ArchiveMailKernel::folderCollectionMonitor() const
{
  return mFolderCollectionMonitor->monitor();
}

void ArchiveMailKernel::updateSystemTray()
{
  Q_ASSERT( false );
}

bool ArchiveMailKernel::showPopupAfterDnD()
{
  return false;
}

qreal ArchiveMailKernel::closeToQuotaThreshold()
{
  return 80;
}

QStringList ArchiveMailKernel::customTemplates()
{
  Q_ASSERT( false );
  return QStringList();
}

bool ArchiveMailKernel::excludeImportantMailFromExpiry()
{
  Q_ASSERT( false );
  return true;
}

Akonadi::Entity::Id ArchiveMailKernel::lastSelectedFolder()
{
  return Akonadi::Entity::Id();
}

void ArchiveMailKernel::setLastSelectedFolder(const Akonadi::Entity::Id& col)
{
}



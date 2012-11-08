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

#include "backupmailkernel.h"

#include <kglobal.h>
#include <kpimidentities/identitymanager.h>
#include <messagecomposer/akonadisender.h>
#include <mailcommon/foldercollectionmonitor.h>
#include <akonadi/session.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/changerecorder.h>

BackupMailKernel::BackupMailKernel( QObject *parent )
  : QObject( parent )
{
  mMessageSender = new AkonadiSender( this );
  mIdentityManager = new KPIMIdentities::IdentityManager( false, this );
  mFolderCollectionMonitor = new MailCommon::FolderCollectionMonitor( this );

  Akonadi::Session *session = new Akonadi::Session( "Backup Mail Kernel ETM", this );
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
}

KPIMIdentities::IdentityManager *BackupMailKernel::identityManager()
{
  return mIdentityManager;
}

MessageSender *BackupMailKernel::msgSender()
{
  return mMessageSender;
}

Akonadi::EntityMimeTypeFilterModel *BackupMailKernel::collectionModel() const
{
  return mCollectionModel;
}

KSharedConfig::Ptr BackupMailKernel::config()
{
  return KGlobal::config();
}

void BackupMailKernel::syncConfig()
{
  Q_ASSERT( false );
}

MailCommon::JobScheduler* BackupMailKernel::jobScheduler() const
{
  Q_ASSERT( false );
  return 0;
}

Akonadi::ChangeRecorder *BackupMailKernel::folderCollectionMonitor() const
{
  return mFolderCollectionMonitor->monitor();
}

void BackupMailKernel::updateSystemTray()
{
  Q_ASSERT( false );
}

bool BackupMailKernel::showPopupAfterDnD()
{
  return false;
}

qreal BackupMailKernel::closeToQuotaThreshold()
{
  return 80;
}

QStringList BackupMailKernel::customTemplates()
{
  Q_ASSERT( false );
  return QStringList();
}

bool BackupMailKernel::excludeImportantMailFromExpiry()
{
  Q_ASSERT( false );
  return true;
}

Akonadi::Entity::Id BackupMailKernel::lastSelectedFolder()
{
  Q_ASSERT( false );
  return Akonadi::Entity::Id();
}

void BackupMailKernel::setLastSelectedFolder(const Akonadi::Entity::Id& col)
{
  Q_UNUSED(col);
}





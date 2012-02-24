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

#include "importwizardkernel.h"

#include <kglobal.h>
#include <kpimidentities/identitymanager.h>
#include <messagecomposer/akonadisender.h>
#include <mailcommon/foldercollectionmonitor.h>
#include <akonadi/session.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/changerecorder.h>

ImportWizardKernel::ImportWizardKernel( QObject *parent )
  : QObject( parent )
{
  mMessageSender = new AkonadiSender( this );
  mIdentityManager = new KPIMIdentities::IdentityManager( false, this );
  mFolderCollectionMonitor = new MailCommon::FolderCollectionMonitor( this );

  Akonadi::Session *session = new Akonadi::Session( "MailFilter Kernel ETM", this );
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

KPIMIdentities::IdentityManager *ImportWizardKernel::identityManager()
{
  return mIdentityManager;
}

MessageSender *ImportWizardKernel::msgSender()
{
  return mMessageSender;
}

Akonadi::EntityMimeTypeFilterModel *ImportWizardKernel::collectionModel() const
{
  return mCollectionModel;
}

KSharedConfig::Ptr ImportWizardKernel::config()
{
  return KGlobal::config();
}

void ImportWizardKernel::syncConfig()
{
  Q_ASSERT( false );
}

MailCommon::JobScheduler* ImportWizardKernel::jobScheduler() const
{
  Q_ASSERT( false );
  return 0;
}

Akonadi::ChangeRecorder *ImportWizardKernel::folderCollectionMonitor() const
{
  return mFolderCollectionMonitor->monitor();
}

void ImportWizardKernel::updateSystemTray()
{
  Q_ASSERT( false );
}

bool ImportWizardKernel::showPopupAfterDnD()
{
  return false;
}

qreal ImportWizardKernel::closeToQuotaThreshold()
{
  return 80;
}

QStringList ImportWizardKernel::customTemplates()
{
  Q_ASSERT( false );
  return QStringList();
}

bool ImportWizardKernel::excludeImportantMailFromExpiry()
{
  Q_ASSERT( false );
  return true;
}

Akonadi::Entity::Id ImportWizardKernel::lastSelectedFolder()
{
  Q_ASSERT( false );
  return Akonadi::Entity::Id();
}

void ImportWizardKernel::setLastSelectedFolder(const Akonadi::Entity::Id& col)
{
  Q_UNUSED(col);
}





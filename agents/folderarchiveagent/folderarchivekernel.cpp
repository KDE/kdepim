/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "folderarchivekernel.h"

#include <kglobal.h>
#include <kpimidentities/identitymanager.h>
#include <mailcommon/folder/foldercollectionmonitor.h>
#include <akonadi/session.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/changerecorder.h>

FolderArchiveKernel::FolderArchiveKernel( QObject *parent )
    : QObject( parent )
{
    mIdentityManager = new KPIMIdentities::IdentityManager( false, this );
    Akonadi::Session *session = new Akonadi::Session( "Folder Archive Kernel ETM", this );
    mFolderCollectionMonitor = new MailCommon::FolderCollectionMonitor( session, this );

    mFolderCollectionMonitor->monitor()->setChangeRecordingEnabled(false);

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

KPIMIdentities::IdentityManager *FolderArchiveKernel::identityManager()
{
    return mIdentityManager;
}

MessageComposer::MessageSender *FolderArchiveKernel::msgSender()
{
    return 0;
}

Akonadi::EntityMimeTypeFilterModel *FolderArchiveKernel::collectionModel() const
{
    return mCollectionModel;
}

KSharedConfig::Ptr FolderArchiveKernel::config()
{
    return KGlobal::config();
}

void FolderArchiveKernel::syncConfig()
{
    Q_ASSERT( false );
}

MailCommon::JobScheduler* FolderArchiveKernel::jobScheduler() const
{
    return 0;
}

Akonadi::ChangeRecorder *FolderArchiveKernel::folderCollectionMonitor() const
{
    return mFolderCollectionMonitor->monitor();
}

void FolderArchiveKernel::updateSystemTray()
{
    Q_ASSERT( false );
}

bool FolderArchiveKernel::showPopupAfterDnD()
{
    return false;
}

qreal FolderArchiveKernel::closeToQuotaThreshold()
{
    return 80;
}

QStringList FolderArchiveKernel::customTemplates()
{
    Q_ASSERT( false );
    return QStringList();
}

bool FolderArchiveKernel::excludeImportantMailFromExpiry()
{
    Q_ASSERT( false );
    return true;
}

Akonadi::Entity::Id FolderArchiveKernel::lastSelectedFolder()
{
    return Akonadi::Entity::Id();
}

void FolderArchiveKernel::setLastSelectedFolder(const Akonadi::Entity::Id& col)
{
    Q_UNUSED( col );
}



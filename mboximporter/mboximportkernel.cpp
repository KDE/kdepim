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

#include "mboximportkernel.h"

#include <KPIMIdentities/kpimidentities/identitymanager.h>
#include <mailcommon/folder/foldercollectionmonitor.h>
#include <AkonadiCore/session.h>
#include <entitytreemodel.h>
#include <entitymimetypefiltermodel.h>
#include <changerecorder.h>
#include <KSharedConfig>

MBoxImporterKernel::MBoxImporterKernel( QObject *parent )
    : QObject( parent )
{
    mIdentityManager = new KPIMIdentities::IdentityManager( false, this );
    Akonadi::Session *session = new Akonadi::Session( "MBox importer Kernel ETM", this );
    mFolderCollectionMonitor = new MailCommon::FolderCollectionMonitor( session, this );

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

KPIMIdentities::IdentityManager *MBoxImporterKernel::identityManager()
{
    return mIdentityManager;
}

MessageComposer::MessageSender *MBoxImporterKernel::msgSender()
{
    return 0;
}

Akonadi::EntityMimeTypeFilterModel *MBoxImporterKernel::collectionModel() const
{
    return mCollectionModel;
}

KSharedConfig::Ptr MBoxImporterKernel::config()
{
    return KSharedConfig::openConfig();
}

void MBoxImporterKernel::syncConfig()
{
    Q_ASSERT( false );
}

MailCommon::JobScheduler* MBoxImporterKernel::jobScheduler() const
{
    Q_ASSERT( false );
    return 0;
}

Akonadi::ChangeRecorder *MBoxImporterKernel::folderCollectionMonitor() const
{
    return mFolderCollectionMonitor->monitor();
}

void MBoxImporterKernel::updateSystemTray()
{
    Q_ASSERT( false );
}

bool MBoxImporterKernel::showPopupAfterDnD()
{
    return false;
}

qreal MBoxImporterKernel::closeToQuotaThreshold()
{
    return 80;
}

QStringList MBoxImporterKernel::customTemplates()
{
    Q_ASSERT( false );
    return QStringList();
}

bool MBoxImporterKernel::excludeImportantMailFromExpiry()
{
    Q_ASSERT( false );
    return true;
}

Akonadi::Entity::Id MBoxImporterKernel::lastSelectedFolder()
{
    Q_ASSERT( false );
    return Akonadi::Entity::Id();
}

void MBoxImporterKernel::setLastSelectedFolder(const Akonadi::Entity::Id &col)
{
    Q_UNUSED(col);
}

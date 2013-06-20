/*
  Copyright (c) 2012, 2013 Montel Laurent <montel@kde.org>

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

#include "kmailcvtkernel.h"

#include <KConfigGroup>
#include <kglobal.h>
#include <kpimidentities/identitymanager.h>
#include <mailcommon/folder/foldercollectionmonitor.h>
#include <messagecomposer/sender/akonadisender.h>
#include <akonadi/session.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/changerecorder.h>

KMailCVTKernel::KMailCVTKernel( QObject *parent )
    : QObject( parent )
{
    mIdentityManager = new KPIMIdentities::IdentityManager( false, this );
    Akonadi::Session *session = new Akonadi::Session( "KMailCVT Kernel ETM", this );
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

KPIMIdentities::IdentityManager *KMailCVTKernel::identityManager()
{
    return mIdentityManager;
}

MessageComposer::MessageSender *KMailCVTKernel::msgSender()
{
    return 0;
}

Akonadi::EntityMimeTypeFilterModel *KMailCVTKernel::collectionModel() const
{
    return mCollectionModel;
}

KSharedConfig::Ptr KMailCVTKernel::config()
{
    return KGlobal::config();
}

void KMailCVTKernel::syncConfig()
{
    Q_ASSERT( false );
}

MailCommon::JobScheduler* KMailCVTKernel::jobScheduler() const
{
    Q_ASSERT( false );
    return 0;
}

Akonadi::ChangeRecorder *KMailCVTKernel::folderCollectionMonitor() const
{
    return mFolderCollectionMonitor->monitor();
}

void KMailCVTKernel::updateSystemTray()
{
    Q_ASSERT( false );
}

bool KMailCVTKernel::showPopupAfterDnD()
{
    return false;
}

qreal KMailCVTKernel::closeToQuotaThreshold()
{
    return 80;
}

QStringList KMailCVTKernel::customTemplates()
{
    Q_ASSERT( false );
    return QStringList();
}

bool KMailCVTKernel::excludeImportantMailFromExpiry()
{
    Q_ASSERT( false );
    return true;
}

Akonadi::Entity::Id KMailCVTKernel::lastSelectedFolder()
{
    return Akonadi::Entity::Id();
}

void KMailCVTKernel::setLastSelectedFolder(const Akonadi::Entity::Id& col)
{
    Q_UNUSED( col );
}



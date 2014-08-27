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

#include "filtertestkernel.h"

#include <kglobal.h>
#include <KIdentityManagement/kidentitymanagement/identitymanager.h>
#include <messagecomposer/sender/akonadisender.h>
#include <mailcommon/folder/foldercollectionmonitor.h>
#include <AkonadiCore/session.h>
#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/entitymimetypefiltermodel.h>
#include <AkonadiCore/changerecorder.h>
#include <KSharedConfig>

FilterTestKernel::FilterTestKernel( QObject *parent )
    : QObject( parent )
{
    mMessageSender = new MessageComposer::AkonadiSender( this );
    mIdentityManager = new KIdentityManagement::IdentityManager( false, this );
    Akonadi::Session *session = new Akonadi::Session( "Filter Kernel ETM", this );
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

KIdentityManagement::IdentityManager *FilterTestKernel::identityManager()
{
    return mIdentityManager;
}

MessageComposer::MessageSender *FilterTestKernel::msgSender()
{
    return mMessageSender;
}

Akonadi::EntityMimeTypeFilterModel *FilterTestKernel::collectionModel() const
{
    return mCollectionModel;
}

KSharedConfig::Ptr FilterTestKernel::config()
{
    return KSharedConfig::openConfig();
}

void FilterTestKernel::syncConfig()
{
    Q_ASSERT( false );
}

MailCommon::JobScheduler* FilterTestKernel::jobScheduler() const
{
    Q_ASSERT( false );
    return 0;
}

Akonadi::ChangeRecorder *FilterTestKernel::folderCollectionMonitor() const
{
    return mFolderCollectionMonitor->monitor();
}

void FilterTestKernel::updateSystemTray()
{
    Q_ASSERT( false );
}

bool FilterTestKernel::showPopupAfterDnD()
{
    return false;
}

qreal FilterTestKernel::closeToQuotaThreshold()
{
    return 80;
}

QStringList FilterTestKernel::customTemplates()
{
    Q_ASSERT( false );
    return QStringList();
}

bool FilterTestKernel::excludeImportantMailFromExpiry()
{
    Q_ASSERT( false );
    return true;
}

Akonadi::Entity::Id FilterTestKernel::lastSelectedFolder()
{
    Q_ASSERT( false );
    return Akonadi::Entity::Id();
}

void FilterTestKernel::setLastSelectedFolder(const Akonadi::Entity::Id& col)
{
    Q_UNUSED(col);
}





/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "pimsettingexporterkernel.h"

#include <kglobal.h>
#include <kpimidentities/identitymanager.h>
#include <messagecomposer/akonadisender.h>
#include <mailcommon/folder/foldercollectionmonitor.h>
#include <akonadi/session.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/changerecorder.h>

PimSettingExporterKernel::PimSettingExporterKernel( QObject *parent )
    : QObject( parent )
{
    mMessageSender = new MessageComposer::AkonadiSender( this );
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

KPIMIdentities::IdentityManager *PimSettingExporterKernel::identityManager()
{
    return mIdentityManager;
}

MessageComposer::MessageSender *PimSettingExporterKernel::msgSender()
{
    return mMessageSender;
}

Akonadi::EntityMimeTypeFilterModel *PimSettingExporterKernel::collectionModel() const
{
    return mCollectionModel;
}

KSharedConfig::Ptr PimSettingExporterKernel::config()
{
    return KGlobal::config();
}

void PimSettingExporterKernel::syncConfig()
{
    Q_ASSERT( false );
}

MailCommon::JobScheduler* PimSettingExporterKernel::jobScheduler() const
{
    Q_ASSERT( false );
    return 0;
}

Akonadi::ChangeRecorder *PimSettingExporterKernel::folderCollectionMonitor() const
{
    return mFolderCollectionMonitor->monitor();
}

void PimSettingExporterKernel::updateSystemTray()
{
    Q_ASSERT( false );
}

bool PimSettingExporterKernel::showPopupAfterDnD()
{
    return false;
}

qreal PimSettingExporterKernel::closeToQuotaThreshold()
{
    return 80;
}

QStringList PimSettingExporterKernel::customTemplates()
{
    Q_ASSERT( false );
    return QStringList();
}

bool PimSettingExporterKernel::excludeImportantMailFromExpiry()
{
    Q_ASSERT( false );
    return true;
}

Akonadi::Entity::Id PimSettingExporterKernel::lastSelectedFolder()
{
    Q_ASSERT( false );
    return Akonadi::Entity::Id();
}

void PimSettingExporterKernel::setLastSelectedFolder(const Akonadi::Entity::Id& col)
{
    Q_UNUSED(col);
}





/*
   Copyright (C) 2013-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "mboximportkernel.h"

#include <KIdentityManagement/kidentitymanagement/identitymanager.h>
#include <MailCommon/FolderCollectionMonitor>
#include <AkonadiCore/session.h>
#include <entitytreemodel.h>
#include <entitymimetypefiltermodel.h>
#include <changerecorder.h>
#include <KSharedConfig>

MBoxImporterKernel::MBoxImporterKernel(QObject *parent)
    : QObject(parent)
{
    mIdentityManager = new KIdentityManagement::IdentityManager(false, this);
    Akonadi::Session *session = new Akonadi::Session("MBox importer Kernel ETM", this);
    mFolderCollectionMonitor = new MailCommon::FolderCollectionMonitor(session, this);

    mEntityTreeModel = new Akonadi::EntityTreeModel(folderCollectionMonitor(), this);
    mEntityTreeModel->setListFilter(Akonadi::CollectionFetchScope::Enabled);
    mEntityTreeModel->setItemPopulationStrategy(Akonadi::EntityTreeModel::LazyPopulation);

    mCollectionModel = new Akonadi::EntityMimeTypeFilterModel(this);
    mCollectionModel->setSourceModel(mEntityTreeModel);
    mCollectionModel->addMimeTypeInclusionFilter(Akonadi::Collection::mimeType());
    mCollectionModel->setHeaderGroup(Akonadi::EntityTreeModel::CollectionTreeHeaders);
    mCollectionModel->setDynamicSortFilter(true);
    mCollectionModel->setSortCaseSensitivity(Qt::CaseInsensitive);
}

KIdentityManagement::IdentityManager *MBoxImporterKernel::identityManager()
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
    Q_ASSERT(false);
}

MailCommon::JobScheduler *MBoxImporterKernel::jobScheduler() const
{
    Q_ASSERT(false);
    return 0;
}

Akonadi::ChangeRecorder *MBoxImporterKernel::folderCollectionMonitor() const
{
    return mFolderCollectionMonitor->monitor();
}

void MBoxImporterKernel::updateSystemTray()
{
    Q_ASSERT(false);
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
    Q_ASSERT(false);
    return QStringList();
}

bool MBoxImporterKernel::excludeImportantMailFromExpiry()
{
    Q_ASSERT(false);
    return true;
}

Akonadi::Collection::Id MBoxImporterKernel::lastSelectedFolder()
{
    Q_ASSERT(false);
    return Akonadi::Collection::Id();
}

void MBoxImporterKernel::setLastSelectedFolder(Akonadi::Collection::Id col)
{
    Q_UNUSED(col);
}

void MBoxImporterKernel::expunge(Akonadi::Collection::Id col, bool sync)
{
    Q_UNUSED(col);
    Q_UNUSED(sync);
}

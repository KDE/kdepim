/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#ifndef FOLDERARCHIVEKERNEL_H
#define FOLDERARCHIVEKERNEL_H

#include <mailcommon/interfaces/mailinterfaces.h>

namespace Akonadi {
class EntityTreeModel;
class EntityMimeTypeFilterModel;
}

namespace MailCommon {
class FolderCollectionMonitor;
}

class FolderArchiveKernel : public QObject, public MailCommon::IKernel, public MailCommon::ISettings
{
public:
    explicit  FolderArchiveKernel( QObject *parent = 0 );

    KPIMIdentities::IdentityManager *identityManager();
    MessageComposer::MessageSender *msgSender();

    Akonadi::EntityMimeTypeFilterModel *collectionModel() const;
    KSharedConfig::Ptr config();
    void syncConfig();
    MailCommon::JobScheduler* jobScheduler() const;
    Akonadi::ChangeRecorder *folderCollectionMonitor() const;
    void updateSystemTray();

    qreal closeToQuotaThreshold();
    bool excludeImportantMailFromExpiry();
    QStringList customTemplates();
    Akonadi::Entity::Id lastSelectedFolder();
    void setLastSelectedFolder(const Akonadi::Entity::Id& col);
    bool showPopupAfterDnD();
    

private:
    KPIMIdentities::IdentityManager *mIdentityManager;
    MailCommon::FolderCollectionMonitor *mFolderCollectionMonitor;
    Akonadi::EntityTreeModel *mEntityTreeModel;
    Akonadi::EntityMimeTypeFilterModel *mCollectionModel;
};

#endif

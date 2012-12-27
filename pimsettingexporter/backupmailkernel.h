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
#ifndef BACKUPMAILKERNEL_H
#define BACKUPMAILKERNEL_H

#include <mailcommon/mailinterfaces.h>

namespace Akonadi {
class EntityTreeModel;
class EntityMimeTypeFilterModel;
}

namespace MailCommon {
class FolderCollectionMonitor;
}

class BackupMailKernel : public QObject, public MailCommon::IKernel, public MailCommon::ISettings
{
  public:
    explicit BackupMailKernel( QObject *parent = 0 );

    virtual KPIMIdentities::IdentityManager *identityManager();
    virtual MessageSender *msgSender();

    virtual Akonadi::EntityMimeTypeFilterModel *collectionModel() const;
    virtual KSharedConfig::Ptr config();
    virtual void syncConfig();
    virtual MailCommon::JobScheduler* jobScheduler() const;
    virtual Akonadi::ChangeRecorder *folderCollectionMonitor() const;
    virtual void updateSystemTray();

    virtual qreal closeToQuotaThreshold();
    virtual bool excludeImportantMailFromExpiry();
    virtual QStringList customTemplates();
    virtual Akonadi::Entity::Id lastSelectedFolder();
    virtual void setLastSelectedFolder(const Akonadi::Entity::Id& col);
    virtual bool showPopupAfterDnD();
    

  private:
    KPIMIdentities::IdentityManager *mIdentityManager;
    MessageSender *mMessageSender;
    MailCommon::FolderCollectionMonitor *mFolderCollectionMonitor;
    Akonadi::EntityTreeModel *mEntityTreeModel;
    Akonadi::EntityMimeTypeFilterModel *mCollectionModel;
};

#endif

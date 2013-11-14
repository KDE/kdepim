/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef MOBILEKERNEL_H
#define MOBILEKERNEL_H

#include "mailcommon/interfaces/mailinterfaces.h"

#include <KSharedConfig>

namespace MessageComposer {
class AkonadiSender;
}
namespace KPIMIdentities {
class IdentityManager;
}

namespace Akonadi {
class ChangeRecorder;
}

namespace MailCommon {
  class JobScheduler;
}

namespace PimCommon {
  class ComposerAutoCorrection;
}

class MobileKernel : public MailCommon::IKernel, public MailCommon::ISettings, public MailCommon::IFilter
{

public:
    static MobileKernel *self();

    void setFolderCollectionMonitor( Akonadi::ChangeRecorder* monitor ) { mMonitor = monitor; }
    void setCollectionModel( Akonadi::EntityMimeTypeFilterModel *collectionModel ) { mCollectionModel = collectionModel; }
    PimCommon::ComposerAutoCorrection* composerAutoCorrection() const;

//IKernel methods:
    /*reimp*/ Akonadi::ChangeRecorder* folderCollectionMonitor() const { return mMonitor; }
    /*reimp*/ MailCommon::JobScheduler* jobScheduler() const { return mJobScheduler; }
    /*reimp*/ KSharedConfig::Ptr config();
    /*reimp*/ void syncConfig();
    /*reimp*/ KPIMIdentities::IdentityManager* identityManager();
    /*reimp*/ Akonadi::EntityMimeTypeFilterModel* collectionModel() const { return mCollectionModel; }
    /*reimp*/ MessageComposer::MessageSender* msgSender();

//ISettings methods:
    /*reimp*/ void updateSystemTray();
    /*reimp*/ void setLastSelectedFolder(const Akonadi::Entity::Id& col);
    /*reimp*/ Akonadi::Entity::Id lastSelectedFolder();
    /*reimp*/ qreal closeToQuotaThreshold();
    /*reimp*/ bool excludeImportantMailFromExpiry();
    /*reimp*/ bool showPopupAfterDnD();
    /*reimp*/ QStringList customTemplates();

//IFilter methods:
    virtual void createFilter(const QByteArray& field, const QString& value);
    virtual void openFilterDialog( bool createDummyFilter = true );

    /*reimp*/ ~MobileKernel();

private:
    MobileKernel();

    MailCommon::JobScheduler *mJobScheduler;
    Akonadi::ChangeRecorder *mMonitor;
    KPIMIdentities::IdentityManager *mIdentityManager;
    Akonadi::EntityMimeTypeFilterModel *mCollectionModel;
    MessageComposer::AkonadiSender *mMessageSender;
    KSharedConfig::Ptr mConfig;
    PimCommon::ComposerAutoCorrection *mAutoCorrection;
};

#endif // MOBILEKERNEL_H

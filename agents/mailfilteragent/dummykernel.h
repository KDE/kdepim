#ifndef DUMMYKERNEL_H
#define DUMMYKERNEL_H

#include <mailcommon/interfaces/mailinterfaces.h>

namespace Akonadi {
class EntityTreeModel;
class EntityMimeTypeFilterModel;
}

namespace MailCommon {
class FolderCollectionMonitor;
}

class DummyKernel : public QObject, public MailCommon::IKernel, public MailCommon::ISettings
{
public:
    explicit DummyKernel( QObject *parent = 0 );

    virtual KIdentityManagement::IdentityManager *identityManager();
    virtual MessageComposer::MessageSender *msgSender();

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
    KIdentityManagement::IdentityManager *mIdentityManager;
    MessageComposer::MessageSender *mMessageSender;
    MailCommon::FolderCollectionMonitor *mFolderCollectionMonitor;
    Akonadi::EntityTreeModel *mEntityTreeModel;
    Akonadi::EntityMimeTypeFilterModel *mCollectionModel;
};

#endif

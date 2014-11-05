#ifndef DUMMYKERNEL_H
#define DUMMYKERNEL_H

#include <mailcommon/interfaces/mailinterfaces.h>

namespace Akonadi
{
class EntityTreeModel;
class EntityMimeTypeFilterModel;
}

namespace MailCommon
{
class FolderCollectionMonitor;
}

class DummyKernel : public QObject, public MailCommon::IKernel, public MailCommon::ISettings
{
public:
    explicit DummyKernel(QObject *parent = 0);

    virtual KIdentityManagement::IdentityManager *identityManager() Q_DECL_OVERRIDE;
    virtual MessageComposer::MessageSender *msgSender() Q_DECL_OVERRIDE;

    virtual Akonadi::EntityMimeTypeFilterModel *collectionModel() const Q_DECL_OVERRIDE;
    virtual KSharedConfig::Ptr config() Q_DECL_OVERRIDE;
    virtual void syncConfig() Q_DECL_OVERRIDE;
    virtual MailCommon::JobScheduler *jobScheduler() const Q_DECL_OVERRIDE;
    virtual Akonadi::ChangeRecorder *folderCollectionMonitor() const Q_DECL_OVERRIDE;
    virtual void updateSystemTray() Q_DECL_OVERRIDE;

    virtual qreal closeToQuotaThreshold() Q_DECL_OVERRIDE;
    virtual bool excludeImportantMailFromExpiry() Q_DECL_OVERRIDE;
    virtual QStringList customTemplates() Q_DECL_OVERRIDE;
    virtual Akonadi::Entity::Id lastSelectedFolder() Q_DECL_OVERRIDE;
    virtual void setLastSelectedFolder(const Akonadi::Entity::Id &col) Q_DECL_OVERRIDE;
    virtual bool showPopupAfterDnD() Q_DECL_OVERRIDE;

private:
    KIdentityManagement::IdentityManager *mIdentityManager;
    MessageComposer::MessageSender *mMessageSender;
    MailCommon::FolderCollectionMonitor *mFolderCollectionMonitor;
    Akonadi::EntityTreeModel *mEntityTreeModel;
    Akonadi::EntityMimeTypeFilterModel *mCollectionModel;
};

#endif

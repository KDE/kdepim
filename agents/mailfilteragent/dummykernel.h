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

    KIdentityManagement::IdentityManager *identityManager() Q_DECL_OVERRIDE;
    MessageComposer::MessageSender *msgSender() Q_DECL_OVERRIDE;

    Akonadi::EntityMimeTypeFilterModel *collectionModel() const Q_DECL_OVERRIDE;
    KSharedConfig::Ptr config() Q_DECL_OVERRIDE;
    void syncConfig() Q_DECL_OVERRIDE;
    MailCommon::JobScheduler *jobScheduler() const Q_DECL_OVERRIDE;
    Akonadi::ChangeRecorder *folderCollectionMonitor() const Q_DECL_OVERRIDE;
    void updateSystemTray() Q_DECL_OVERRIDE;

    qreal closeToQuotaThreshold() Q_DECL_OVERRIDE;
    bool excludeImportantMailFromExpiry() Q_DECL_OVERRIDE;
    QStringList customTemplates() Q_DECL_OVERRIDE;
    Akonadi::Entity::Id lastSelectedFolder() Q_DECL_OVERRIDE;
    void setLastSelectedFolder(const Akonadi::Entity::Id &col) Q_DECL_OVERRIDE;
    bool showPopupAfterDnD() Q_DECL_OVERRIDE;

private:
    KIdentityManagement::IdentityManager *mIdentityManager;
    MessageComposer::MessageSender *mMessageSender;
    MailCommon::FolderCollectionMonitor *mFolderCollectionMonitor;
    Akonadi::EntityTreeModel *mEntityTreeModel;
    Akonadi::EntityMimeTypeFilterModel *mCollectionModel;
};

#endif

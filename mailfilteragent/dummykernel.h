#ifndef DUMMYKERNEL_H
#define DUMMYKERNEL_H

#include <mailcommon/mailinterfaces.h>

class DummyKernel : public QObject, public MailCommon::IKernel
{
  public:
    DummyKernel( QObject *parent = 0 );

    virtual KPIMIdentities::IdentityManager *identityManager();
    virtual MessageSender *msgSender();

    virtual Akonadi::EntityMimeTypeFilterModel *collectionModel() const;
    virtual KSharedConfig::Ptr config();
    virtual void syncConfig();
    virtual MailCommon::JobScheduler* jobScheduler() const;
    virtual Akonadi::ChangeRecorder *folderCollectionMonitor() const;
    virtual void updateSystemTray();

  private:
    KPIMIdentities::IdentityManager *mIdentityManager;
    MessageSender *mMessageSender;
};

#endif

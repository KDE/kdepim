#include "dummykernel.h"

#include <kglobal.h>
#include <kpimidentities/identitymanager.h>
#include <messagecomposer/akonadisender.h>

DummyKernel::DummyKernel( QObject *parent )
  : QObject( parent )
{
  mMessageSender = new AkonadiSender( this );
  mIdentityManager = new KPIMIdentities::IdentityManager( false, this );
}

KPIMIdentities::IdentityManager *DummyKernel::identityManager()
{
  return mIdentityManager;
}

MessageSender *DummyKernel::msgSender()
{
  return mMessageSender;
}

Akonadi::EntityMimeTypeFilterModel *DummyKernel::collectionModel() const
{
  Q_ASSERT( false );
  return 0;
}

KSharedConfig::Ptr DummyKernel::config()
{
  return KGlobal::config();
}

void DummyKernel::syncConfig()
{
  Q_ASSERT( false );
}

MailCommon::JobScheduler* DummyKernel::jobScheduler() const
{
  Q_ASSERT( false );
  return 0;
}

Akonadi::ChangeRecorder *DummyKernel::folderCollectionMonitor() const
{
  Q_ASSERT( false );
  return 0;
}

void DummyKernel::updateSystemTray()
{
  Q_ASSERT( false );
}

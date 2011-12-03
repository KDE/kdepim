#include "dummykernel.h"

#include <kglobal.h>
#include <kpimidentities/identitymanager.h>
#include <messagecomposer/akonadisender.h>
#include <mailcommon/foldercollectionmonitor.h>
#include <akonadi/session.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/changerecorder.h>

DummyKernel::DummyKernel( QObject *parent )
  : QObject( parent )
{
  mMessageSender = new AkonadiSender( this );
  mIdentityManager = new KPIMIdentities::IdentityManager( false, this );
  mFolderCollectionMonitor = new MailCommon::FolderCollectionMonitor( this );

  Akonadi::Session *session = new Akonadi::Session( "MailFilter Kernel ETM", this );
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
  return mCollectionModel;
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
  return mFolderCollectionMonitor->monitor();
}

void DummyKernel::updateSystemTray()
{
  Q_ASSERT( false );
}

bool DummyKernel::showPopupAfterDnD()
{
  return false;
}

qreal DummyKernel::closeToQuotaThreshold()
{
  return 80;
}

QStringList DummyKernel::customTemplates()
{
  Q_ASSERT( false );
  return QStringList();
}

bool DummyKernel::excludeImportantMailFromExpiry()
{
  Q_ASSERT( false );
  return true;
}

Akonadi::Entity::Id DummyKernel::lastSelectedFolder()
{
  Q_ASSERT( false );
  return Akonadi::Entity::Id();
}

void DummyKernel::setLastSelectedFolder(const Akonadi::Entity::Id& col)
{
  Q_UNUSED(col);
}





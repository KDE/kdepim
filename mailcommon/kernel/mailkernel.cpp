/*
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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

#include "mailkernel.h"
#include "util/mailutil.h"
#include "imapsettings.h"
#include "pop3settings.h"

#include "pimcommon/util/pimutil.h"

#include <Akonadi/AgentInstance>
#include <Akonadi/AgentManager>
#include <Akonadi/EntityMimeTypeFilterModel>
#include <Akonadi/KMime/SpecialMailCollections>
#include <Akonadi/KMime/SpecialMailCollectionsRequestJob>
#include <Akonadi/KMime/SpecialMailCollectionsDiscoveryJob>

#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityManager>

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

namespace MailCommon {

class KernelPrivate
{
  public:
    KernelPrivate() : kernel( new Kernel )
    {
    }

    ~KernelPrivate()
    {
      kDebug();
      delete kernel;
    }
    Kernel *kernel;
};

K_GLOBAL_STATIC( KernelPrivate, sInstance )

Kernel::Kernel( QObject *parent ) : QObject( parent )
{
  mKernelIf = 0;
  mSettingsIf = 0;
  mFilterIf = 0;
}

Kernel::~Kernel()
{
  kDebug();
}

Kernel *Kernel::self()
{
  return sInstance->kernel; //will create it
}

Akonadi::Collection Kernel::collectionFromId( const Akonadi::Collection::Id &id ) const
{
  const QModelIndex idx =
    Akonadi::EntityTreeModel::modelIndexForCollection(
      kernelIf()->collectionModel(), Akonadi::Collection( id ) );

  return idx.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
}

Akonadi::Collection Kernel::trashCollectionFolder()
{
  return
    Akonadi::SpecialMailCollections::self()->defaultCollection(
      Akonadi::SpecialMailCollections::Trash );
}

Akonadi::Collection Kernel::inboxCollectionFolder()
{
  return
    Akonadi::SpecialMailCollections::self()->defaultCollection(
      Akonadi::SpecialMailCollections::Inbox );
}

Akonadi::Collection Kernel::outboxCollectionFolder()
{
  return
    Akonadi::SpecialMailCollections::self()->defaultCollection(
      Akonadi::SpecialMailCollections::Outbox );
}

Akonadi::Collection Kernel::sentCollectionFolder()
{
  return
    Akonadi::SpecialMailCollections::self()->defaultCollection(
      Akonadi::SpecialMailCollections::SentMail );
}

Akonadi::Collection Kernel::draftsCollectionFolder()
{
  return
    Akonadi::SpecialMailCollections::self()->defaultCollection(
      Akonadi::SpecialMailCollections::Drafts );
}

Akonadi::Collection Kernel::templatesCollectionFolder()
{
  return
    Akonadi::SpecialMailCollections::self()->defaultCollection(
      Akonadi::SpecialMailCollections::Templates );
}

bool Kernel::isSystemFolderCollection( const Akonadi::Collection &col )
{
  return ( col == inboxCollectionFolder() ||
           col == outboxCollectionFolder() ||
           col == sentCollectionFolder() ||
           col == trashCollectionFolder() ||
           col == draftsCollectionFolder() ||
           col == templatesCollectionFolder() );
}

bool Kernel::isMainFolderCollection( const Akonadi::Collection &col )
{
  return col == inboxCollectionFolder();
}

//-----------------------------------------------------------------------------
void Kernel::initFolders()
{
  kDebug() << "Initialized and looking for specialcollection folders.";
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Inbox );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Outbox );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::SentMail );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Drafts );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Trash );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Templates );

  Akonadi::SpecialMailCollectionsDiscoveryJob *job =
       new Akonadi::SpecialMailCollectionsDiscoveryJob( this );
  job->start();
  // we don't connect to the job directly: it will register stuff into SpecialMailCollections, which will emit signals.
}

void Kernel::findCreateDefaultCollection( Akonadi::SpecialMailCollections::Type type )
{
  if ( Akonadi::SpecialMailCollections::self()->hasDefaultCollection( type ) ) {
    const Akonadi::Collection col =
      Akonadi::SpecialMailCollections::self()->defaultCollection( type );

    if ( !( col.rights() & Akonadi::Collection::AllRights ) ) {
      emergencyExit( i18n( "You do not have read/write permission to your inbox folder." ) );
    }
  } else {
    Akonadi::SpecialMailCollectionsRequestJob *job =
      new Akonadi::SpecialMailCollectionsRequestJob( this );

    connect( job, SIGNAL(result(KJob*)),
             this, SLOT(createDefaultCollectionDone(KJob*)) );

    job->requestDefaultCollection( type );
  }
}

void Kernel::createDefaultCollectionDone( KJob *job )
{
  if ( job->error() ) {
    emergencyExit( job->errorText() );
    return;
  }

  Akonadi::SpecialMailCollectionsRequestJob *requestJob =
    qobject_cast<Akonadi::SpecialMailCollectionsRequestJob*>( job );

  const Akonadi::Collection col = requestJob->collection();
  if ( !( col.rights() & Akonadi::Collection::AllRights ) ) {
    emergencyExit( i18n( "You do not have read/write permission to your inbox folder." ) );
  }
  Akonadi::SpecialMailCollections::self()->verifyI18nDefaultCollection( Akonadi::SpecialMailCollections::Inbox );
  Akonadi::SpecialMailCollections::self()->verifyI18nDefaultCollection( Akonadi::SpecialMailCollections::Outbox );
  Akonadi::SpecialMailCollections::self()->verifyI18nDefaultCollection( Akonadi::SpecialMailCollections::SentMail );
  Akonadi::SpecialMailCollections::self()->verifyI18nDefaultCollection( Akonadi::SpecialMailCollections::Drafts );
  Akonadi::SpecialMailCollections::self()->verifyI18nDefaultCollection( Akonadi::SpecialMailCollections::Trash );
  Akonadi::SpecialMailCollections::self()->verifyI18nDefaultCollection( Akonadi::SpecialMailCollections::Templates );

  connect( Akonadi::SpecialMailCollections::self(), SIGNAL(defaultCollectionsChanged()),
           this, SLOT(slotDefaultCollectionsChanged()), Qt::UniqueConnection );
}

void Kernel::slotDefaultCollectionsChanged()
{
  disconnect( Akonadi::SpecialMailCollections::self(), SIGNAL(defaultCollectionsChanged()),
              this, SLOT(slotDefaultCollectionsChanged()));
  initFolders();
}

void Kernel::emergencyExit( const QString &reason )
{
  QString mesg;
  if ( reason.isEmpty() ) {
    mesg = i18n( "The Email program encountered a fatal error and will terminate now" );
  } else {
    mesg = i18n( "The Email program encountered a fatal error and will terminate now.\n"
                 "The error was:\n%1", reason );
  }

  kWarning() << mesg;

  // Show error box for the first error that caused emergencyExit.
  static bool s_showingErrorBox = false;
  if ( !s_showingErrorBox ) {
      s_showingErrorBox = true;
      if ( qApp ) { //see bug 313104
        KMessageBox::error( 0, mesg );
      }
      ::exit(1);
  }
}

bool Kernel::folderIsDraftOrOutbox( const Akonadi::Collection &col )
{
  if ( col == Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Outbox ) ) {
    return true;
  }

  return folderIsDrafts( col );
}

bool Kernel::folderIsDrafts( const Akonadi::Collection &col )
{
  if ( col == Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Drafts ) ) {
    return true;
  }

  const QString idString = QString::number( col.id() );
  if ( idString.isEmpty() ) {
    return false;
  }

  // search the identities if the folder matches the drafts-folder
  const KPIMIdentities::IdentityManager * im = KernelIf->identityManager();
  KPIMIdentities::IdentityManager::ConstIterator end( im->end() );
  for ( KPIMIdentities::IdentityManager::ConstIterator it = im->begin(); it != end; ++it ) {
    if ( (*it).drafts() == idString ) {
      return true;
    }
  }
  return false;
}

bool Kernel::folderIsTemplates( const Akonadi::Collection &col )
{
  if ( col == Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Templates ) ) {
    return true;
  }

  const QString idString = QString::number( col.id() );
  if ( idString.isEmpty() ) {
    return false;
  }

  // search the identities if the folder matches the templates-folder
  const KPIMIdentities::IdentityManager * im = KernelIf->identityManager();
  KPIMIdentities::IdentityManager::ConstIterator end( im->end() );
  for ( KPIMIdentities::IdentityManager::ConstIterator it = im->begin(); it != end; ++it ) {
    if ( (*it).templates() == idString ) {
      return true;
    }
  }
  return false;
}

Akonadi::Collection Kernel::trashCollectionFromResource( const Akonadi::Collection &col )
{
  Akonadi::Collection trashCol;
  if ( col.isValid() ) {
    const Akonadi::AgentInstance agent = Akonadi::AgentManager::self()->instance(col.resource());
    trashCol = Akonadi::SpecialMailCollections::self()->collection(Akonadi::SpecialMailCollections::Trash, agent);
  }
  return trashCol;
}

bool Kernel::folderIsTrash( const Akonadi::Collection &col )
{
  if ( col == Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Trash ) ) {
    return true;
  }

  const Akonadi::AgentInstance::List lst = MailCommon::Util::agentInstances();
  foreach ( const Akonadi::AgentInstance &agent, lst ) {
    const Akonadi::Collection trash = Akonadi::SpecialMailCollections::self()->collection(Akonadi::SpecialMailCollections::Trash, agent);
    if (col == trash) {
      return true;
    }
  }
  return false;
}

bool Kernel::folderIsSentMailFolder( const Akonadi::Collection &col )
{
  if ( col == Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::SentMail ) ) {
    return true;
  }

  const QString idString = QString::number( col.id() );
  if ( idString.isEmpty() ) {
    return false;
  }

  // search the identities if the folder matches the sent-folder
  const KPIMIdentities::IdentityManager * im = KernelIf->identityManager();
  KPIMIdentities::IdentityManager::ConstIterator end( im->end() );
  for ( KPIMIdentities::IdentityManager::ConstIterator it = im->begin(); it != end; ++it ) {
    if ( (*it).fcc() == idString ) {
      return true;
    }
  }
  return false;
}

bool Kernel::folderIsInbox( const Akonadi::Collection &collection, bool withoutPop3InboxSetting )
{
  if ( collection.remoteId().toLower() == QLatin1String( "inbox" ) ||
       collection.remoteId().toLower() == QLatin1String( "/inbox" ) ||
       collection.remoteId().toLower() == QLatin1String( ".inbox" ) ) {
    return true;
  }
  //Fix order. Remoteid is not "inbox" when translated
  if ( collection == Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Inbox ) ) {
    return true;
  }

  //MBOX is one folder only, treat as inbox
  if ( collection.resource().contains(MBOX_RESOURCE_IDENTIFIER) ) {
    return true;
  }

  if ( !withoutPop3InboxSetting ) {
    const Akonadi::AgentInstance::List lst = MailCommon::Util::agentInstances();
    foreach ( const Akonadi::AgentInstance &type, lst ) {
      if ( type.status() == Akonadi::AgentInstance::Broken ) {
        continue;
      }
      if ( type.identifier().contains( POP3_RESOURCE_IDENTIFIER ) ) {
        OrgKdeAkonadiPOP3SettingsInterface *iface =
          MailCommon::Util::createPop3SettingsInterface( type.identifier() );

        if ( iface->isValid() ) {
          if ( iface->targetCollection() == collection.id() ) {
            delete iface;
            return true;
          }
        }
        delete iface;
      }
    }
  }
  return false;
}

}

#include "mailkernel.moc"


/*
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mailkernel.h"

#include "akonadi/entitymimetypefiltermodel.h"
#include "akonadi/kmime/specialmailcollections.h"
#include "akonadi/kmime/specialmailcollectionsrequestjob.h"

#include <KDebug>
#include <KMessageBox>
#include <KLocale>

namespace MailCommon {

class KernelPrivate {
  public:
    KernelPrivate() : kernel( new Kernel ) {}
    ~KernelPrivate() {
      kDebug();
      delete kernel;      
    }
    Kernel* kernel;
};

K_GLOBAL_STATIC( KernelPrivate, sInstance )

Kernel::Kernel( QObject* parent ) : QObject( parent )
{
  the_draftsCollectionFolder = -1;
  the_inboxCollectionFolder = -1;
  the_outboxCollectionFolder = -1;
  the_sentCollectionFolder = -1;
  the_templatesCollectionFolder = -1;
  the_trashCollectionFolder = -1;
  mKernelIf = 0;
  mSettingsIf = 0;
}

Kernel::~Kernel()
{
  kDebug();
}

Kernel *Kernel::self()
{
  return sInstance->kernel; //will create it
}

Akonadi::Collection Kernel::collectionFromId(const Akonadi::Collection::Id& id) const
{
  const QModelIndex idx = Akonadi::EntityTreeModel::modelIndexForCollection(
    kernelIf()->collectionModel(), Akonadi::Collection(id)
  );
  return idx.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
}

Akonadi::Collection Kernel::collectionFromId( const QString &idString ) const
{
  bool ok;
  Akonadi::Collection::Id id = idString.toLongLong( &ok );
  if ( !ok )
    return Akonadi::Collection();
  return collectionFromId( id );
}

Akonadi::Collection Kernel::trashCollectionFolder()
{
  if ( the_trashCollectionFolder < 0 )
    the_trashCollectionFolder = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Trash ).id();
  return collectionFromId( the_trashCollectionFolder );
}

Akonadi::Collection Kernel::inboxCollectionFolder()
{
  if ( the_inboxCollectionFolder < 0 )
    the_inboxCollectionFolder = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Inbox ).id();
  return collectionFromId( the_inboxCollectionFolder );
}

Akonadi::Collection Kernel::outboxCollectionFolder()
{
  if ( the_outboxCollectionFolder < 0 )
    the_outboxCollectionFolder = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Outbox ).id();
  return collectionFromId( the_outboxCollectionFolder );
}

Akonadi::Collection Kernel::sentCollectionFolder()
{
  if ( the_sentCollectionFolder < 0 )
    the_sentCollectionFolder = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::SentMail ).id();
  return collectionFromId( the_sentCollectionFolder );
}

Akonadi::Collection Kernel::draftsCollectionFolder()
{
  if ( the_draftsCollectionFolder < 0 )
    the_draftsCollectionFolder = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Drafts ).id();
  return collectionFromId( the_draftsCollectionFolder );
}

Akonadi::Collection Kernel::templatesCollectionFolder()
{
  if ( the_templatesCollectionFolder < 0 )
    the_templatesCollectionFolder = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Templates ).id();
  return collectionFromId( the_templatesCollectionFolder );
}

bool Kernel::isSystemFolderCollection( const Akonadi::Collection &col)
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
  kDebug() << "KMail is initialize and looking for default specialcollection folders.";
  the_draftsCollectionFolder = the_inboxCollectionFolder = the_outboxCollectionFolder = the_sentCollectionFolder
    = the_templatesCollectionFolder = the_trashCollectionFolder = -1;
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Inbox );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Outbox );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::SentMail );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Drafts );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Trash );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Templates );
}

void Kernel::findCreateDefaultCollection( Akonadi::SpecialMailCollections::Type type )
{
  if( Akonadi::SpecialMailCollections::self()->hasDefaultCollection( type ) ) {
    const Akonadi::Collection col = Akonadi::SpecialMailCollections::self()->defaultCollection( type );
    if ( !( col.rights() & Akonadi::Collection::AllRights ) )
      emergencyExit( i18n("You do not have read/write permission to your inbox folder.") );
  }
  else {
    Akonadi::SpecialMailCollectionsRequestJob *job = new Akonadi::SpecialMailCollectionsRequestJob( this );
    connect( job, SIGNAL( result( KJob* ) ),
             this, SLOT( createDefaultCollectionDone( KJob* ) ) );
    job->requestDefaultCollection( type );
  }
}

void Kernel::createDefaultCollectionDone( KJob * job)
{
  if ( job->error() ) {
    emergencyExit( job->errorText() );
    return;
  }

  Akonadi::SpecialMailCollectionsRequestJob *requestJob = qobject_cast<Akonadi::SpecialMailCollectionsRequestJob*>( job );
  const Akonadi::Collection col = requestJob->collection();
  if ( !( col.rights() & Akonadi::Collection::AllRights ) )
    emergencyExit( i18n("You do not have read/write permission to your inbox folder.") );

  connect( Akonadi::SpecialMailCollections::self(), SIGNAL( defaultCollectionsChanged() ),
           this, SLOT( slotDefaultCollectionsChanged () ), Qt::UniqueConnection  );
}

void Kernel::slotDefaultCollectionsChanged()
{
  initFolders();
}

void Kernel::emergencyExit( const QString& reason )
{
  QString mesg;
  if ( reason.length() == 0 ) {
    mesg = i18n("KMail encountered a fatal error and will terminate now");
  }
  else {
    mesg = i18n("KMail encountered a fatal error and will "
                      "terminate now.\nThe error was:\n%1", reason );
  }

  kWarning() << mesg;

  // Show error box for the first error that caused emergencyExit.
  static bool s_showingErrorBox = false;
  if ( !s_showingErrorBox ) {
      s_showingErrorBox = true;
      KMessageBox::error( 0, mesg );
      ::exit(1);
  }
}


}

#include "mailkernel.moc"
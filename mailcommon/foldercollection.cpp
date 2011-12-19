/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2009,2010,2011 Montel Laurent <montel@kde.org>

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

#include "foldercollection.h"
#include "mailutil.h"
#include "imapsettings.h"
#include "expirejob.h"
#include "jobscheduler.h"

#include <kdebug.h>
#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <kio/jobuidelegate.h>

#include <QMutex>
#include <QMutexLocker>
#include <QWeakPointer>
#include "mailkernel.h"

using namespace Akonadi;

namespace MailCommon {

static QMutex mapMutex;
static QMap<Collection::Id,QWeakPointer<FolderCollection> > fcMap;


QSharedPointer<FolderCollection> FolderCollection::forCollection( const Akonadi::Collection& coll, bool writeConfig )
{
  QMutexLocker lock( &mapMutex );

  QSharedPointer<FolderCollection> sptr = fcMap.value( coll.id() ).toStrongRef();

  if ( !sptr ) {
    sptr = QSharedPointer<FolderCollection>( new FolderCollection( coll, writeConfig ) );
    fcMap.insert( coll.id(), sptr );
  } else {
    if ( !sptr->isWriteConfig() && writeConfig )
      sptr->setWriteConfig( true );
  }

  return sptr;
}

FolderCollection::FolderCollection( const Akonadi::Collection & col, bool writeconfig )
  : mCollection( col ),
    mIgnoreNewMail( false ),
    mPutRepliesInSameFolder( false ),
    mHideInSelectionDialog( false ),
    mWriteConfig( writeconfig ),
    mOldIgnoreNewMail( false )
{
  assert( col.isValid() );
  mIdentity = KernelIf->identityManager()->defaultIdentity().uoid();

  readConfig();
  connect( KernelIf->identityManager(), SIGNAL(changed()),
           this, SLOT(slotIdentitiesChanged()) );
}

FolderCollection::~FolderCollection()
{
  //kDebug()<<" FolderCollection::~FolderCollection"<<this;
  if ( mWriteConfig )
    writeConfig();
}

bool FolderCollection::isWriteConfig() const
{
  return mWriteConfig;
}

void FolderCollection::setWriteConfig( bool writeConfig )
{
  mWriteConfig = writeConfig;
}

QString FolderCollection::name() const
{
  return mCollection.name();
}

bool FolderCollection::isSystemFolder() const
{
  return Kernel::self()->isSystemFolderCollection( mCollection );
}

bool FolderCollection::isStructural() const
{
  return mCollection.contentMimeTypes().isEmpty();
}

bool FolderCollection::isReadOnly() const
{
  return mCollection.rights() & Akonadi::Collection::ReadOnly;
}

bool FolderCollection::canDeleteMessages() const
{
  return mCollection.rights() & Akonadi::Collection::CanDeleteItem;
}

bool FolderCollection::canCreateMessages() const
{
  return mCollection.rights() & Akonadi::Collection::CanCreateItem;
}

qint64 FolderCollection::count() const
{
  return mCollection.statistics().count();
}

Akonadi::Collection::Rights FolderCollection::rights() const
{
  return mCollection.rights();
}

Akonadi::CollectionStatistics FolderCollection::statistics() const
{
  return mCollection.statistics();
}

Akonadi::Collection FolderCollection::collection() const
{
  return mCollection;
}

void FolderCollection::setCollection( const Akonadi::Collection& collection)
{
  mCollection = collection;
}


void FolderCollection::slotIdentitiesChanged()
{
  uint defaultIdentity =  KernelIf->identityManager()->defaultIdentity().uoid();
  // The default identity may have changed, therefore set it again
  // if necessary
  if ( mUseDefaultIdentity )
    mIdentity = defaultIdentity;

  // Fall back to the default identity if the one used currently is invalid
  if ( KernelIf->identityManager()->identityForUoid( mIdentity ).isNull() ) {
    mIdentity = defaultIdentity;
    mUseDefaultIdentity = true;
  }
}

QString FolderCollection::configGroupName(const Akonadi::Collection& col)
{
  return QString::fromLatin1( "Folder-%1" ).arg( QString::number( col.id() ) );
}

void FolderCollection::readConfig()
{
  const KConfigGroup configGroup( KernelIf->config(), configGroupName(mCollection) );
  mMailingListEnabled = configGroup.readEntry( "MailingListEnabled", false );
  mMailingList.readConfig( configGroup );

  mUseDefaultIdentity = configGroup.readEntry( "UseDefaultIdentity", true );
  uint defaultIdentity = KernelIf->identityManager()->defaultIdentity().uoid();
  mIdentity = configGroup.readEntry("Identity", defaultIdentity );
  slotIdentitiesChanged();

  mPutRepliesInSameFolder = configGroup.readEntry( "PutRepliesInSameFolder", false );
  mHideInSelectionDialog = configGroup.readEntry( "HideInSelectionDialog", false );
  mIgnoreNewMail = configGroup.readEntry( "IgnoreNewMail", false );
  mOldIgnoreNewMail = mIgnoreNewMail;


  const QString shortcut( configGroup.readEntry( "Shortcut" ) );
  if ( !shortcut.isEmpty() ) {
    KShortcut sc( shortcut );
    setShortcut( sc );
  }
}

bool FolderCollection::isValid() const
{
  return mCollection.isValid();
}

void FolderCollection::writeConfig() const
{
  KConfigGroup configGroup( KernelIf->config(), configGroupName(mCollection) );

  configGroup.writeEntry("MailingListEnabled", mMailingListEnabled);
  mMailingList.writeConfig( configGroup );


  configGroup.writeEntry( "UseDefaultIdentity", mUseDefaultIdentity );

  if ( !mUseDefaultIdentity ) {
    uint defaultIdentityId = -1;

    if ( mCollection.resource().contains( IMAP_RESOURCE_IDENTIFIER ) ) {
      OrgKdeAkonadiImapSettingsInterface *imapSettingsInterface = Util::createImapSettingsInterface( mCollection.resource() );
      if ( imapSettingsInterface->isValid() ) {
        QDBusReply<int> reply = imapSettingsInterface->accountIdentity();
        if ( reply.isValid() ) {
          defaultIdentityId = static_cast<uint>( reply );
        }
      }
      delete imapSettingsInterface;
    } else {
      defaultIdentityId = KernelIf->identityManager()->defaultIdentity().uoid();
    }

    if ( mIdentity != defaultIdentityId )
      configGroup.writeEntry( "Identity", mIdentity );
    else
      configGroup.deleteEntry( "Identity" );

  } else {
    configGroup.deleteEntry("Identity");
  }

  configGroup.writeEntry( "PutRepliesInSameFolder", mPutRepliesInSameFolder );
  configGroup.writeEntry( "HideInSelectionDialog", mHideInSelectionDialog );
  configGroup.writeEntry( "IgnoreNewMail", mIgnoreNewMail );
  if ( !mShortcut.isEmpty() )
    configGroup.writeEntry( "Shortcut", mShortcut.toString() );
  else
    configGroup.deleteEntry( "Shortcut" );
  if ( mIgnoreNewMail != mOldIgnoreNewMail ) {
    KernelIf->updateSystemTray();
  }
}

void FolderCollection::setShortcut( const KShortcut &sc )
{
  if ( mShortcut != sc ) {
    mShortcut = sc;
  }
}

void FolderCollection::setUseDefaultIdentity( bool useDefaultIdentity )
{
  mUseDefaultIdentity = useDefaultIdentity;
  if ( mUseDefaultIdentity )
    mIdentity = KernelIf->identityManager()->defaultIdentity().uoid();
  KernelIf->syncConfig();
}

void FolderCollection::setIdentity( uint identity )
{
  mIdentity = identity;
  KernelIf->syncConfig();
}

uint FolderCollection::identity() const
{
  if ( mUseDefaultIdentity ) {
    int identityId = -1;
    OrgKdeAkonadiImapSettingsInterface *imapSettingsInterface = Util::createImapSettingsInterface( mCollection.resource() );
    if ( imapSettingsInterface->isValid() ) {
      QDBusReply<bool> useDefault = imapSettingsInterface->useDefaultIdentity();
      if( useDefault.isValid() && useDefault.value() ) {
	delete imapSettingsInterface;
        return mIdentity;
      }

       QDBusReply<int> remoteAccountIdent = imapSettingsInterface->accountIdentity();
      if ( remoteAccountIdent.isValid() && remoteAccountIdent.value() > 0 ) {
        identityId = remoteAccountIdent;
      }
    }
    delete imapSettingsInterface;
    if ( identityId != -1 && !KernelIf->identityManager()->identityForUoid( identityId ).isNull() )
      return identityId;
  }
  return mIdentity;
}


QString FolderCollection::mailingListPostAddress() const
{
  if ( mMailingList.features() & MailingList::Post ) {
    KUrl::List post = mMailingList.postUrls();
    KUrl::List::const_iterator end( post.constEnd() );
    for( KUrl::List::const_iterator it = post.constBegin(); it != end; ++it ) {
      // We check for isEmpty because before 3.3 postAddress was just an
      // email@kde.org and that leaves protocol() field in the kurl class
      const QString protocol = (*it).protocol();
      if ( protocol == QLatin1String( "mailto" ) ||
           protocol.isEmpty() )
        return (*it).path();
    }
  }
  return QString();
}

void FolderCollection::setMailingListEnabled( bool enabled )
{
  if ( mMailingListEnabled != enabled ) {
    mMailingListEnabled = enabled;
    writeConfig();
  }
}

void FolderCollection::setMailingList( const MailingList& mlist )
{
  if ( mMailingList == mlist )
    return;
  
  mMailingList = mlist;
  writeConfig();
}

}


#include "foldercollection.moc"

/*
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

// config keys:
static const char configKeyDefaultIdentity[] = "Default Identity";

#include "identitymanager.h"
#include "identity.h" // for IdentityList::{export,import}Data

#include <kpimutils/email.h> // for static helper functions

#include <kemailsettings.h> // for IdentityEntry::fromControlCenter()
#include <klocale.h>
#include <klocalizedstring.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kuser.h>
#include <kconfiggroup.h>

#include <QList>
#include <QRegExp>
#include <QtDBus/QtDBus>

#include <assert.h>
#include <krandom.h>

#include "identitymanageradaptor.h"

using namespace KPIMIdentities;

static QString newDBusObjectName()
{
  static int s_count = 0;
  QString name( "/KPIMIDENTITIES_IdentityManager" );
  if ( s_count++ ) {
    name += '_';
    name += QString::number( s_count );
  }
  return name;
}

IdentityManager::IdentityManager( bool readonly, QObject *parent,
                                  const char *name )
    : QObject( parent )
{
  setObjectName( name );
  KGlobal::locale()->insertCatalog( "libkpimidentities" );
  new IdentityManagerAdaptor( this );
  QDBusConnection dbus = QDBusConnection::sessionBus();
  const QString dbusPath = newDBusObjectName();
  setProperty( "uniqueDBusPath", dbusPath );
  const QString dbusInterface = "org.kde.pim.IdentityManager";
  dbus.registerObject( dbusPath, this );
  dbus.connect( QString(), QString(), dbusInterface, "identitiesChanged", this,
                SLOT(slotIdentitiesChanged(QString)) );

  mReadOnly = readonly;
  mConfig = new KConfig( "emailidentities" );
  readConfig( mConfig );
  if ( mIdentities.isEmpty() ) {
    kDebug( 5325 ) << "emailidentities is empty -> convert from kmailrc";
    // No emailidentities file, or an empty one due to broken conversion
    // (kconf_update bug in kdelibs <= 3.2.2)
    // => convert it, i.e. read settings from kmailrc
    KConfig kmailConf( "kmailrc" );
    readConfig( &kmailConf );
  }
  // we need at least a default identity:
  if ( mIdentities.isEmpty() ) {
    kDebug( 5325 ) << "IdentityManager: No identity found. Creating default.";
    createDefaultIdentity();
    commit();
  }
  // Migration: people without settings in kemailsettings should get some
  if ( KEMailSettings().getSetting( KEMailSettings::EmailAddress ).isEmpty() ) {
    writeConfig();
  }
}

IdentityManager::~IdentityManager()
{
  kWarning( hasPendingChanges(), 5325 )
  << "IdentityManager: There were uncommitted changes!";
  delete mConfig;
}

QString IdentityManager::makeUnique( const QString &name ) const
{
  int suffix = 1;
  QString result = name;
  while ( identities().contains( result ) ) {
    result = i18nc( "%1: name; %2: number appended to it to make it unique "
                    "among a list of names", "%1 #%2",
                    name, suffix );
    suffix++;
  }
  return result;
}

bool IdentityManager::isUnique( const QString &name ) const
{
  return !identities().contains( name );
}

void IdentityManager::commit()
{
  // early out:
  if ( !hasPendingChanges() || mReadOnly ) {
    return;
  }

  QList<uint> seenUOIDs;
  QList<Identity>::ConstIterator end = mIdentities.constEnd();
  for ( QList<Identity>::ConstIterator it = mIdentities.constBegin();
        it != end; ++it ) {
    seenUOIDs << ( *it ).uoid();
  }

  QList<uint> changedUOIDs;
  // find added and changed identities:
  for ( QList<Identity>::ConstIterator it = mShadowIdentities.constBegin();
        it != mShadowIdentities.constEnd(); ++it ) {
    int index = seenUOIDs.indexOf( ( *it ).uoid() );
    if ( index != -1 ) {
      uint uoid = seenUOIDs.at( index );
      const Identity &orig = identityForUoid( uoid );  // look up in mIdentities
      if ( *it != orig ) {
        // changed identity
        kDebug( 5325 ) << "emitting changed() for identity" << uoid;
        emit changed( *it );
        changedUOIDs << uoid;
      }
      seenUOIDs.removeAll( uoid );
    } else {
      // new identity
      kDebug( 5325 ) << "emitting added() for identity" << ( *it ).uoid();
      emit added( *it );
    }
  }

  // what's left are deleted identities:
  for ( QList<uint>::ConstIterator it = seenUOIDs.constBegin();
        it != seenUOIDs.constEnd(); ++it ) {
    kDebug( 5325 ) << "emitting deleted() for identity" << ( *it );
    emit deleted( *it );
  }

  mIdentities = mShadowIdentities;
  writeConfig();

  // now that mIdentities has all the new info, we can emit the added/changed
  // signals that ship a uoid. This is because the slots might use
  // identityForUoid(uoid)...
  QList<uint>::ConstIterator changedEnd( changedUOIDs.constEnd() );
  for ( QList<uint>::ConstIterator it = changedUOIDs.constBegin();
        it != changedEnd; ++it ) {
    emit changed( *it );
  }

  emit changed(); // normal signal

  // DBus signal for other IdentityManager instances
  const QString ourIdentifier = QString::fromLatin1( "%1/%2" ).
                                  arg( QDBusConnection::sessionBus().baseService() ).
                                  arg( property( "uniqueDBusPath" ).toString() );
  emit identitiesChanged( ourIdentifier );
}

void IdentityManager::rollback()
{
  mShadowIdentities = mIdentities;
}

bool IdentityManager::hasPendingChanges() const
{
  return mIdentities != mShadowIdentities;
}

QStringList IdentityManager::identities() const
{
  QStringList result;
  ConstIterator end = mIdentities.constEnd();
  for ( ConstIterator it = mIdentities.constBegin();
        it != end; ++it ) {
    result << ( *it ).identityName();
  }
  return result;
}

QStringList IdentityManager::shadowIdentities() const
{
  QStringList result;
  ConstIterator end = mShadowIdentities.constEnd();
  for ( ConstIterator it = mShadowIdentities.constBegin();
        it != end; ++it ) {
    result << ( *it ).identityName();
  }
  return result;
}

void IdentityManager::sort()
{
  qSort( mShadowIdentities );
}

void IdentityManager::writeConfig() const
{
  const QStringList identities = groupList( mConfig );
  QStringList::const_iterator groupEnd = identities.constEnd();
  for ( QStringList::const_iterator group = identities.constBegin();
        group != groupEnd; ++group ) {
    mConfig->deleteGroup( *group );
  }
  int i = 0;
  ConstIterator end = mIdentities.constEnd();
  for ( ConstIterator it = mIdentities.constBegin();
        it != end; ++it, ++i ) {
    KConfigGroup cg( mConfig, QString::fromLatin1( "Identity #%1" ).arg( i ) );
    ( *it ).writeConfig( cg );
    if ( ( *it ).isDefault() ) {
      // remember which one is default:
      KConfigGroup general( mConfig, "General" );
      general.writeEntry( configKeyDefaultIdentity, ( *it ).uoid() );

      // Also write the default identity to emailsettings
      KEMailSettings es;
      es.setSetting( KEMailSettings::RealName, ( *it ).fullName() );
      es.setSetting( KEMailSettings::EmailAddress, ( *it ).primaryEmailAddress() );
      es.setSetting( KEMailSettings::Organization, ( *it ).organization() );
      es.setSetting( KEMailSettings::ReplyToAddress, ( *it ).replyToAddr() );
    }
  }
  mConfig->sync();

}

void IdentityManager::readConfig( KConfig *config )
{
  mIdentities.clear();

  const QStringList identities = groupList( config );
  if ( identities.isEmpty() ) {
    return; // nothing to be done...
  }

  KConfigGroup general( config, "General" );
  uint defaultIdentity = general.readEntry( configKeyDefaultIdentity, 0 );
  bool haveDefault = false;
  QStringList::const_iterator groupEnd = identities.constEnd();
  for ( QStringList::const_iterator group = identities.constBegin();
        group != groupEnd; ++group ) {
    KConfigGroup configGroup( config, *group );
    mIdentities << Identity();
    mIdentities.last().readConfig( configGroup );
    if ( !haveDefault && mIdentities.last().uoid() == defaultIdentity ) {
      haveDefault = true;
      mIdentities.last().setIsDefault( true );
    }
  }

  if ( !haveDefault ) {
    kWarning( 5325 ) << "IdentityManager: There was no default identity."
                     << "Marking first one as default.";
    mIdentities.first().setIsDefault( true );
  }
  qSort( mIdentities );

  mShadowIdentities = mIdentities;
}

QStringList IdentityManager::groupList( KConfig *config ) const
{
  return config->groupList().filter( QRegExp( "^Identity #\\d+$" ) );
}

IdentityManager::ConstIterator IdentityManager::begin() const
{
  return mIdentities.begin();
}

IdentityManager::ConstIterator IdentityManager::end() const
{
  return mIdentities.end();
}

IdentityManager::Iterator IdentityManager::modifyBegin()
{
  return mShadowIdentities.begin();
}

IdentityManager::Iterator IdentityManager::modifyEnd()
{
  return mShadowIdentities.end();
}

const Identity &IdentityManager::identityForUoid( uint uoid ) const
{
  for ( ConstIterator it = begin(); it != end(); ++it ) {
    if ( ( *it ).uoid() == uoid ) {
      return ( *it );
    }
  }
  return Identity::null();
}

const Identity &IdentityManager::identityForUoidOrDefault( uint uoid ) const
{
  const Identity &ident = identityForUoid( uoid );
  if ( ident.isNull() ) {
    return defaultIdentity();
  } else {
    return ident;
  }
}

const Identity &IdentityManager::identityForAddress(
  const QString &addresses ) const
{
  const QStringList addressList = KPIMUtils::splitAddressList( addresses );
  foreach ( const QString &fullAddress, addressList ) {
    const QString addrSpec = KPIMUtils::extractEmailAddress( fullAddress ).toLower();
    for ( ConstIterator it = begin(); it != end(); ++it ) {
      const Identity &identity = *it;
      if ( identity.matchesEmailAddress( addrSpec ) ) {
        return identity;
      }
    }
  }
  return Identity::null();
}

bool IdentityManager::thatIsMe( const QString &addressList ) const
{
  return !identityForAddress( addressList ).isNull();
}

Identity &IdentityManager::modifyIdentityForName( const QString &name )
{
  for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
    if ( ( *it ).identityName() == name ) {
      return ( *it );
    }
  }

  kWarning( 5325 ) << "IdentityManager::modifyIdentityForName() used as"
                   << "newFromScratch() replacement!"
                   << endl << "  name == \"" << name << "\"";
  return newFromScratch( name );
}

Identity &IdentityManager::modifyIdentityForUoid( uint uoid )
{
  for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
    if ( ( *it ).uoid() == uoid ) {
      return ( *it );
    }
  }

  kWarning( 5325 ) << "IdentityManager::identityForUoid() used as"
                   << "newFromScratch() replacement!"
                   << endl << "  uoid == \"" << uoid << "\"";
  return newFromScratch( i18n( "Unnamed" ) );
}

const Identity &IdentityManager::defaultIdentity() const
{
  for ( ConstIterator it = begin(); it != end(); ++it ) {
    if ( ( *it ).isDefault() ) {
      return ( *it );
    }
  }

  if ( mIdentities.isEmpty() ) {
    kFatal( 5325 ) << "IdentityManager: No default identity found!";
  } else {
    kWarning( 5325 ) << "IdentityManager: No default identity found!";
  }
  return *begin();
}

bool IdentityManager::setAsDefault( uint uoid )
{
  // First, check if the identity actually exists:
  bool found = false;
  for ( ConstIterator it = mShadowIdentities.constBegin();
        it != mShadowIdentities.constEnd(); ++it ) {
    if ( ( *it ).uoid() == uoid ) {
      found = true;
      break;
    }
  }

  if ( !found ) {
    return false;
  }

  // Then, change the default as requested:
  for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
    ( *it ).setIsDefault( ( *it ).uoid() == uoid );
  }

  // and re-sort:
  sort();
  return true;
}

bool IdentityManager::removeIdentity( const QString &name )
{
  if ( mShadowIdentities.size() <= 1 ) {
    return false;
  }

  for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
    if ( ( *it ).identityName() == name ) {
      bool removedWasDefault = ( *it ).isDefault();
      mShadowIdentities.erase( it );
      if ( removedWasDefault && !mShadowIdentities.isEmpty() ) {
        mShadowIdentities.first().setIsDefault( true );
      }
      return true;
    }
  }
  return false;
}

bool IdentityManager::removeIdentityForced( const QString &name )
{
  for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
    if ( ( *it ).identityName() == name ) {
      bool removedWasDefault = ( *it ).isDefault();
      mShadowIdentities.erase( it );
      if ( removedWasDefault && !mShadowIdentities.isEmpty() ) {
        mShadowIdentities.first().setIsDefault( true );
      }
      return true;
    }
  }
  return false;
}

Identity &IdentityManager::newFromScratch( const QString &name )
{
  return newFromExisting( Identity( name ) );
}

Identity &IdentityManager::newFromControlCenter( const QString &name )
{
  KEMailSettings es;
  es.setProfile( es.defaultProfileName() );

  return
    newFromExisting( Identity( name,
                               es.getSetting( KEMailSettings::RealName ),
                               es.getSetting( KEMailSettings::EmailAddress ),
                               es.getSetting( KEMailSettings::Organization ),
                               es.getSetting( KEMailSettings::ReplyToAddress ) ) );
}

Identity &IdentityManager::newFromExisting( const Identity &other, const QString &name )
{
  mShadowIdentities << other;
  Identity &result = mShadowIdentities.last();
  result.setIsDefault( false );  // we don't want two default identities!
  result.setUoid( newUoid() );  // we don't want two identies w/ same UOID
  if ( !name.isNull() ) {
    result.setIdentityName( name );
  }
  return result;
}

void IdentityManager::createDefaultIdentity()
{
  QString fullName, emailAddress;
  bool done = false;

  // Check if the application has any settings
  createDefaultIdentity( fullName, emailAddress );

  // If not, then use the kcontrol settings
  if ( fullName.isEmpty() && emailAddress.isEmpty() ) {
    KEMailSettings emailSettings;
    fullName = emailSettings.getSetting( KEMailSettings::RealName );
    emailAddress = emailSettings.getSetting( KEMailSettings::EmailAddress );

    if ( !fullName.isEmpty() && !emailAddress.isEmpty() ) {
      newFromControlCenter( i18nc( "use default address from control center",
                                   "Default" ) );
      done = true;
    } else {
      // If KEmailSettings doesn't have name and address, generate something from KUser
      KUser user;
      if ( fullName.isEmpty() ) {
        fullName = user.property( KUser::FullName ).toString();
      }
      if ( emailAddress.isEmpty() ) {
        emailAddress = user.loginName();
        if ( !emailAddress.isEmpty() ) {
          KConfigGroup general( mConfig, "General" );
          QString defaultdomain = general.readEntry( "Default domain" );
          if ( !defaultdomain.isEmpty() ) {
            emailAddress += '@' + defaultdomain;
          } else {
            emailAddress.clear();
          }
        }
      }
    }
  }

  if ( !done ) {
    // Default identity name
    QString name( i18nc( "Default name for new email accounts/identities.", "Unnamed" ) );

    if ( !emailAddress.isEmpty() ) {
      // If we have an email address, create a default identity name from it
      QString idName = emailAddress;
      int pos = idName.indexOf( '@' );
      if ( pos != -1 ) {
        name = idName.mid( pos + 1, -1 );
      }

      // Make the name a bit more human friendly
      name.replace( '.', ' ' );
      pos = name.indexOf( ' ' );
      if ( pos != 0 ) {
        name[pos + 1] = name[pos + 1].toUpper();
      }
      name[0] = name[0].toUpper();
    } else if ( !fullName.isEmpty() ) {
      // If we have a full name, create a default identity name from it
      name = fullName;
    }
    mShadowIdentities << Identity( name, fullName, emailAddress );
  }

  mShadowIdentities.last().setIsDefault( true );
  mShadowIdentities.last().setUoid( newUoid() );
  if ( mReadOnly ) { // commit won't do it in readonly mode
    mIdentities = mShadowIdentities;
  }
}

int IdentityManager::newUoid()
{
  int uoid;

  // determine the UOIDs of all saved identities
  QList<uint> usedUOIDs;
  QList<Identity>::ConstIterator end( mIdentities.constEnd() );
  for ( QList<Identity>::ConstIterator it = mIdentities.constBegin();
        it != end; ++it ) {
    usedUOIDs << ( *it ).uoid();
  }

  if ( hasPendingChanges() ) {
    // add UOIDs of all shadow identities. Yes, we will add a lot of duplicate
    // UOIDs, but avoiding duplicate UOIDs isn't worth the effort.
    QList<Identity>::ConstIterator endShadow( mShadowIdentities.constEnd() );
    for ( QList<Identity>::ConstIterator it = mShadowIdentities.constBegin();
          it != endShadow; ++it ) {
      usedUOIDs << ( *it ).uoid();
    }
  }

  usedUOIDs << 0; // no UOID must be 0 because this value always refers to the
  // default identity

  do {
    uoid = KRandom::random();
  } while ( usedUOIDs.indexOf( uoid ) != -1 );

  return uoid;
}

QStringList KPIMIdentities::IdentityManager::allEmails() const
{
  QStringList lst;
  for ( ConstIterator it = begin(); it != end(); ++it ) {
    lst << ( *it ).primaryEmailAddress();
    if ( !( *it ).emailAliases().isEmpty() ) {
      lst << ( *it ).emailAliases();
    }
  }
  return lst;
}

void KPIMIdentities::IdentityManager::slotRollback()
{
  rollback();
}

void KPIMIdentities::IdentityManager::slotIdentitiesChanged( const QString &id )
{
  kDebug( 5325 ) << " KPIMIdentities::IdentityManager::slotIdentitiesChanged :" << id;
  const QString ourIdentifier = QString::fromLatin1( "%1/%2" ).
                                  arg( QDBusConnection::sessionBus().baseService() ).
                                  arg( property( "uniqueDBusPath" ).toString() );
  if ( id != ourIdentifier ) {
    mConfig->reparseConfiguration();
    Q_ASSERT( !hasPendingChanges() );
    readConfig( mConfig );
    emit changed();
  }
}


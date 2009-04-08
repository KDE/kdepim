/*
    This file is part of kdepim.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kmailchanges.h"

#include <kapplication.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <kemailsettings.h>
#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include <kdebug.h>
#include <kstringhandler.h>
#include <krandom.h>
#include <kwallet.h>
using namespace KWallet;

static const char* s_folderContentsType[] = {
  I18N_NOOP( "Calendar" ),
  I18N_NOOP( "Contacts" ),
  I18N_NOOP( "Notes" ),
  I18N_NOOP( "Tasks" ),
  I18N_NOOP( "Journal" ) };

Wallet* CreateImapAccount::mWallet = 0;

CreateImapAccount::CreateImapAccount( const QString &accountName, const QString &title )
  : KConfigPropagator::Change( title ),
    mAccountName( accountName ), mPort( 993 ), mEnableSieve( false ), mEnableSavePassword( true ),
    mEncryption( None ), mAuthentication( NONE ), mAuthenticationSend( PLAIN ), mSmtpPort( 25 ),
    mExistingAccountId( -1 ), mExistingTransportId( -1 ),
    mCustomWriter( 0 )
{
}

CreateImapAccount::~CreateImapAccount()
{
  delete mCustomWriter;
}

void CreateImapAccount::setServer( const QString &s )
{
  mServer = s;
}

void CreateImapAccount::setUser( const QString &s )
{
  mUser = s;
}

void CreateImapAccount::setPassword( const QString &s )
{
  mPassword = s;
}

void CreateImapAccount::setRealName( const QString &s )
{
  mRealName = s;
}

void CreateImapAccount::setPort( int port )
{
  mPort = port;
}

void CreateImapAccount::setEmail( const QString &s )
{
  mEmail = s;
}

void CreateImapAccount::enableSieve( bool b )
{
  mEnableSieve = b;
}

void CreateImapAccount::setSieveVacationFileName( const QString& f )
{
  mSieveVacationFileName = f;
}

void CreateImapAccount::enableSavePassword( bool b )
{
  mEnableSavePassword = b;
}

void CreateImapAccount::setEncryption(
  CreateImapAccount::Encryption e )
{
  mEncryption = e;
}

void CreateImapAccount::setAuthentication(
  CreateImapAccount::Authentication a )
{
  mAuthentication = a;
}

void CreateImapAccount::setDefaultDomain(const QString &d)
{
  mDefaultDomain = d;
}

void CreateImapAccount::setAuthenticationSend(
  CreateImapAccount::Authentication a )
{
  mAuthenticationSend = a;
}

void CreateImapAccount::setSmtpPort( int port )
{
  mSmtpPort = port;
}

void CreateImapAccount::setExistingAccountId( int id )
{
  mExistingAccountId = id;
}

void CreateImapAccount::setExistingTransportId( int id )
{
  mExistingTransportId = id;
}

void CreateImapAccount::setCustomWriter(
  CreateImapAccount::CustomWriter *writer )
{
  mCustomWriter = writer;
}


CreateDisconnectedImapAccount::CreateDisconnectedImapAccount(const QString & accountName) :
    CreateImapAccount( accountName, i18n("Create Disconnected IMAP Account for KMail") ),
    mLocalSubscription( false ), mGroupwareType( GroupwareKolab )
{
}

void CreateDisconnectedImapAccount::apply()
{
  if ( mEmail.isEmpty() ) mEmail = mUser + '@' + mServer;

  KConfig c( "kmailrc" );
  KConfigGroup group = c.group( "General" );
  group.writeEntry( "Default domain", mDefaultDomain );
  int accountId;
  if ( mExistingAccountId < 0 ) {
    uint accCnt = group.readEntry( "accounts", 0 );
    accountId = accCnt + 1;
    group.writeEntry( "accounts", accountId );
  } else {
    accountId = mExistingAccountId;
  }
  int transportId;
  if ( mExistingTransportId < 0 ) {
    uint transCnt = group.readEntry( "transports", 0 );
    transportId = transCnt + 1;
    group.writeEntry( "transports", transportId );
  } else {
    transportId = mExistingTransportId;
  }

  group = c.group( QString("Account %1").arg( accountId ) );
  int uid;
  if ( mExistingAccountId < 0 ) {
    uid = KRandom::random();
    group.writeEntry( "Folder", uid );
  } else {
    uid = group.readEntry( "Folder",0 );
  }
  group.writeEntry( "Id", uid );
  group.writeEntry( "Type", "DImap");
  switch ( mAuthentication ) {
    case NONE:
      group.writeEntry( "auth", "*" );
      break;
    case PLAIN:
      group.writeEntry( "auth", "PLAIN" );
      break;
    case LOGIN:
      group.writeEntry( "auth", "LOGIN" );
      break;
    case NTLM_SPA:
      group.writeEntry( "auth", "NTLM" );
      break;
    case GSSAPI:
      group.writeEntry( "auth", "GSSAPI" );
      break;
    case DIGEST_MD5:
      group.writeEntry( "auth", "DIGEST-MD5" );
      break;
    case CRAM_MD5:
      group.writeEntry( "auth", "CRAM-MD5" );
      break;
  }
  group.writeEntry( "Name", mAccountName );
  group.writeEntry( "host", mServer );
  group.writeEntry( "port", mPort );

  group.writeEntry( "groupwareType", (int)mGroupwareType );

  // in case the user wants to get rid of some groupware folders
  group.writeEntry( "locally-subscribed-folders", mLocalSubscription );

  group.writeEntry( "login", mUser );

  group.writeEntry( "sieve-support", mEnableSieve ? "true" : "false" );
  if ( !mSieveVacationFileName.isEmpty() )
    group.writeEntry( "sieve-vacation-filename", mSieveVacationFileName );

  if ( mEncryption == SSL ) {
    group.writeEntry( "use-ssl", true );
  } else if ( mEncryption == TLS ) {
    group.writeEntry( "use-tls", true );
  }

  if ( mEnableSavePassword ) {
    if ( !writeToWallet( "account", accountId ) ) {
      group.writeEntry( "pass", KStringHandler::obscure( mPassword ) );
      group.writeEntry( "store-passwd", true );
    }
  }


  group = c.group( QString("Folder-%1").arg( uid ) );
  group.writeEntry( "isOpen", true );

  group = c.group( QLatin1String( "AccountWizard" ) );
  group.writeEntry( QLatin1String( "ShowOnStartup" ), false );

  group = c.group( QLatin1String( "Composer" ) );
  group.writeEntry( "default-transport", mAccountName );

  KConfig transport( "mailtransports" );
  group = transport.group( QString("Transport %1").arg( transportId ) );
  group.writeEntry( "name", mAccountName );
  group.writeEntry( "host", mServer );
  group.writeEntry( "type", "smtp" );
  group.writeEntry( "port", mSmtpPort );
  if ( mEncryption == SSL ) {
    group.writeEntry( "encryption", "SSL" );
  } else if ( mEncryption == TLS ) {
    group.writeEntry( "encryption", "TLS" );
  }
  group.writeEntry( "auth", true );
  if ( mAuthenticationSend == PLAIN ) {
    group.writeEntry( "authtype", "PLAIN" );
  } else if ( mAuthenticationSend == LOGIN ) {
    group.writeEntry( "authtype", "LOGIN" );
  }
  group.writeEntry( "id", transportId );
  group.writeEntry( "user", mUser );
  if ( mEnableSavePassword ) {
    if ( !writeToWallet( "transport", transportId ) ) {
      group.writeEntry( "pass", KStringHandler::obscure( mPassword ) );
      group.writeEntry( "storepass", true );
    }
  }

  // Write email in "default kcontrol settings", used by IdentityManager
  // if it has to create a default identity.
  KEMailSettings es;
  es.setSetting( KEMailSettings::RealName, mRealName );
  es.setSetting( KEMailSettings::EmailAddress, mEmail );

  KPIMIdentities::IdentityManager identityManager;
  if ( !identityManager.allEmails().contains( mEmail ) ) {
    // Not sure how to name the identity. First one is "Default", next one mAccountName, but then...
    // let's use the server name after that.
    QString accountName = mAccountName;
    const QStringList identities = identityManager.identities();
    if ( identities.contains( accountName )  ) {
      accountName = mServer;
      int i = 2;
      // And if there's already one, number them
      while ( identities.contains( accountName )  ) {
        accountName = mServer + ' ' + QString::number( i++ );
      }
    }

    KPIMIdentities::Identity& identity = identityManager.newFromScratch( accountName );
    identity.setFullName( mRealName );
    identity.setEmailAddr( mEmail );
    identityManager.commit();
  }

  if ( mCustomWriter ) {
    mCustomWriter->writeFolder( c, uid );
    mCustomWriter->writeIds( accountId, transportId );
  }
}

CreateOnlineImapAccount::CreateOnlineImapAccount(const QString & accountName) :
    CreateImapAccount( accountName, i18n("Create Online IMAP Account for KMail") )
{
}

void CreateOnlineImapAccount::apply()
{
  KConfig c( "kmailrc" );
  KConfigGroup group = c.group( "General" );
  uint accCnt = group.readEntry( "accounts", 0 );
  group.writeEntry( "accounts", accCnt+1 );

  group = c.group( QString("Account %1").arg(accCnt+1) );
  int uid = KRandom::random();
  group.writeEntry( "Folder", uid );
  group.writeEntry( "Id", uid );
  group.writeEntry( "Type", "Imap" );
  group.writeEntry( "auth", "*" );
  group.writeEntry( "Name", mAccountName );
  group.writeEntry( "host", mServer );

  group.writeEntry( "login", mUser );

  if ( mEnableSavePassword ) {
    if ( !writeToWallet( "account", accCnt+1 ) ) {
      group.writeEntry( "pass", KStringHandler::obscure( mPassword ) );
      group.writeEntry( "store-passwd", true );
    }
  }
  group.writeEntry( "port", "993" );

  if ( mEncryption == SSL ) {
    group.writeEntry( "use-ssl", true );
  } else if ( mEncryption == TLS ) {
    group.writeEntry( "use-tls", true );
  }

  if ( mAuthenticationSend == PLAIN ) {
    group.writeEntry( "authtype", "PLAIN" );
  } else if ( mAuthenticationSend == LOGIN ) {
    group.writeEntry( "authtype", "LOGIN" );
  }

  group.writeEntry( "sieve-support", mEnableSieve );

  // locally unsubscribe the default folders
  group.writeEntry( "locally-subscribed-folders", true );
  QString groupwareFolders = QString("/INBOX/%1/,/INBOX/%2/,/INBOX/%3/,/INBOX/%4/,/INBOX/%5/")
      .arg( i18n(s_folderContentsType[0]) ).arg( i18n(s_folderContentsType[1]) )
      .arg( i18n(s_folderContentsType[2]) ).arg( i18n(s_folderContentsType[3]) )
      .arg( i18n(s_folderContentsType[4]) );
  group.writeEntry( "locallyUnsubscribedFolders", groupwareFolders );

  group = c.group( QString("Folder-%1").arg( uid ) );
  group.writeEntry( "isOpen", true );

  group = c.group( QLatin1String( "AccountWizard" ) );
  group.writeEntry( QLatin1String( "ShowOnStartup" ), false );
}

bool CreateImapAccount::writeToWallet(const QString & type, int id)
{
  if ( !Wallet::isEnabled() )
    return false;
  if ( !mWallet || !mWallet->isOpen() ) {
    delete mWallet;
    WId window = 0;
    if ( qApp->activeWindow() )
      window = qApp->activeWindow()->winId();
    mWallet = Wallet::openWallet( Wallet::NetworkWallet(), window );
    if ( !mWallet )
      return false;
  }
  QString folder, str_id;
  if ( type=="transport" ) {
    folder = "mailtransports";
    str_id = QString::number( id );
  } else {
    folder = "kmail";
    str_id = type + '-' + QString::number( id );
  }
  if ( !mWallet->hasFolder( folder ) )
    mWallet->createFolder( folder );
  mWallet->setFolder( folder );
  return mWallet->writePassword( str_id, mPassword ) == 0;
}

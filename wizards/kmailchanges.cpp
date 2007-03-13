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
#include <kstandarddirs.h>
#include <kemailsettings.h>
#include <identitymanager.h>
#include <identity.h>
#include <kdebug.h>
#include <kstringhandler.h>

CreateImapAccount::CreateImapAccount( const QString &accountName, const QString &title )
  : KConfigPropagator::Change( title ),
    mAccountName( accountName ), mEnableSieve( false ), mEnableSavePassword( true ),
    mEncryption( None ), mAuthenticationSend( PLAIN ), mSmtpPort( 25 ),
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
    CreateImapAccount( accountName, i18n("Create Disconnected IMAP Account for KMail") )
{
}

void CreateDisconnectedImapAccount::apply()
{
  if ( mEmail.isEmpty() ) mEmail = mUser + "@" + mServer;

  KConfig c( "kmailrc" );
  c.setGroup( "General" );
  c.writeEntry( "Default domain", mDefaultDomain );
  int accountId;
  if ( mExistingAccountId < 0 ) {
    uint accCnt = c.readNumEntry( "accounts", 0 );
    accountId = accCnt + 1;
    c.writeEntry( "accounts", accountId );
  } else {
    accountId = mExistingAccountId;
  }
  int transportId;
  if ( mExistingTransportId < 0 ) {
    uint transCnt = c.readNumEntry( "transports", 0 );
    transportId = transCnt + 1;
    c.writeEntry( "transports", transportId );
  } else {
    transportId = mExistingTransportId;
  }

  c.setGroup( QString("Account %1").arg( accountId ) );
  int uid;
  if ( mExistingAccountId < 0 ) {
    uid = kapp->random();
    c.writeEntry( "Folder", uid );
  } else {
    uid = c.readNumEntry( "Folder" );
  }
  c.writeEntry( "Id", uid );
  c.writeEntry( "Type", "cachedimap");
  c.writeEntry( "auth", "*");
  c.writeEntry( "Name", mAccountName );
  c.writeEntry( "host", mServer );
  c.writeEntry( "port", "993" );

  c.writeEntry( "login", mUser );

  c.writeEntry( "sieve-support", mEnableSieve ? "true" : "false" );
  if ( !mSieveVacationFileName.isEmpty() )
    c.writeEntry( "sieve-vacation-filename", mSieveVacationFileName );

  if ( mEncryption == SSL ) {
    c.writeEntry( "use-ssl", true );
  } else if ( mEncryption == TLS ) {
    c.writeEntry( "use-tls", true );
  }


  c.setGroup( QString("Folder-%1").arg( uid ) );
  c.writeEntry( "isOpen", true );

  if ( mEnableSavePassword ) {
    c.writeEntry( "pass", KStringHandler::obscure( mPassword ) );
    c.writeEntry( "store-passwd", true );
  }

  c.setGroup( QString("Transport %1").arg( transportId ) );
  c.writeEntry( "name", mAccountName );
  c.writeEntry( "host", mServer );
  c.writeEntry( "type", "smtp" );
  c.writeEntry( "port", mSmtpPort );
  if ( mEncryption == SSL ) {
    c.writeEntry( "encryption", "SSL" );
  } else if ( mEncryption == TLS ) {
    c.writeEntry( "encryption", "TLS" );
  }
  c.writeEntry( "auth", true );
  if ( mAuthenticationSend == PLAIN ) {
    c.writeEntry( "authtype", "PLAIN" );
  } else if ( mAuthenticationSend == LOGIN ) {
    c.writeEntry( "authtype", "LOGIN" );
  }
  c.writeEntry( "user", mUser );
  if ( mEnableSavePassword ) {
    c.writeEntry( "pass", KStringHandler::obscure( mPassword ) );
    c.writeEntry( "storepass", "true" );
  }

  // Write email in "default kcontrol settings", used by IdentityManager
  // if it has to create a default identity.
  KEMailSettings es;
  es.setSetting( KEMailSettings::RealName, mRealName );
  es.setSetting( KEMailSettings::EmailAddress, mEmail );

  KPIM::IdentityManager identityManager;
  if ( !identityManager.allEmails().contains( mEmail ) ) {
    // Not sure how to name the identity. First one is "Default", next one mAccountName, but then...
    // let's use the server name after that.
    QString accountName = mAccountName;
    const QStringList identities = identityManager.identities();
    if ( identities.find( accountName ) != identities.end() ) {
      accountName = mServer;
      int i = 2;
      // And if there's already one, number them
      while ( identities.find( accountName ) != identities.end() ) {
        accountName = mServer + " " + QString::number( i++ );
      }
    }

    KPIM::Identity& identity = identityManager.newFromScratch( accountName );
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
  c.setGroup( "General" );
  uint accCnt = c.readNumEntry( "accounts", 0 );
  c.writeEntry( "accounts", accCnt+1 );

  c.setGroup( QString("Account %1").arg(accCnt+1) );
  int uid = kapp->random();
  c.writeEntry( "Folder", uid );
  c.writeEntry( "Id", uid );
  c.writeEntry( "Type", "imap" );
  c.writeEntry( "auth", true );
  c.writeEntry( "Name", mAccountName );
  c.writeEntry( "host", mServer );

  c.writeEntry( "login", mUser );

  if ( mEnableSavePassword ) {
    c.writeEntry( "pass", KStringHandler::obscure( mPassword ) );
    c.writeEntry( "store-passwd", true );
  }
  c.writeEntry( "port", "993" );

  if ( mEncryption == SSL ) {
    c.writeEntry( "encryption", "SSL" );
  } else if ( mEncryption == TLS ) {
    c.writeEntry( "encryption", "TLS" );
  }

  if ( mAuthenticationSend == PLAIN ) {
    c.writeEntry( "authtype", "PLAIN" );
  } else if ( mAuthenticationSend == LOGIN ) {
    c.writeEntry( "authtype", "LOGIN" );
  }

  c.writeEntry( "sieve-support", mEnableSieve );

  c.setGroup( QString("Folder-%1").arg( uid ) );
  c.writeEntry( "isOpen", true );
}

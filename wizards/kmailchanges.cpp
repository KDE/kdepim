/*
    This file is part of kdepim.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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

CreateDisconnectedImapAccount::CreateDisconnectedImapAccount( const QString &accountName )
  : KConfigPropagator::Change( i18n("Create Disconnected IMAP Account for KMail") ),
    mAccountName( accountName ), mEnableSieve( false ), mEnableSavePassword( true ),
    mEncryptionReceive( None ), mCustomWriter( 0 )
{
}

CreateDisconnectedImapAccount::~CreateDisconnectedImapAccount()
{
  delete mCustomWriter;
}

void CreateDisconnectedImapAccount::setServer( const QString &s )
{
  mServer = s;
}

void CreateDisconnectedImapAccount::setUser( const QString &s )
{
  mUser = s;
}

void CreateDisconnectedImapAccount::setPassword( const QString &s )
{
  mPassword = s;
}

void CreateDisconnectedImapAccount::setRealName( const QString &s )
{
  mRealName = s;
}

void CreateDisconnectedImapAccount::setEmail( const QString &s )
{
  mEmail = s;
}

void CreateDisconnectedImapAccount::enableSieve( bool b )
{
  mEnableSieve = b;
}

void CreateDisconnectedImapAccount::enableSavePassword( bool b )
{
  mEnableSavePassword = b;
}

void CreateDisconnectedImapAccount::setEncryptionReceive(
  CreateDisconnectedImapAccount::Encryption e )
{
  mEncryptionReceive = e;
}

void CreateDisconnectedImapAccount::setCustomWriter(
  CreateDisconnectedImapAccount::CustomWriter *writer )
{
  mCustomWriter = writer;
}

void CreateDisconnectedImapAccount::apply()
{
  if ( mEmail.isEmpty() ) mEmail = mUser + "@" + mServer;

  KConfig c( "kmailrc" );
  c.setGroup( "General" );
  uint accCnt = c.readNumEntry( "accounts", 0 );
  c.writeEntry( "accounts", accCnt + 1 );
  uint transCnt = c.readNumEntry( "transports", 0 );
  c.writeEntry( "transports", transCnt + 1 );

  c.setGroup( QString("Account %1").arg( accCnt + 1) );
  int uid = kapp->random();
  c.writeEntry( "Folder", uid );
  c.writeEntry( "Id", uid );
  c.writeEntry( "Type", "cachedimap");
  c.writeEntry( "auth", "*");
  c.writeEntry( "Name", mAccountName );
  c.writeEntry( "host", mServer );

  c.writeEntry( "login", mUser );

  c.writeEntry( "sieve-support", mEnableSieve ? "true" : "false" );

  if ( mEncryptionReceive == SSL ) {
    c.writeEntry( "use-ssl", true );
  } else if ( mEncryptionReceive == TLS ) {
    c.writeEntry( "use-tls", true );
  }


  c.setGroup( QString("Folder-%1").arg( uid ) );
  c.writeEntry( "isOpen", true );

  if ( mEnableSavePassword ) {
    c.writeEntry( "pass", KStringHandler::obscure( mPassword ) );
    c.writeEntry( "store-passwd", true );
  }
  c.writeEntry( "port", "993" );


  c.setGroup( QString("Transport %1").arg( transCnt + 1 ) );
  c.writeEntry( "name", mAccountName );
  c.writeEntry( "host", mServer );
  c.writeEntry( "type", "smtp" );
  c.writeEntry( "port", "465" );
  c.writeEntry( "encryption", "SSL" );
  c.writeEntry( "auth", true );
  c.writeEntry( "authtype", "PLAIN" );
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
    KPIM::Identity& identity = identityManager.newFromScratch( mAccountName );
    identity.setFullName( mRealName );
    identity.setEmailAddr( mEmail );
    identityManager.commit();
  }

  if ( mCustomWriter ) mCustomWriter->write( c, uid );
}

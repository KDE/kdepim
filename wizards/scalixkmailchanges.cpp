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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "scalixkmailchanges.h"

#include "scalixconfig.h"
#include "kmailchanges.h"

#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>

class ScalixCustomWriter : public CreateDisconnectedImapAccount::CustomWriter
{
  void writeFolder( KConfig &c, int id )
  {
    c.setGroup( "IMAP Resource" );
    c.writeEntry( "TheIMAPResourceAccount", id );
    c.writeEntry( "TheIMAPResourceFolderParent", QString(".%1.directory/INBOX").arg( id ) );
    c.writeEntry( "HideGroupwareFolders", false );
  }
  void writeIds( int, int ) {}
};

void createKMailChanges( KConfigPropagator::Change::List& changes )
{
  KConfigPropagator::ChangeConfig *c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "Groupware";
  c->name = "Enabled";
  c->value = "true";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "Groupware";
  c->name = "AutoAccept";
  c->value = "false";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "Groupware";
  c->name = "AutoDeclConflict";
  c->value = "false";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "Groupware";
  c->name = "LegacyMangleFromToHeaders";
  c->value = "true";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "Groupware";
  c->name = "LegacyBodyInvites";
  c->value = "true";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "IMAP Resource";
  c->name = "Enabled";
  c->value = "true";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "IMAP Resource";
  c->name = "TheIMAPResourceEnabled";
  c->value = "true";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "IMAP Resource";
  c->name = "TheIMAPResourceStorageFormat";
  c->value = "IcalVcard";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "IMAP Resource";
  c->name = "Folder Language";
  c->value = "0";
  changes.append( c );

  // Don't show the account wizard as we created an account already
  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "AccountWizard";
  c->name = "ShowOnStartup";
  c->value = "false";
  changes.append( c );

  CreateDisconnectedImapAccount *account =
    new CreateDisconnectedImapAccount( i18n("Scalix Server") );

  account->setServer( ScalixConfig::self()->server() );
  account->setUser( ScalixConfig::self()->user() );
  account->setPassword( ScalixConfig::self()->password() );
  account->setRealName( ScalixConfig::self()->realName() );
  account->setEmail( ScalixConfig::self()->eMail() );
  if ( ScalixConfig::self()->security() == ScalixConfig::None )
    account->setPort( 143 );
  else
    account->setPort( 993 );

  account->enableSieve( false );
  account->enableSavePassword( ScalixConfig::self()->savePassword() );

  switch ( ScalixConfig::self()->security() ) {
    case ScalixConfig::None:
      account->setEncryption( CreateImapAccount::None );
      break;
    case ScalixConfig::TLS:
      account->setEncryption( CreateImapAccount::TLS );
      break;
    case ScalixConfig::SSL:
      account->setEncryption( CreateImapAccount::SSL );
      break;
  }

  switch ( ScalixConfig::self()->authentication() ) {
    case ScalixConfig::Password:
      account->setAuthentication( CreateImapAccount::NONE );
      break;
    case ScalixConfig::NTLM_SPA:
      account->setAuthentication( CreateImapAccount::NTLM_SPA );
      break;
    case ScalixConfig::GSSAPI:
      account->setAuthentication( CreateImapAccount::GSSAPI );
      break;
    case ScalixConfig::DIGEST_MD5:
      account->setAuthentication( CreateImapAccount::DIGEST_MD5 );
      break;
    case ScalixConfig::CRAM_MD5:
      account->setAuthentication( CreateImapAccount::CRAM_MD5 );
      break;
  }

  account->setAuthenticationSend( CreateDisconnectedImapAccount::PLAIN );
  account->setSmtpPort( 465 );
  account->setDefaultDomain( ScalixConfig::self()->server() );
  account->enableLocalSubscription( false );
  account->setGroupwareType( CreateDisconnectedImapAccount::GroupwareScalix );

  account->setCustomWriter( new ScalixCustomWriter );

  changes.append( account );
}

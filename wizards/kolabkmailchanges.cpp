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

#include "kolabkmailchanges.h"

#include "kolabconfig.h"
#include "kmailchanges.h"

#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>

class KolabCustomWriter : public CreateDisconnectedImapAccount::CustomWriter
{
  void writeFolder( KConfig &c, int id )
  {
    c.setGroup( "IMAP Resource" );
    c.writeEntry( "TheIMAPResourceAccount", id );
    c.writeEntry( "TheIMAPResourceFolderParent", QString(".%1.directory/INBOX").arg( id ) );
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
  c->value = KolabConfig::self()->kolab1Legacy() ? "IcalVcard" : "XML";
  changes.append( c );

  c = new KConfigPropagator::ChangeConfig;
  c->file = "kmailrc";
  c->group = "IMAP Resource";
  c->name = "Folder Language";
  c->value = "0"; // TODO: Fix the language
  changes.append( c );

  QString email;
  QString defaultDomain = KolabConfig::self()->server();
  const QString server = KolabConfig::self()->server();
  QString user = KolabConfig::self()->user();
  int pos = user.find( "@" );
  // with kolab the userid _is_ the full email
  if ( pos > 0 ) {
    // The user typed in a full email address. Assume it's correct
    email = user;
    const QString h = user.mid( pos+1 );
    if ( !h.isEmpty() )
      // The user did type in a domain on the email address. Use that
      defaultDomain = h;
  }
  else
    // Construct the email address. And use it for the username also
    user = email = user+"@"+KolabConfig::self()->server();

  if ( KolabConfig::self()->useOnlineForNonGroupware() ) {
    c = new KConfigPropagator::ChangeConfig;
    c->file = "kmailrc";
    c->group = "IMAP Resource";
    c->name = "ShowOnlyGroupwareFoldersForGroupwareAccount";
    c->value = "true";
    changes.append( c );

    CreateOnlineImapAccount *account = new CreateOnlineImapAccount( i18n("Kolab Server Mail") );

    account->setServer( server );
    account->setUser( user );
    account->setPassword( KolabConfig::self()->password() );
    account->setRealName( KolabConfig::self()->realName() );
    account->setEmail( email );
    account->enableSieve( true );
    account->enableSavePassword( KolabConfig::self()->savePassword() );
    account->setEncryption( CreateImapAccount::SSL );
    account->setDefaultDomain( defaultDomain );

    changes.append( account );
  }

  CreateDisconnectedImapAccount *account =
    new CreateDisconnectedImapAccount( i18n("Kolab Server") );

  account->setServer( server );
  account->setUser( user );
  account->setPassword( KolabConfig::self()->password() );
  account->setRealName( KolabConfig::self()->realName() );
  account->setEmail( email );
  account->enableSieve( true );
  account->setSieveVacationFileName( "kolab-vacation.siv" );
  account->enableSavePassword( KolabConfig::self()->savePassword() );
  account->setEncryption( CreateImapAccount::SSL );
  account->setAuthenticationSend( CreateDisconnectedImapAccount::PLAIN );
  account->setSmtpPort( 465 );
  account->setDefaultDomain( defaultDomain );
  account->enableLocalSubscription( KolabConfig::self()->useOnlineForNonGroupware() );

  account->setCustomWriter( new KolabCustomWriter );

  changes.append( account );
}

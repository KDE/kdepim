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
  c->value = "false";
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
  c->name = "Folder Language";
  c->value = "0"; // TODO: Fix the language
  changes.append( c );

  CreateDisconnectedImapAccount *account =
    new CreateDisconnectedImapAccount( i18n("Kolab Server") );

  QString email;
  const QString server = KolabConfig::self()->server();
  QString user = KolabConfig::self()->user();
  int pos = user.find( "@" );
  // with kolab the userid _is_ the full email
  if ( pos > 0 )
    // The user typed in a full email address. Assume it's correct
    email = user;
  else
    // Construct the email address. And use it for the username also
    user = email = user+"@"+KolabConfig::self()->server();


  account->setServer( server );
  account->setUser( user );
  account->setPassword( KolabConfig::self()->password() );
  account->setRealName( KolabConfig::self()->realName() );
  account->setEmail( email );
  account->enableSieve( true );
  account->enableSavePassword( KolabConfig::self()->savePassword() );
  account->setEncryption( CreateDisconnectedImapAccount::SSL );
  account->setAuthenticationSend( CreateDisconnectedImapAccount::PLAIN );
  account->setSmtpPort( 465 );

  account->setCustomWriter( new KolabCustomWriter );

  changes.append( account );
}

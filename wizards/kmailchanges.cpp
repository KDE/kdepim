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
#include "kolabconfig.h"

#include <kapplication.h>
#include <klocale.h>


class CreateDisconnectedImapAccount : public KConfigPropagator::Change
{
  public:
    CreateDisconnectedImapAccount()
      : KConfigPropagator::Change( i18n("Create Disconnected IMAP Account for KMail") )
    {
    }

    // This sucks as this is a 1:1 copy from KMAccount::encryptStr()
    static QString encryptStr(const QString& aStr)
    {
      QString result;
      for (uint i = 0; i < aStr.length(); i++)
        result += (aStr[i].unicode() < 0x20) ? aStr[i] :
          QChar(0x1001F - aStr[i].unicode());
      return result;

    }

    void apply()
    {
      KConfig c( "kmailrc" );
      c.setGroup( "General" );
      uint accCnt = c.readNumEntry( "accounts", 0 );
      c.writeEntry( "accounts", accCnt+1 );
      uint transCnt = c.readNumEntry( "transports", 0 );
      c.writeEntry( "transports", transCnt+1 );

      c.setGroup( QString("Account %1").arg(accCnt+1) );
      int uid = kapp->random();
      c.writeEntry( "Folder", uid );
      c.writeEntry( "Id", uid );
      c.writeEntry( "Type", "cachedimap");
      c.writeEntry( "auth", "*");
      c.writeEntry( "Name", "Kolab Server" );
      c.writeEntry( "host", KolabConfig::self()->server() );

      c.writeEntry( "login", KolabConfig::self()->user() );

      if ( KolabConfig::self()->savePassword() ) {
        c.writeEntry( "pass", encryptStr(KolabConfig::self()->password()) );
        c.writeEntry( "store-passwd", true );
      }

      c.setGroup( QString("Transport %1").arg(transCnt+1) );
      c.writeEntry( "name", "Kolab Server" );
      c.writeEntry( "host", KolabConfig::self()->server() );

      // ### Should be done using the KPIM identity manager

      if ( !c.hasGroup("Identity") )
      {
        c.setGroup( "Identity" );
      }
      else
      {
        int i = 0;
        while (c.hasGroup(QString("Identity #%1").arg(i)))
          ++i;

        c.setGroup( QString("Identity #%1").arg(++i) );
      }

      QString user = KolabConfig::self()->user();
      int pos = user.find( "@" );
      if ( pos > 0 ) user = user.left( pos );

      c.writeEntry("Email Address", user+"@"+KolabConfig::self()->server() );
      c.writeEntry("Name", KolabConfig::self()->realName() );
      c.writeEntry("uoid", kapp->random() );

      // This needs to be done here, since it reference just just generated id
      c.setGroup( "IMAP Resource" );
      c.writeEntry("TheIMAPResourceFolderParent", QString(".%1.directory/INBOX").arg( uid ));
   }

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

  changes.append( new CreateDisconnectedImapAccount );
}

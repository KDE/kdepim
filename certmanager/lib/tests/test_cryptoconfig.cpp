/*  -*- mode: C++; c-file-style: "gnu" -*-
    test_cryptoconfig.cpp

    This file is part of libkleopatra's test suite.
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <backends/qgpgme/qgpgmecryptoconfig.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <iostream>
using namespace std;

int main( int argc, char** argv ) {

  KAboutData aboutData( "test_cryptoconfig", "CryptoConfig Test", "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KApplication app( false, false );

  const Kleo::CryptoConfig * config = new QGpgMECryptoConfig();

  // Dynamic querying of the options
  cout << "Components:" << endl;
  QStringList components = config->componentList();

  for( QStringList::Iterator compit = components.begin(); compit != components.end(); ++compit ) {
      cout << "Component " << (*compit).local8Bit() << ":" << endl;
      const Kleo::CryptoConfigComponent* comp = config->component( *compit );
      assert( comp );
      QStringList groups = comp->groupList();
      for( QStringList::Iterator groupit = groups.begin(); groupit != groups.end(); ++groupit ) {
          cout << " Group [" << (*groupit).local8Bit() << "] :" << endl;
          const Kleo::CryptoConfigGroup* group = comp->group( *groupit );
          assert( group );
          QStringList entries = group->entryList();
          cout << "  " << (entries.join( " " )).local8Bit() << endl;
          // ...
      }
  }

  // Static querying of a single option
  const Kleo::CryptoConfigEntry* entry = config->entry( "dirmngr", "<nogroup>", "ldaptimeout" );
  if ( entry ) {
      assert( entry->dataType() == Kleo::CryptoConfigEntry::DataType_UInt );
      cout << "LDAP timeout: " << entry->uintValue() << " seconds." << endl;
  }
  else
      cout << "Entry dirmngr/<nogroup>/ldaptimeout not found" << endl;

  // TODO setting options

  cout << "Done." << endl;
}

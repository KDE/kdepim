/*  -*- mode: C++; c-file-style: "gnu"; c-basic-offset: 2 -*-
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
      const Kleo::CryptoConfigGroup* group = comp->group( *groupit );
      assert( group );
      cout << " Group " << (*groupit).local8Bit() << ": descr=" << group->description().local8Bit()
           << " level=" << group->level() << endl;
      QStringList entries = group->entryList();
      for( QStringList::Iterator entryit = entries.begin(); entryit != entries.end(); ++entryit ) {
        const Kleo::CryptoConfigEntry* entry = group->entry( *entryit );
        assert( entry );
        cout << "  Entry " << (*entryit).local8Bit() << ":"
             << " descr=\"" << entry->description().local8Bit() << "\"";
        if ( !entry->isList() )
          switch( entry->dataType() ) {
          case Kleo::CryptoConfigEntry::DataType_Bool:
            cout << " boolean value=" << ( entry->boolValue()?"true":"false");
            break;
          case Kleo::CryptoConfigEntry::DataType_Int:
            cout << " int value=" << entry->intValue();
            break;
          case Kleo::CryptoConfigEntry::DataType_UInt:
            cout << " uint value=" << entry->uintValue();
            break;
          case Kleo::CryptoConfigEntry::DataType_URL:
            cout << " URL value=" << entry->urlValue().prettyURL().local8Bit();
            // fallthrough
          case Kleo::CryptoConfigEntry::DataType_Path:
            // fallthrough
          case Kleo::CryptoConfigEntry::DataType_String:

            cout << " string value=" << entry->stringValue().local8Bit();
            break;
          }
        else // lists
          switch( entry->dataType() ) {
          case Kleo::CryptoConfigEntry::DataType_Bool: {
            QValueList<bool> lst = entry->boolValueList();
            QString str;
            for( QValueList<bool>::Iterator it = lst.begin(); it != lst.end(); ++it ) {
              str += ( *it ) ? "true" : "false";
            }
            cout << " boolean values=" << str.local8Bit();
            break;
          }
          case Kleo::CryptoConfigEntry::DataType_Int: {
            QValueList<int> lst = entry->intValueList();
            QString str;
            for( QValueList<int>::Iterator it = lst.begin(); it != lst.end(); ++it ) {
              str += QString::number( *it );
            }
            cout << " int values=" << str.local8Bit();
            break;
          }
          case Kleo::CryptoConfigEntry::DataType_UInt: {
            QValueList<uint> lst = entry->uintValueList();
            QString str;
            for( QValueList<uint>::Iterator it = lst.begin(); it != lst.end(); ++it ) {
              str += QString::number( *it );
            }
            cout << " uint values=" << str.local8Bit();
            break;
          }
          case Kleo::CryptoConfigEntry::DataType_URL:
            // fallthrough
          case Kleo::CryptoConfigEntry::DataType_Path:
            // fallthrough
          case Kleo::CryptoConfigEntry::DataType_String: {
            QStringList lst = entry->stringValueList();
            cout << " string values=" << lst.join(" ").local8Bit();
            break;
          }
          }
        cout << endl;
      }
      // ...
    }
  }

  {
    // Static querying of a single boolean option
    const Kleo::CryptoConfigEntry* entry = config->entry( "dirmngr", "<nogroup>", "ldaptimeout" );
    if ( entry ) {
      assert( entry->dataType() == Kleo::CryptoConfigEntry::DataType_UInt );
      uint val = entry->uintValue();
      cout << "LDAP timeout: " << val << " seconds." << endl;

      // Test setting the option directly, then querying again
      system( "echo 'ldaptimeout:101' | gpgconf --change-options dirmngr" );

      // Clear cached values!
      // Hmm, should clear() be const? It sounds strange, but since it's only about discarding cached data...
      // Bah, I guess config shouldn't be a const pointer.
      const_cast<Kleo::CryptoConfig*>( config )->clear();

      // Check new value
      const Kleo::CryptoConfigEntry* entry = config->entry( "dirmngr", "<nogroup>", "ldaptimeout" );
      assert( entry );
      assert( entry->dataType() == Kleo::CryptoConfigEntry::DataType_UInt );
      cout << "LDAP timeout: " << entry->uintValue() << " seconds." << endl;
      assert( entry->uintValue() == 101 );

      // Reset old value
      QCString sys;
      sys.sprintf( "echo 'ldaptimeout:%d' | gpgconf --change-options dirmngr", val );
      system( sys.data() );

      cout << "LDAP timeout reset to " << val << " seconds." << endl;
    }
    else
      cout << "Entry dirmngr/<nogroup>/ldaptimeout not found" << endl;
  }

  {
    // Static querying of a single string option
    const Kleo::CryptoConfigEntry* entry = config->entry( "dirmngr", "<nogroup>", "log-file" );
    if ( entry ) {
      assert( entry->dataType() == Kleo::CryptoConfigEntry::DataType_Path );
      QString val = entry->stringValue();
      cout << "Log-file: " << val.local8Bit() << endl;

      // Test setting the option directly, then querying again
      system( "echo 'log-file:\"/tmp/log' | gpgconf --change-options dirmngr" );

      // Clear cached values!
      // Hmm, should clear() be const? It sounds strange, but since it's only about discarding cached data...
      // Bah, I guess config shouldn't be a const pointer.
      const_cast<Kleo::CryptoConfig*>( config )->clear();

      // Check new value
      const Kleo::CryptoConfigEntry* entry = config->entry( "dirmngr", "<nogroup>", "log-file" );
      assert( entry );
      assert( entry->dataType() == Kleo::CryptoConfigEntry::DataType_Path );
      cout << "Log-file: " << entry->stringValue().local8Bit() << endl;
      assert( entry->stringValue() == "/tmp/log" );

      // Reset old value
      QString arg( val );
      if ( !arg.isEmpty() )
        arg.prepend( '"' );
      QCString sys;
      sys.sprintf( "echo 'log-file:%s' | gpgconf --change-options dirmngr", arg.local8Bit().data() );
      system( sys.data() );

      cout << "Log-file reset to " << val.local8Bit() << endl;
    }
    else
      cout << "Entry dirmngr/<nogroup>/log-file not found" << endl;
  }

  // TODO setting options

  cout << "Done." << endl;
}

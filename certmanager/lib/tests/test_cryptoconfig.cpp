/*
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

  Kleo::CryptoConfig * config = new QGpgMECryptoConfig();

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
      cout << " Group " << (*groupit).local8Bit() << ": descr=\"" << group->description().local8Bit() << "\""
           << " level=" << group->level() << endl;
      QStringList entries = group->entryList();
      for( QStringList::Iterator entryit = entries.begin(); entryit != entries.end(); ++entryit ) {
        const Kleo::CryptoConfigEntry* entry = group->entry( *entryit );
        assert( entry );
        cout << "  Entry " << (*entryit).local8Bit() << ":"
             << " descr=\"" << entry->description().local8Bit() << "\""
             << " " << ( entry->isSet() ? "is set" : "is not set" );
        if ( !entry->isList() )
          switch( entry->argType() ) {
          case Kleo::CryptoConfigEntry::ArgType_None:
            break;
          case Kleo::CryptoConfigEntry::ArgType_Int:
            cout << " int value=" << entry->intValue();
            break;
          case Kleo::CryptoConfigEntry::ArgType_UInt:
            cout << " uint value=" << entry->uintValue();
            break;
          case Kleo::CryptoConfigEntry::ArgType_URL:
            cout << " URL value=" << entry->urlValue().prettyURL().local8Bit();
            // fallthrough
          case Kleo::CryptoConfigEntry::ArgType_Path:
            // fallthrough
          case Kleo::CryptoConfigEntry::ArgType_String:

            cout << " string value=" << entry->stringValue().local8Bit();
            break;
          }
        else // lists
          switch( entry->argType() ) {
          case Kleo::CryptoConfigEntry::ArgType_None: {
            cout << " set " << entry->numberOfTimesSet() << " times";
            break;
          }
          case Kleo::CryptoConfigEntry::ArgType_Int: {
            QValueList<int> lst = entry->intValueList();
            QString str;
            for( QValueList<int>::Iterator it = lst.begin(); it != lst.end(); ++it ) {
              str += QString::number( *it );
            }
            cout << " int values=" << str.local8Bit();
            break;
          }
          case Kleo::CryptoConfigEntry::ArgType_UInt: {
            QValueList<uint> lst = entry->uintValueList();
            QString str;
            for( QValueList<uint>::Iterator it = lst.begin(); it != lst.end(); ++it ) {
              str += QString::number( *it );
            }
            cout << " uint values=" << str.local8Bit();
            break;
          }
          case Kleo::CryptoConfigEntry::ArgType_URL:
            // fallthrough
          case Kleo::CryptoConfigEntry::ArgType_Path:
            // fallthrough
          case Kleo::CryptoConfigEntry::ArgType_String: {
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
    Kleo::CryptoConfigEntry* entry = config->entry( "dirmngr", "LDAP", "ldaptimeout" );
    if ( entry ) {
      assert( entry->argType() == Kleo::CryptoConfigEntry::ArgType_UInt );
      uint val = entry->uintValue();
      cout << "LDAP timeout: " << val << " seconds." << endl;

      // Test setting the option directly, then querying again
      //system( "echo 'ldaptimeout:101' | gpgconf --change-options dirmngr" );
      // Now let's do it with the C++ API instead
      entry->setUIntValue( 101 );
      assert( entry->isDirty() );
      config->sync( true );

      // Clear cached values!
      config->clear();

      // Check new value
      Kleo::CryptoConfigEntry* entry = config->entry( "dirmngr", "LDAP", "ldaptimeout" );
      assert( entry );
      assert( entry->argType() == Kleo::CryptoConfigEntry::ArgType_UInt );
      cout << "LDAP timeout: " << entry->uintValue() << " seconds." << endl;
      assert( entry->uintValue() == 101 );

      // Set to default
      entry->resetToDefault();
      assert( entry->isDirty() );
      assert( !entry->isSet() );
      config->sync( true );
      config->clear();

      // Check value
      entry = config->entry( "dirmngr", "LDAP", "ldaptimeout" );
      assert( !entry->isDirty() );
      assert( !entry->isSet() );
      cout << "LDAP timeout reset to default, " << val << " seconds." << endl;

      // Reset old value
      entry->setUIntValue( val );
      assert( entry->isDirty() );
      config->sync( true );

      cout << "LDAP timeout reset to initial " << val << " seconds." << endl;
    }
    else
      cout << "Entry dirmngr/LDAP/ldaptimeout not found" << endl;
  }

  {
    // Static querying of a single string option
    Kleo::CryptoConfigEntry* entry = config->entry( "dirmngr", "Debug", "log-file" );
    if ( entry ) {
      assert( entry->argType() == Kleo::CryptoConfigEntry::ArgType_Path );
      QString val = entry->stringValue();
      cout << "Log-file: " << val.local8Bit() << endl;

      // Test setting the option directly, then querying again
      //system( "echo 'log-file:\"/tmp/test%3a%e5ä' | gpgconf --change-options dirmngr" );
      // Now let's do it with the C++ API instead
      entry->setStringValue( "/tmp/test:%e5ä" );
      assert( entry->isDirty() );
      config->sync( true );

      // Let's see how it prints it
      system( "gpgconf --list-options dirmngr | grep log-file" );

      // Clear cached values!
      config->clear();

      // Check new value
      Kleo::CryptoConfigEntry* entry = config->entry( "dirmngr", "Debug", "log-file" );
      assert( entry );
      assert( entry->argType() == Kleo::CryptoConfigEntry::ArgType_Path );
      cout << "Log-file: " << entry->stringValue().local8Bit() << endl;
      // This is what it should be, but gpgconf escapes wrongly the arguments (aegypten issue90)
      //assert( entry->stringValue() == "/tmp/test:%e5ä" ); // (or even with %e5 decoded)

      // Reset old value
#if 0
      QString arg( val );
      if ( !arg.isEmpty() )
        arg.prepend( '"' );
      QCString sys;
      sys.sprintf( "echo 'log-file:%s' | gpgconf --change-options dirmngr", arg.local8Bit().data() );
      system( sys.data() );
#endif
      entry->setStringValue( val );
      assert( entry->isDirty() );
      config->sync( true );

      cout << "Log-file reset to " << val.local8Bit() << endl;
    }
    else
      cout << "Entry dirmngr/Debug/log-file not found" << endl;
  }

  // TODO setting options

  cout << "Done." << endl;
}

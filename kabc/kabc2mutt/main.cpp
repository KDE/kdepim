/*
    KAbc2Mutt

    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id$
*/

#include <kapplication.h>
#include <kabc/stdaddressbook.h>
#include <kabc/distributionlist.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <qregexp.h>

#include <stdio.h>

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "kabc2mutt",
                        i18n( "kabc - mutt converter" ), "0.1" );

    KApplication app( false, false );

    KABC::AddressBook *ab = KABC::StdAddressBook::self();

    // print all addressees
    KABC::AddressBook::Iterator it;
    for ( it = ab->begin(); it != ab->end(); ++it ) {
      if ( (*it).preferredEmail().isEmpty() )
        continue;

      QString key = (*it).givenName().left( 3 ) + (*it).familyName().left( 3 );

      printf( "alias %s\t%s %s <%s>\n", key.local8Bit().data(),
              (*it).givenName().local8Bit().data(),
              (*it).familyName().local8Bit().data(),
              (*it).preferredEmail().local8Bit().data() );
    }

    // print all ditribution lists
    KABC::DistributionListManager manager( ab );
    manager.load();

    QStringList dists = manager.listNames();
    for ( QStringList::Iterator it = dists.begin(); it != dists.end(); ++it ) {
      KABC::DistributionList *list = manager.list( *it );
      if ( list ) {
        QStringList emails = list->emails();
        printf( "alias %s\t %s\n",
                (*it).replace( QRegExp( " " ), "_" ).local8Bit().data(),
                emails.join( "," ).local8Bit().data() );
    }
  }

  return 0;
}

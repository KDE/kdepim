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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    $Id$
*/

#include <kapplication.h>
#include <kabc/stdaddressbook.h>
#include <kabc/distributionlist.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <qregexp.h>

#include <stdio.h>

static KCmdLineOptions k2moptions[] =
{
    { "query <substring>", I18N_NOOP("Only show contacts where name or address matches <substring>"), 0 },
    { "format <format>", I18N_NOOP("Default format is 'alias'. 'query' returns email<tab>name<tab>, as needed by mutt's query_command"), "alias" },
    KCmdLineLastOption
};

int main( int argc, char **argv )
{
    KApplication::disableAutoDcopRegistration();
    KCmdLineArgs::init( argc, argv, "kabc2mutt",
                        i18n( "kabc - mutt converter" ), "0.1" );
    KCmdLineArgs::addCmdLineOptions( k2moptions );

    KApplication app( false, false );

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    // Handle --format option
    QCString formatString = args->getOption("format");
    enum { Aliases, QueryCommand } format;
    if ( formatString == "query" )
        format = QueryCommand;
    else
        format = Aliases;

    // Handle --query option
    QString subString = QString::fromLocal8Bit( args->getOption("query") );
    if ( !subString.isEmpty() )
    {
        // Mutt wants a first line with some status message on it
        // See http://mutt.org/doc/manual/manual-4.html#ss4.5
        printf( "%s\n", i18n("Searching KDE addressbook...").local8Bit().data() );
    }

    KABC::AddressBook *ab = KABC::StdAddressBook::self();

    // print addressees
    KABC::AddressBook::Iterator it;
    for ( it = ab->begin(); it != ab->end(); ++it ) {
      if ( (*it).preferredEmail().isEmpty() )
        continue;
      QString name = (*it).givenName() + ' ' + (*it).familyName();

      if ( !subString.isEmpty() )
      {
        bool match = (name.find(subString) > -1) || ((*it).preferredEmail().find(subString) > -1 );
        if ( !match )
          continue;
      }

      if ( format == Aliases )
      {
          QString key = (*it).givenName().left( 3 ) + (*it).familyName().left( 3 );

          printf( "alias %s\t%s <%s>\n", key.local8Bit().data(),
                  name.local8Bit().data(),
                  (*it).preferredEmail().local8Bit().data() );
      } else {
          printf( "%s\t%s\t\n", (*it).preferredEmail().local8Bit().data(),
                  name.local8Bit().data() );
      }
    }

    // print all distribution lists
    KABC::DistributionListManager manager( ab );
    manager.load();

    QStringList dists = manager.listNames();
    for ( QStringList::Iterator it = dists.begin(); it != dists.end(); ++it ) {
      KABC::DistributionList *list = manager.list( *it );
      if ( list ) {

        if ( !subString.isEmpty() )
        {
          bool match = ((*it).find(subString) > -1);
          if ( !match )
            continue;
        }

        QStringList emails = list->emails();
        if ( format == Aliases )
            printf( "alias %s\t %s\n",
                    (*it).replace( QRegExp( " " ), "_" ).local8Bit().data(),
                    emails.join( "," ).local8Bit().data() );
        else
            printf( "%s\t%s\t\n",
                    emails.join( "," ).local8Bit().data(),
                    (*it).local8Bit().data() );
    }
  }

  return 0;
}

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

#include <iostream>

static const char version[] = "0.2";
static const char appName[] = "kabc2mutt";
static const char programName[] = I18N_NOOP( "kabc2mutt" );
static const char description[] = I18N_NOOP( "kabc - mutt converter" );

static KCmdLineOptions k2moptions[] =
{
  { "query <substring>", I18N_NOOP( "Only show contacts where name or address matches <substring>" ), 0 },
  { "format <format>", I18N_NOOP( "Default format is 'alias'. 'query' returns email<tab>name<tab>, as needed by mutt's query_command" ), "alias" },
  { "ignore-case", I18N_NOOP( "Make queries case insensitive" ), 0 },
  { "all-addresses", I18N_NOOP( "Return all mail addresses, not just the preferred one" ), 0},
  KCmdLineLastOption
};


static std::ostream & operator<< ( std::ostream &os, const QString &s );

int main( int argc, char **argv )
{
  enum format_t { Aliases, QueryCommand };

  KApplication::disableAutoDcopRegistration();
  KCmdLineArgs::init( argc, argv, appName, programName, description, version );
  KCmdLineArgs::addCmdLineOptions( k2moptions );

  KApplication app( false, false );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // Handle --format option
  const format_t format = (args->getOption("format") == "query") ? QueryCommand : Aliases;

  // Handle --all-addresses option
  const bool all = args->isSet( "all-addresses" );

  // Handle --query option
  const QString subString = QString::fromLocal8Bit( args->getOption( "query" ) );
  if ( !subString.isEmpty() ) {
    // Mutt wants a first line with some status message on it
    // See http://mutt.org/doc/manual/manual-4.html#ss4.5
    std::cout << i18n( "Searching KDE addressbook" ) << std::endl;
  }

  // Handle --ignore-case option
  const bool cs = !args->isSet( "ignore-case" );

  KABC::AddressBook *ab = KABC::StdAddressBook::self();
  KABC::StdAddressBook::setAutomaticSave( false );

  // print addressees
  KABC::AddressBook::ConstIterator iaddr;
  for ( iaddr = ab->begin(); iaddr != ab->end(); ++iaddr ) {
    const QString name = (*iaddr).givenName() + ' ' + (*iaddr).familyName();
    if ( !subString.isEmpty() ) {
      bool match = (name.find(subString, 0, cs) > -1) || ((*iaddr).preferredEmail().find(subString, 0, cs) > -1 );
      if ( !match )
        continue;
    }

    const QStringList &allAddresses = (*iaddr).emails();
    QStringList::const_iterator from, to;
    bool multiple = false;

    if ( all ) {
      // use all entries
      multiple = allAddresses.size() > 1;
      from = allAddresses.begin();
      to = allAddresses.end();
    } else {
      // use only the first entry, the one returned by preferredEmail()
      from = to = allAddresses.begin();  // start with empty list
      if ( to != allAddresses.end() )
        ++to;
    }

    size_t index = 0;
    if ( format == Aliases ) {
      const QString key = (*iaddr).givenName().left( 3 ) + (*iaddr).familyName().left( 3 );
      while ( from != to ) {
        std::cout << "alias " << key;
        if ( index )
          std::cout << index;
        std::cout << '\t' << name << " <" << (*from) << '>' << std::endl;
        ++index;
        ++from;
      }
    } else {
      while ( from != to ) {
        std::cout << (*from) << '\t' << name;
        if ( multiple ) {
          if ( index )
            std::cout << "\t#" << index;
          else
            std::cout << '\t' << i18n("preferred");
          ++index;
        }
        std::cout << std::endl;
        ++from;
      }
    }
  }

  // print all distribution lists
  KABC::DistributionListManager manager( ab );
  manager.load();

  QStringList dists = manager.listNames();
  for ( QStringList::Iterator iaddr = dists.begin(); iaddr != dists.end(); ++iaddr ) {
    KABC::DistributionList *list = manager.list( *iaddr );
    if ( list ) {
      if ( !subString.isEmpty() ) {
        bool match = ((*iaddr).find(subString) > -1);
        if ( !match )
          continue;
      }

      QStringList emails = list->emails();
      if ( format == Aliases ) {
        std::cout << "alias " << (*iaddr).replace( QRegExp( " " ), "_" )
                  << '\t' << emails.join( "," ) << std::endl;
      } else {
        std::cout << emails.join( "," ) << '\t' << (*iaddr) << '\t' << std::endl;
      }
    }
  }

  return 0;
}

static std::ostream & operator<< ( std::ostream &os, const QString &s )
{
  os << s.local8Bit().data();
  return os;
}


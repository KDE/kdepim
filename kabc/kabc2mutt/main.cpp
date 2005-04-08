/*
    KAbc2Mutt

    Copyright (c) 2003 - 2004 Tobias Koenig <tokoe@kde.org>

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
*/

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <iostream>

#include "kabc2mutt.h"

static const char version[] = "0.2";
static const char appName[] = "kabc2mutt";
static const char programName[] = I18N_NOOP( "kabc2mutt" );
static const char description[] = I18N_NOOP( "kabc - mutt converter" );

static KCmdLineOptions k2moptions[] =
{
  { "query <substring>", I18N_NOOP( "Only show contacts where name or address matches <substring>" ), 0 },
  { "format <format>", I18N_NOOP( "Default format is 'alias'. 'query' returns email<tab>name<tab>, as needed by mutt's query_command" ), "alias" },
  { "alternate-key-format", I18N_NOOP( "Default key format is 'JohDoe', this option turns it into 'jdoe'" ), 0 },
  { "ignore-case", I18N_NOOP( "Make queries case insensitive" ), 0 },
  { "all-addresses", I18N_NOOP( "Return all mail addresses, not just the preferred one" ), 0},
  KCmdLineLastOption
};


int main( int argc, char **argv )
{
  KApplication::disableAutoDcopRegistration();
  KCmdLineArgs::init( argc, argv, appName, programName, description, version );
  KCmdLineArgs::addCmdLineOptions( k2moptions );

  KApplication app( false, false );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KABC2Mutt *object = new KABC2Mutt( 0 );

  // Handle --format option
  object->setFormat( (args->getOption("format") == "query") ? KABC2Mutt::Query : KABC2Mutt::Aliases );

  // Handle --alternate-key-format option
  object->setAlternateKeyFormat( args->isSet( "alternate-key-format" ) );

  // Handle --all-addresses option
  object->setAllAddresses( args->isSet( "all-addresses" ) );

  // Handle --query option
  const QString subString = QString::fromLocal8Bit( args->getOption( "query" ) );
  if ( !subString.isEmpty() ) {
    // Mutt wants a first line with some status message on it
    // See http://mutt.org/doc/manual/manual-4.html#ss4.5
    std::cout << i18n( "Searching KDE addressbook" ).latin1() << std::endl;
  }
  object->setQuery( subString );

  // Handle --ignore-case option
  object->setIgnoreCase( !args->isSet( "ignore-case" ) );

  object->run();

  return app.exec();
}

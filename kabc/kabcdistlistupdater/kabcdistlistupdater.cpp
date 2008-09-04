/*
    This file is part of libkabc.
    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <kabc/stdaddressbook.h>
#include <libkdepim/distributionlist.h>

static const KCmdLineOptions options[] =
{
  { "disable-autostart", "Disable automatic startup on login", 0 },
  KCmdLineLastOption
};

void convertDistributionLists()
{
  KSimpleConfig cfg( locateLocal( "data", "kabc/distlists" ) );
  const QMap<QString, QString> entryMap = cfg.entryMap( "DistributionLists" );

  if ( entryMap.isEmpty() ) // nothing to convert
    return;

  QMapConstIterator<QString, QString> it;
  for ( it = entryMap.begin(); it != entryMap.end(); ++it ) {
    const QString listName = it.key();
    const QStringList entries = QStringList::split( ',', it.data(), true );

    KPIM::DistributionList distList;
    distList.setUid( KApplication::randomString( 10 ) );
    distList.setName( listName );

    if ( entries.count() > 1 ) {
      for ( uint i = 0; i < entries.count(); i += 2 ) {
        const QString uid = entries[ i ];
        const QString preferredEMail = entries[ i + 1 ];

        distList.insertEntry( uid, preferredEMail );
      }
    }

    KABC::StdAddressBook::self()->insertAddressee( distList );
  }

  KABC::StdAddressBook::self()->save();
}

int main( int argc, char **argv )
{
  KApplication::disableAutoDcopRegistration();

  KAboutData aboutData( "kabcdistlistupdater", "Converter tool for distribution lists", "0.1" );
  aboutData.addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app( false, false );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->isSet( "disable-autostart" ) ) {
    kdDebug() << "Disable autostart." << endl;

    KConfig *config = app.config();
    config->setGroup( "Startup" );
    config->writeEntry( "EnableAutostart", false );
  }

  convertDistributionLists();
}


/*
    This file is part of libkabc.
    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2008 Kevin Kofler <kevin.kofler@chello.at>

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

#include <QtCore/QCoreApplication>

#include <kabc/stdaddressbook.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <krandom.h>
#include <kstandarddirs.h>
#include <libkdepim/distributionlist.h>

void convertDistributionLists()
{
  KConfig cfg( KStandardDirs::locateLocal( "data", "kabc/distlists" ), KConfig::SimpleConfig );
  const QMap<QString, QString> entryMap = cfg.entryMap( "DistributionLists" );

  if ( entryMap.isEmpty() ) // nothing to convert
    return;

  QMap<QString, QString>::ConstIterator it;
  for ( it = entryMap.begin(); it != entryMap.end(); ++it ) {
    const QString listName = it.key();
    const QStringList entries = it.value().split( ',', QString::KeepEmptyParts );

    KPIM::DistributionList distList;
    distList.setUid( KRandom::randomString( 10 ) );
    distList.setName( listName );

    if ( entries.count() > 1 ) {
      for ( int i = 0; i < entries.count(); i += 2 ) {
        const QString uid = entries[ i ];
        const QString preferredEMail = entries[ i + 1 ];

        distList.insertEntry( uid, preferredEMail );
      }
    }

    KABC::StdAddressBook::self()->insertAddressee( distList );
  }

  KABC::StdAddressBook::save();
}

int main( int argc, char **argv )
{
  KAboutData aboutData( "kabcdistlistupdater", QByteArray(), ki18n( "Converter tool for distribution lists" ), "0.1" );
  aboutData.addAuthor( ki18n( "Tobias Koenig" ), ki18n( "Author" ), "tokoe@kde.org" );
  aboutData.addAuthor( ki18n( "Kevin Kofler" ), ki18n( "Porter" ), "kevin.kofler@chello.at" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineOptions options;
  options.add( "disable-autostart", ki18n( "Disable automatic startup on login" ) );
  KCmdLineArgs::addCmdLineOptions( options );

  QCoreApplication app( KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv() );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->isSet( "disable-autostart" ) ) {
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup group( config, "Startup" );
    group.writeEntry( "EnableAutostart", false );
  }

  convertDistributionLists();

  return 0;
}

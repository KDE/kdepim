/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "kde-features.h"
#include "kde-features_parser.h"

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <QFile>
#include <QCoreApplication>
#include <qtextstream.h>

#include <iostream>

static const KCmdLineOptions options[] =
{
  { "+featurelist", "Name of featurelist XML file", 0 },
  KCmdLineLastOption
};

void displayFeature( const Feature &f )
{
  std::cout << "FEATURE: " << f.summary().toLocal8Bit().data() << std::endl;
  Responsible::List r = f.responsibleList();
  Responsible::List::ConstIterator it;
  for( it = r.begin(); it != r.end(); ++it ) {
    std::cout << "  RESPONSIBLE: " << (*it).name().toLocal8Bit().data() << " ("
              << (*it).email().toLocal8Bit().data() << ")" << std::endl;
  }
  std::cout << "  TARGET: " << f.target().toLocal8Bit().data() << std::endl;
  std::cout << "  STATUS: " << f.status().toLocal8Bit().data() << std::endl;
}

void displayCategory( const Category::List &categories )
{
  Category::List::ConstIterator it;
  for( it = categories.begin(); it != categories.end(); ++it ) {
    std::cout << "CATEGORY: " << (*it).name().toLocal8Bit().data() << std::endl;

    Feature::List features = (*it).featureList();
    Feature::List::ConstIterator it2;
    for( it2 = features.begin(); it2 != features.end(); ++it2 ) {
      displayFeature( *it2 );
    }

    displayCategory( (*it).categoryList() );
  }
}

int main( int argc, char **argv )
{
  KAboutData aboutData( "benchmarkfeaturelist",
                        "Benchmark for feature list XML parser",
                        "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData, KCmdLineArgs::CmdLineArgNone );
  KCmdLineArgs::addCmdLineOptions( options );

  QCoreApplication app( KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv() );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->count() != 1 ) {
    args->usage( "Wrong number of arguments." );
  }

  QString filename = QFile::decodeName( args->arg( 0 ) );

  for( int i = 0; i < 1; ++i ) {
    bool ok = false;
    Features features = FeaturesParser::parseFile( filename, &ok );

    if ( !ok ) {
      kError() << "Parse error" << endl;
      return 1;
    }
  }
}

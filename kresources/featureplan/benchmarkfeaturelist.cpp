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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kde-features.h"
#include "kde-features_parser.custom.h"

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <qfile.h>
#include <qtextstream.h>

#include <iostream>

static const KCmdLineOptions options[] =
{
  { "+featurelist", "Name of featurelist XML file", 0 },
  KCmdLineLastOption
};

void displayFeature( Feature *f )
{
  std::cout << "FEATURE: " << f->summary().local8Bit() << std::endl;
  Responsible::List r = f->responsibleList();
  Responsible::List::ConstIterator it;
  for( it = r.begin(); it != r.end(); ++it ) {
    std::cout << "  RESPONSIBLE: " << (*it)->name().local8Bit() << " ("
              << (*it)->email().local8Bit() << ")" << std::endl;
  }
  std::cout << "  TARGET: " << f->target().local8Bit() << std::endl;
  std::cout << "  STATUS: " << f->status().local8Bit() << std::endl;
}

void displayCategory( const QValueList<Category *> categories )
{
  Category::List::ConstIterator it;
  for( it = categories.begin(); it != categories.end(); ++it ) {
    std::cout << "CATEGORY: " << (*it)->name().local8Bit() << std::endl;
    
    Feature::List features = (*it)->featureList();
    Feature::List::ConstIterator it2;
    for( it2 = features.begin(); it2 != features.end(); ++it2 ) {
      displayFeature( *it2 );
    }
  
    displayCategory( (*it)->categoryList() );
  }
}

int main( int argc, char **argv )
{
  KAboutData aboutData( "benchmarkfeaturelist",
                        "Benchmark for feature list XML parser",
                        "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app( false, false );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->count() != 1 ) {
    args->usage( "Wrong number of arguments." );
  }

  QString filename = QFile::decodeName( args->arg( 0 ) );

  for( int i = 0; i < 1; ++i ) {
    FeaturesParser parser;

    Features *features = parser.parseFile( filename );

    if ( !features ) {
      kdError() << "Parse error" << endl;
      return 1;
    }
  }
}

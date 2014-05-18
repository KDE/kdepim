/*
  This file is part of KTnef.

  Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#include "ktnefmain.h"
#include "kdepim-version.h"

#include <k4aboutdata.h>
#include <KApplication>
#include <KCmdLineArgs>
#include <KLocalizedString>

int main( int argc, char *argv[] )
{
  KLocalizedString::setApplicationDomain("ktnef");
  K4AboutData aboutData(
    "ktnef", 0,
    ki18n( "KTnef" ),
    KDEPIM_VERSION,
    ki18n( "Viewer for mail attachments using TNEF format" ),
    K4AboutData::License_GPL,
    ki18n( "Copyright 2000 Michael Goffioul\nCopyright 2012  Allen Winter" ) );
//QT5
#if 0
  aboutData.addAuthor(
    ki18n( "Michael Goffioul" ),
    ki18n( "Author" ),
    "kdeprint@swing.be",
    0 );

  aboutData.addAuthor(
    ki18n( "Allen Winter" ),
    ki18n( "Author, Ported to Qt4/KDE4" ),
    "winter@kde.org",
    0 );
#endif
  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  options.add( "+[file]", ki18n( "An optional argument 'file' " ) );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication a;
  KTNEFMain *tnef = new KTNEFMain();
  tnef->show();

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if ( args->count() > 0 ) {
    tnef->loadFile( args->arg( 0 ) );
  }

  return a.exec();
}

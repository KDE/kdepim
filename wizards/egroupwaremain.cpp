/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "egroupwarewizard.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kglobal.h>

static const KCmdLineOptions options[] =
{
  {"verbose", "Verbose output", 0},
  KCmdLineLastOption
};

int main(int argc,char **argv)
{
  KLocale::setMainCatalogue( "kdepimwizards" );

  KAboutData aboutData( "egroupwarewizard",
                        "eGroupware Server Configuration Wizard",
                        "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KGlobal::locale()->insertCatalogue( "libkdepim" );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  bool verbose = false;
  if ( args->isSet( "verbose" ) ) verbose = true;

  EGroupwareWizard wizard;

  wizard.exec();
}

/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

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

#include <kdebug.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kuniqueapplication.h>
#include <stdlib.h>

#include <mainwindow.h>

#include "splash.h"

static KCmdLineOptions options[] =
{
  { 0, 0, 0}
};

int main( int argc, char *argv[] )
{
  KAboutData aboutData("kitchensync",I18N_NOOP("KitchenSync"),
		       "0.0.6-1",
		       I18N_NOOP("Synchronize Data with KDE"),
		       KAboutData::License_GPL,
		       "(c) 2001-2002 Holger Freyther\n"
		       "(c) 2002 Maximilian Reiss\n"
		       "(c) 2003 Cornelius Schumacher",
		       0,
		       "http://opie.handhelds.org" );
  aboutData.addAuthor("Maximilian Reiss",I18N_NOOP("Current Maintainer"),
                      "harlekin@handhelds.org");
  aboutData.addAuthor("Holger Freyther", I18N_NOOP("Current Maintainer"),
		      "zecke@handhelds.org");
  aboutData.addAuthor("Cornelius Schumacher", "", "schumacher@kde.org" );
  aboutData.addCredit("Alexandra Chalupka",
		      I18N_NOOP("For her understanding that I'm an addict."), 0 );
  aboutData.addCredit("HP ( former Compaq )",
		      I18N_NOOP("For all the support HP is giving OpenSource projects"
		    		"at handhelds.org. Thanks a lot."), 0 );
  aboutData.addCredit("Bipolar and the rest of the Opie TEAM!",
                      I18N_NOOP("Testing, testing, testing"),
                      "opie@handhelds.org" );
  aboutData.addCredit("Philib Bundell",
                      I18N_NOOP("For being such a nice guy."),
                      "pb@gnu.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();

  KUniqueApplication::addCmdLineOptions();

  if( !KUniqueApplication::start() ) {
    kdDebug(5210) << "KitchenSync already runs." << endl;
    exit( 0 );
  };

  KUniqueApplication a;
  // time for a Widget
  KSync::Splash *splash = new KSync::Splash;
  KSync::KSyncMainWindow *mainwindow = new KSync::KSyncMainWindow;
  delete splash;
  mainwindow->show();
  kdDebug(5210) << "exec now " << endl;
  a.exec();
  delete mainwindow;
}

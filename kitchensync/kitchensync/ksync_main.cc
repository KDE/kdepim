/*
� � � �This file is part of the OPIE Project
� � � �Copyright (c)  2002 Holger Freyther <zecke@handhelds.org>
� �                   2002 Maximilian Rei� <harlekin@handhelds.org>
� � � � � �

� � � � � � � �=.            
� � � � � � �.=l.            
� � � � � �.>+-=
�_;:, � � .> � �:=|.         This program is free software; you can 
.> <`_, � > �. � <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.-- � :           the terms of the GNU General Public
.="- .-=="i, � � .._         License as published by the Free Software
�- . � .-<_> � � .<>         Foundation; either version 2 of the License,
� � �._= =} � � � :          or (at your option) any later version.
� � .%`+i> � � � _;_.        
� � .i_,=:_. � � �-<s.       This program is distributed in the hope that  
� � �+ �. �-:. � � � =       it will be useful,  but WITHOUT ANY WARRANTY;
� � : .. � �.:, � � . . .    without even the implied warranty of
� � =_ � � � �+ � � =;=|`    MERCHANTABILITY or FITNESS FOR A
� _.=:. � � � : � �:=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.= � � � = � � � ;      Library General Public License for more
++= � -. � � .` � � .:       details.
�: � � = �...= . :.=-        
�-. � .:....=;==+<;          You should have received a copy of the GNU
� -_. . . � )=. �=           Library General Public License along with
� � -- � � � �:-=`           this library; see the file COPYING.LIB. 
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/


#include <kstandarddirs.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kuniqueapplication.h>
#include <klocale.h>
#include <stdlib.h>

#include <ksync_mainwindow.h>
#include "ksync_splash.h"

//#include "overviewwidget.h"

static KCmdLineOptions options[] =
{
  { 0, 0, 0}
};

int main(int argc,  char* argv[] )
{
  
//  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KAboutData aboutData("kitchensync",I18N_NOOP("KitchenSync"),
		       "0.0 pre Alpha",
		       I18N_NOOP("Synchronize Data with KDE"),
		       KAboutData::License_GPL,
		       "(c) 2001-2002 Holger Freyther\n(c) 2002 Maximilian Reiss",
		       0,
		       "http://www.al-jazeera.com" );
  aboutData.addAuthor("Maximilian Reiss",I18N_NOOP("Current Maintainer"),
                      "harlekin@handhelds.org");
  aboutData.addAuthor("Holger Freyther", I18N_NOOP("Current Maintainer"),
		      "zecke@handhelds.org");
  aboutData.addCredit("Alexandra Chalupka",
		      I18N_NOOP("For her understanding that I'm an addict."), 0 );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();

  KUniqueApplication::addCmdLineOptions();

  if( !KUniqueApplication::start() ){
    kdDebug(5210) << "Could not start" << endl;
    exit(0 );
  };
  KUniqueApplication a;
  // time for a Widget
  KitchenSync::Splash *splash = new KitchenSync::Splash;
  KitchenSync::KSyncMainWindow *mainwindow = new KitchenSync::KSyncMainWindow;
  delete splash;
  mainwindow->show();
//  QWidget *wid = new KitchenSync::OverviewWidget();
//  wid->show();
  kdDebug(5210) << "exec now " << endl;
  a.exec();

}


#include <kstandarddirs.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kuniqueapplication.h>
#include <klocale.h>
#include <stdlib.h>

#include "ksync_mainwindow.h"

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
		       "http://kitchensync.sf.org" );
  aboutData.addAuthor("Maximilian Reiss",I18N_NOOP("Current Maintainer"),
                      "harlekin@handhelds.org");
  aboutData.addAuthor("Holger Freyther", I18N_NOOP("Current Maintainer"),
		      "zecke@handhelds.org");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();

  KUniqueApplication::addCmdLineOptions();

  if( !KUniqueApplication::start() ){
    kdDebug() << "Could not start" << endl;
    exit(0 );
  };
  KUniqueApplication a;
  // time for a Widget

  KitchenSync::KSyncMainWindow *mainwindow = new KitchenSync::KSyncMainWindow;
  mainwindow->show();
//  QWidget *wid = new KitchenSync::OverviewWidget();
//  wid->show();
  kdDebug() << "exec now " << endl;
  a.exec();

}

#include <kdebug.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kuniqueapplication.h>
#include <stdlib.h>

#include "mainwindow.h"

static KCmdLineOptions options[] =
{
  KCmdLineLastOption
};

int main( int argc, char **argv )
{
  KAboutData about( "multisynk", I18N_NOOP( "MultiSynK" ),
                    "0.1", I18N_NOOP( "The KDE Syncing Application" ),
                    KAboutData::License_GPL_V2,
                    I18N_NOOP( "(c) 2004, The KDE PIM Team" ) );
  about.addAuthor( "Tobias Koenig", I18N_NOOP( "Current maintainer" ), "tokoe@kde.org" );

  KCmdLineArgs::init( argc, argv, &about );
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();

  KUniqueApplication::addCmdLineOptions();

  if( !KUniqueApplication::start() ) {
    kdDebug() << "multisynk already runs." << endl;
    exit( 0 );
  };

  KUniqueApplication app;

  MainWindow *mainWindow = new MainWindow;
  mainWindow->show();

  app.exec();
}

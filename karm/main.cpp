#include <signal.h>
#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include "version.h"
#include "mainwindow.h"


namespace
{
  const char* description = I18N_NOOP("KDE Time tracker tool");

  void cleanup( int )
  {
    kdDebug(5970) << i18n("Just caught a software interrupt.") << endl;
    kapp->exit();
  }
}

int main( int argc, char *argv[] )
{
  KAboutData aboutData( "karm", I18N_NOOP("KArm"),
      KARM_VERSION, description, KAboutData::License_GPL,
      "(c) 1997-2000, Sirtaj Singh Kang, Espen Sand, Jesper Pedersen,\n"
      "Kalle Dalheimer, Klarälvdalens Datakonsult AB,\n"
      "Tomas Pospisek, Sourcepole");

  aboutData.addAuthor( "Tomas Pospisek", 0, "tpo_deb@sourcepole.ch" );
  aboutData.addAuthor( "Sirtaj Singh Kang", I18N_NOOP("Original Author"),
                       "taj@kde.org" );
  aboutData.addAuthor( "Espen Sand",        0, "espen@kde.org");
  aboutData.addAuthor( "Kalle Dalheimer",   0, "kalle@kde.org" );
  aboutData.addAuthor( "Jesper Pedersen",   0, "blackie@kde.org" );
  aboutData.addAuthor( "Gioele Barabucci",  0, "gioele@gioelebarabucci.com");
  aboutData.addAuthor( "Jan Schaumann",     0, "jschauma@netmeister.org");
  aboutData.addAuthor( "Scott Monachello",  0, "smonach@cox.net");
  aboutData.addAuthor( "Willi Richert",     0, "w.richert@gmx.net");
  aboutData.addAuthor( "Mark Bucciarelli", I18N_NOOP("Current Maintainer"),
      "mark@hubcapconsulting.com");
  aboutData.addAuthor( "David Faure",       0, "faure@kde.org");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KApplication myApp;

  MainWindow *mainWindow = new MainWindow;
  myApp.setMainWidget( mainWindow );

  if (kapp->isRestored() && KMainWindow::canBeRestored( 1 ))
    mainWindow->restore( 1, false );
  else
    mainWindow->show();

  signal( SIGQUIT, cleanup );
  signal( SIGINT, cleanup );
  int ret = myApp.exec();

  delete mainWindow;
  return ret;
}

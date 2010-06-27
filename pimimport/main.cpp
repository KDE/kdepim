#include <akonadi/control.h>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KGlobal>
#include <KDebug>

#include <QtCore/QTimer>

#include "akonadidumper.h"

int main( int argc, char *argv[] )
{
  KAboutData aboutData( "pimimport", 0,
                    ki18n( "KDE PIM import/export tool" ),
                    "0.1",
                    ki18n( "Import/export of PIM data" ),
                    KAboutData::License_GPL );
  aboutData.setProgramIconName( "akonadi" );

  KCmdLineArgs::init(argc, argv, &aboutData);

  KApplication app;
  app.setQuitOnLastWindowClosed( false );

  KGlobal::setAllowQuit( true );

  if ( !Akonadi::Control::start( 0 ) )
    return 2;

  AkonadiDump dumper( "pimimport_test" );
  QTimer::singleShot(100, &dumper, SLOT(dump()));
  QObject::connect( &dumper, SIGNAL( finished() ), &app, SLOT( quit() ) );

  return app.exec();
}

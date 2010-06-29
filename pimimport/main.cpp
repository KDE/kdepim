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
  KCmdLineOptions options;
  options.add( "dump", ki18n( "Dump akonadi resources" ) );
  options.add( "restore", ki18n( "Restore akonadi resources " ) );
  options.add( "+dir", ki18n( "Dump directory" ) );
  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KApplication app;
  app.setQuitOnLastWindowClosed( false );

  KGlobal::setAllowQuit( true );

  if ( args->count() < 1 ) {
    KCmdLineArgs::usageError( i18n( "No dump directory specified" ) );
    return 1;
  }
  AkonadiDump dumper( args->arg( 0 ) );

  if ( args->isSet( "dump" ) ) {
    QTimer::singleShot( 100, &dumper, SLOT( dump() ) );
  }
  else if ( args->isSet( "restore" ) ) {
    QTimer::singleShot( 100, &dumper, SLOT( restore() ) );
  }
  else {
    KCmdLineArgs::usageError( i18n( "You have to specify either --dump or --restore") );
    return 1;
  }

  QObject::connect( &dumper, SIGNAL( finished() ), &app, SLOT( quit() ) );

  if ( !Akonadi::Control::start( 0 ) )
    return 2;

  return app.exec();
}

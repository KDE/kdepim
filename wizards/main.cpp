#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include "groupwarewizard.h"

static const KCmdLineOptions options[] =
{
  { "serverType <type>", "The server type", 0 },
  KCmdLineLastOption
};

int main( int argc, char **argv )
{
  KAboutData aboutData( "groupwarewizard",
                        "KDE-PIM Groupware Configuration Wizard", "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QString serverType;
  if ( args->isSet( "serverType" ) )
    serverType = args->getOption( "serverType" );

  GroupwareWizard wizard( 0 );
  app.setMainWidget( &wizard );

  wizard.setServerType( serverType );

  wizard.show();

  app.exec();
}

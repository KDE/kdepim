
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <KApplication>

#include "mobile_mainview.h"

int main( int argc, char **argv )
{
  const QByteArray& ba = QByteArray( "coisceim-mobile" );
  const KLocalizedString name = ki18n( "Kontact Touch Trips" );

  KAboutData aboutData( ba, ba, name, ba, name );
  aboutData.setProductName( "Coisceim Mobile" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KApplication app;

  MobileMainview view;
  view.show();

  return app.exec();
}


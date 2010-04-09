#include "kdeclarativemainview.h"

#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>

KDeclarativeMainView::KDeclarativeMainView( const QString &appName, QWidget *parent )
  : QDeclarativeView( parent )
{
  init();

  foreach ( const QString &importPath, KGlobal::dirs()->findDirs( "module", "imports" ) )
    engine()->addImportPath( importPath );

  const QString qmlPath = KStandardDirs::locate( "data", "mobile/" + appName + ".qml" );
  setSource( qmlPath );
}



//#include <qvbox.h>
//#include <qwidgetstack.h>
//#include <qsize.h>

//#include <kaction.h>
//#include <klocale.h>
//#include <kmenubar.h>
//#include <kdebug.h>
//#include <ktrader.h>
//#include <kstatusbar.h>

//#include <kparts/componentfactory.h>
//#include <kpopupmenu.h>

#include <qwidget.h>


#include "ksync_configuredialog.h"



#include "ksync_mainwindow.h"

#include "partbar.h"

using namespace KitchenSync;

KSyncMainWindow::KSyncMainWindow(QWidget *widget,
                        const char *name,
                        WFlags f )
    : KParts::MainWindow( widget,  name,  f )
{
   setInstance( KGlobal::instance() );
}
KSyncMainWindow::~KSyncMainWindow()
{

}

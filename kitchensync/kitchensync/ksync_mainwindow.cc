
#include <qvbox.h>
#include <qsize.h>

#include <kaction.h>
#include <klocale.h>
#include <kmenubar.h>

#include "partbar.h"
#include "ksync_mainwindow.h"


using namespace KitchenSync;


KSyncMainWindow::KSyncMainWindow(QWidget *widget, const char *name, WFlags f)
  :
  KParts::MainWindow( widget, name, f ){
  setInstance( KGlobal::instance() );
  
  initActions();
  setXMLFile("ksyncgui.rc");
  createGUI( 0l );
  // now add a layout or QWidget?
  m_lay = new QHBox(this,   "main widget" );
  setCentralWidget( m_lay );
  m_bar = new PartBar(m_lay , "partBar" );
  QWidget *wid = new QWidget( m_lay, "dummy" );
  wid->setBackgroundColor(Qt::darkRed );
  m_bar->setMaximumWidth(100 );
  m_bar->setMinimumWidth(100 );
  
};

KSyncMainWindow::~KSyncMainWindow()
{

}
void KSyncMainWindow::initActions()
{
  (void)new KAction( i18n("Synchronize" ), 0, this, SLOT( slotSync() ),
		     actionCollection(), "sync" );
}
//#include "ksync_mainwindow.moc"

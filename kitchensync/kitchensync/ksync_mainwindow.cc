
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

  m_parts.setAutoDelete( true );
  initPlugins();  
};

KSyncMainWindow::~KSyncMainWindow()
{

}
void KSyncMainWindow::initActions()
{
  (void)new KAction( i18n("Synchronize" ), 0, this, SLOT( slotSync() ),
		     actionCollection(), "sync" );
  (void)new KAction( i18n("Backup"), 0, this, SLOT( slotBackup() ),
		     actionCollection(), "backup" );
  (void)new KAction( i18n("Restore"), 0, this, SLOT (slotRestore() ),
		     actionCollection(), "restore" );
  (void)new KAction( i18n("Configure Kitchensync") , 0, this, SLOT (slotConfigure() ),
		     actionCollection(), "configure" );
}
void KSyncMainWindow::initPlugins()
{
  /* KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KitchenSync/Manipulator"),
						     QString::null);

  for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
    {
      ManipulatorPart *plugin = KParts::ComponentFactory
	::createInstanceFromService<ManipulatorPart>(*it, this);
      if (!plugin)
	continue;
      addModPart( plugin );
    } 
  */
}
void KSyncMainWindow::addModPart()
{

}
void KSyncMainWindow::slotSync() {
}

void KSyncMainWindow::slotBackup() {
}

void KSyncMainWindow::slotRestore() {
}

void KSyncMainWindow::slotConfigure() {
}


//#include "ksync_mainwindow.moc"

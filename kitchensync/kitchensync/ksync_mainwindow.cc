
#include <qvbox.h>
#include <qwidgetstack.h>
#include <qsize.h>

#include <kaction.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kdebug.h>

#include "partbar.h"
#include "ksync_mainwindow.h"

#include <ksync_configuredialog.h>
#include "organizer/ksync_organizerpart.h"
#include "overview/ksync_overviewpart.h"



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
  m_stack = new QWidgetStack( m_lay, "dummy" );
  QWidget *test = new QWidget(m_stack);
  test->setBackgroundColor(Qt::red);
  m_stack->addWidget(test, 0);
  m_stack->raiseWidget(0);
  m_bar->setMaximumWidth(100 );
  m_bar->setMinimumWidth(100 );
  connect( m_bar, SIGNAL(activated(ManipulatorPart*) ), this, SLOT(slotActivated(ManipulatorPart*) ));

  resize(600,400);
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
  (void)new KAction( i18n("Quit"), 0, this, SLOT(slotQuit()),
		     actionCollection(), "quit" );
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
  OrganizerPart *org = new OrganizerPart(this, "wallah" );
  
  OverviewPart *view = new OverviewPart(this, "hallaw" );
  addModPart(view);
  addModPart( org);
  
}

void KSyncMainWindow::addModPart(ManipulatorPart *part)
{
  static int id=1;
  //m_parts.clear();
  // diable it for testing
  //if( part->partIsVisible )
  {
    kdDebug() << "before part insert \n"  ;
    m_bar->insertItem( part );
    
  }
  m_parts.append( part );
  m_stack->addWidget( part->widget(), id );
  id++;
}

void KSyncMainWindow::slotSync() {
}

void KSyncMainWindow::slotBackup() {
}

void KSyncMainWindow::slotRestore() {
}

void KSyncMainWindow::slotConfigure() {
  ConfigureDialog dlg(this);
  ManipulatorPart *part;
  for (part = m_parts.first(); part != 0; part = m_parts.next() ) {
    dlg.addWidget(part->configWidget(), part->name(), part->pixmap() );
  }
  
  if (dlg.exec()) {
     for (part = m_parts.first(); part != 0; part = m_parts.next() ) {
       part->slotConfigOk();
     }
  }
}

void KSyncMainWindow::slotActivated(ManipulatorPart *part) {
  m_stack->raiseWidget(part->widget() );

}

void KSyncMainWindow::slotQuit() {
  close();
}


//#include "ksync_mainwindow.moc"

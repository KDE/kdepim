/*
† † † † † † † †=.            This file is part of the OPIE Project
† † † † † † †.=l.            Copyright (c)  2002 Holger Freyther <zecke@handhelds.org>
† † † † † †.>+-=                            2002 Maximilian Reiﬂ <harlekin@handhelds.org>
†_;:, † † .> † †:=|.         This library is free software; you can
.> <`_, † > †. † <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.-- † :           the terms of the GNU Library General Public
.="- .-=="i, † † .._         License as published by the Free Software
†- . † .-<_> † † .<>         Foundation; either version 2 of the License,
† † †._= =} † † † :          or (at your option) any later version.
† † .%`+i> † † † _;_.
† † .i_,=:_. † † †-<s.       This library is distributed in the hope that
† † †+ †. †-:. † † † =       it will be useful,  but WITHOUT ANY WARRANTY;
† † : .. † †.:, † † . . .    without even the implied warranty of
† † =_ † † † †+ † † =;=|`    MERCHANTABILITY or FITNESS FOR A
† _.=:. † † † : † †:=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.= † † † = † † † ;      Library General Public License for more
++= † -. † † .` † † .:       details.
†: † † = †...= . :.=-
†-. † .:....=;==+<;          You should have received a copy of the GNU
† -_. . . † )=. †=           Library General Public License along with
† † -- † † † †:-=`           this library; see the file COPYING.LIB.
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/


#include <qapplication.h>
#include <qvbox.h>
#include <qptrlist.h>
#include <qwidgetstack.h>
#include <qsize.h>

#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmenubar.h>
#include <ktrader.h>
#include <kstatusbar.h>

#include <kparts/componentfactory.h>
#include <kpopupmenu.h>

#include <kapabilities.h>
#include <kdevice.h>
#include <konnector.h>

#include "ksync_configpart.h"
#include "ksync_configuredialog.h"
#include "manipulatorpart.h"
#include "partbar.h"

#include "konnectordialog.h"
#include "ksync_mainwindow.h"


using namespace KSync;

KSyncMainWindow::KSyncMainWindow(QWidget *widget, const char *name, WFlags f)
  :
  KParts::MainWindow( widget, name, f ){

  m_konnector = 0;
  initActions();
  setXMLFile("ksyncgui.rc");
  setInstance( KGlobal::instance() );

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

  kdDebug(5210) << "Init konnector" << endl;
  initKonnector();
  initPlugins();

  //statusBar()->insertItem(i18n("Not Connected"), 10, 0, true );
  statusBar()->message(i18n("Not connected") );
  statusBar()->show();

  // show systemtraypart
  initSystray();
  m_tray->show();

};

KSyncMainWindow::~KSyncMainWindow()
{
    createGUI(0l );
}
void KSyncMainWindow::initActions()
{
  (void)new KAction( i18n("Synchronize" ), "reload", 0, this, SLOT( slotSync() ),
		     actionCollection(), "sync" );
  (void)new KAction( i18n("Backup") ,  "mail_get", 0, this, SLOT( slotBackup() ),
		     actionCollection(), "backup" );
  (void)new KAction( i18n("Restore"),  "mail_send", 0, this, SLOT (slotRestore() ),
		     actionCollection(), "restore" );
  (void)new KAction( i18n("Quit"),  "exit", 0, this, SLOT(slotQuit()),
		     actionCollection(), "quit" );
  (void)new KAction( i18n("Configure Kitchensync"), "configure" , 0, this,
		     SLOT (slotConfigure() ), actionCollection(), "configure" );
}
void KSyncMainWindow::initPlugins()
{
  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KitchenSync/Manipulator"),
						     QString::null);

  for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it){
      ManipulatorPart *plugin = KParts::ComponentFactory
	::createInstanceFromService<ManipulatorPart>(*it, this);
      if (!plugin)
	continue;
      addModPart( plugin );
  }

}

void KSyncMainWindow::addModPart(ManipulatorPart *part)
{
  static int id=1;
  //m_parts.clear();
  // diable it for testing
  if( part->partIsVisible() )
  {
    int pos = -1;
    kdDebug(5210) << "before part insert \n"  ;
    m_stack->addWidget( part->widget(), id );
    if( part->type() == QString::fromLatin1("Overview") ){ // Overview is special for us ;)
      m_stack->raiseWidget(id );
      pos = 0;
    }

    m_bar->insertItem( part, pos );
  }
  m_parts.append( part );

  id++;
}

void KSyncMainWindow::initSystray( void ) {

    m_tray = new KSyncSystemTray( this, "KSyncSystemTray");
    KPopupMenu *popMenu = m_tray->getContextMenu();
    popMenu->insertSeparator();

}

void KSyncMainWindow::slotSync()
{
    /*
    kdDebug(5210) << "slotSync " << endl;
    if (m_currentId.isEmpty() ) {
        kdDebug(5210) << "Current Id empty" << endl;
        //KMessageBox::
        return; // KMessageBox
    }
    if (!m_profile.caps().supportsPushSync() ) {
        kdDebug(5210) << "Can not push" << endl;
        return; // pop up
    }
    if (!m_profile.isConfigured() ) {
        kdDebug(5210) << "Not configured" << endl;
        return; // pop up
    }
    kdDebug(5210) << "Ok starting sync" << endl;
    m_konnector->startSync( m_currentId );
    */
}
void KSyncMainWindow::slotBackup() {
}

void KSyncMainWindow::slotRestore() {
}

void KSyncMainWindow::slotConfigure() {
    KonnectorProfile::ValueList lst;
    KonnectorDialog dlg(lst,  m_konnector);
    dlg.exec();
    /*
  ConfigureDialog *dlg = new ConfigureDialog(this);
  ManipulatorPart *part = 0l;
  // get the kapabilities and update with the ones from the Profile
  Kapabilities cap = m_konnector->capabilities( m_currentId );
  Kapabilities cap2 = m_profile.caps();
  cap.setSrcIP( cap2.srcIP() );
  cap.setDestIP( cap2.destIP() );
  cap.setUser( cap2.user() );
  cap.setPassword( cap2.password() );
  cap.setCurrentPort( cap2.currentPort() );
  cap.setCurrentModel( cap2.currentModel() );
  cap.setCurrentConnectionMode( cap2.currentConnectionMode() );
  cap.setMetaSyncingEnabled( cap2.isMetaSyncingEnabled() );

  ConfigPart *par = new ConfigPart(cap , dlg );
  dlg->addWidget( par, i18n("Configure"), &QPixmap() );
  for (part = m_parts.first(); part != 0; part = m_parts.next() ) {
    if( part->configIsVisible() )
      dlg->addWidget(part->configWidget(), part->name(), part->pixmap() );
  }
  if (dlg->exec()) {
      m_profile.setCapability( par->capability() );
      m_profile.setConfigured( true );
      m_konnector->setCapabilities( m_currentId,  m_profile.caps() );
      saveCurrentProfile();
     for (part = m_parts.first(); part != 0; part = m_parts.next() ) {
       part->slotConfigOk();
     }
  }
    */
}

void KSyncMainWindow::slotActivated(ManipulatorPart *part) {
  m_stack->raiseWidget(part->widget() );

  createGUI( part );
}

void KSyncMainWindow::slotQuit() {
    close();
}
KSyncSystemTray* KSyncMainWindow::tray()
{
    return m_tray;
}
KonnectorManager* KSyncMainWindow::konnector()
{
    return m_konnector;
}
QString KSyncMainWindow::currentId() const
{
    return m_currentId;
}
QMap<QString,QString> KSyncMainWindow::ids() const
{
    return m_ids;
}
void KSyncMainWindow::initKonnector()
{
    kdDebug(5210) << "init konnector" << endl;
    m_konnector = new KonnectorManager(this,  "Konnector");

    connect(m_konnector,SIGNAL(wantsToSync(const QString&, Syncee::PtrList ) ),
            this, SLOT(slotSync( const QString&,  Syncee::PtrList) ) );

    connect(m_konnector, SIGNAL(stateChanged(const QString&,  bool) ),
            this,  SLOT(slotStateChanged(const QString&,  bool) ) );

    connect(m_konnector, SIGNAL(konnectorError(const QString&,  int,  const QString& ) ),
            this,  SLOT(slotKonnectorError( const QString&,  int, const QString&) ) );

    // ok now just load the Opie Konnector // FIXME Don't hard code
    Device::ValueList device;
    device = m_konnector->query();
    for(Device::ValueList::Iterator it = device.begin(); it != device.end(); ++it ){
        kdDebug(5210) << "Identify "  << (*it).identify() << endl;
        kdDebug(5210) << "Group " << (*it).group() << endl;
        kdDebug(5210) << "Vendor " << (*it).vendor() << endl;
        kdDebug(5210) << "UNIX " << (*it).id() << endl;
        if ( (*it).id() == QString::fromLatin1("Opie-1") ) {
        }
    }
}
// do we need to change the Konnector first?
// raise overview and then pipe informations
// when switching to KSyncee/KSyncEntry we will make
// it asynchronus
void KSyncMainWindow::slotSync( const QString &udi,
                                Syncee::PtrList lis)
{
/*    KSyncEntry::List ret;
    kdDebug(5210) << "Some data arrived Yeah baby" << endl;
    kdDebug(5210) << "Lis got "  << lis.count() << "elements" << endl;
    KSyncEntry *entry=0;
    for ( entry = lis.first(); entry != 0; entry = lis.next() ) {
        kdDebug() << "Type is " << entry->type() << endl;
    }
    // pass them through all widgets
    ManipulatorPart* part=0l;
    ManipulatorPart* po=0l;
    for ( part = m_parts.first(); part != 0; part = m_parts.next() ) {
        part->startSync();
    }
    qApp->processEvents();
    for ( part = m_parts.first(); part != 0; part = m_parts.next() ) {
    // part is the activated part
        // rather inefficent can QSignal be more direct? Request first?
        // but this is rather brain dead
        QPtrListIterator<ManipulatorPart> it(m_parts);
        for ( ; it.current(); ++it ) {
            it.current()->slotSyncPartActivated( part );
            it.current()->slotProgress( part, SYNC_START,  0 );
        }
        qApp->processEvents(); // HACK make it asynchronus
        part->processEntry( lis,  ret );

        kdDebug(5210 ) << "processed " << part->name() << endl;
        it.toFirst();
        for ( ; it.current(); ++it ) {
            it.current()->slotProgress( part, SYNC_DONE,  0 );
        }
        qApp->processEvents();
    }
    lis.setAutoDelete( TRUE );
    lis.clear(); //there is a bug now we will leak but not crash :(
    m_konnector->write( udi, ret );
*/
}
void KSyncMainWindow::slotStateChanged( const QString &udi,
                                        bool connected )
{
    kdDebug(5210) << "State changed" << connected << endl;
    if ( !connected )
        statusBar()->message(i18n("Not connected") );
    else
        statusBar()->message(i18n("Connected") );
    statusBar()->show();
}
void KSyncMainWindow::slotKonnectorError( const QString& udi,
                                          int error,
                                          const QString& id )
{

}
// Check if we already configured a device with  the id
// It's fairly easy cause we only allow one at a time
void KSyncMainWindow::setupKonnector( const Device& udi,  const QString &id )
{
}
void KSyncMainWindow::saveCurrentProfile()
{
/*    KConfig *cfg = kapp->config();
    Kapabilities cap = m_profile.caps();
    cfg->setGroup("Opie-Konnector");
    cfg->writeEntry( "name",  "Opie StandardProfile");
    cfg->writeEntry( "srcIP",  cap.srcIP() );
    cfg->writeEntry( "destIP",  cap.destIP() );
    cfg->writeEntry( "user",  cap.user() );
    cfg->writeEntry( "pass",  cap.password() );
    cfg->writeEntry( "port",  cap.currentPort() );
    cfg->writeEntry( "model",  cap.currentModel() );
    cfg->writeEntry( "mode",  cap.currentConnectionMode() );
    cfg->writeEntry( "meta",  cap.isMetaSyncingEnabled() );
    cfg->writeEntry( "push",  cap.supportsPushSync() );
*/
}
#include "ksync_mainwindow.moc"

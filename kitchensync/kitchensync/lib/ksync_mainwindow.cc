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

#include <syncer.h>
#include <syncuikde.h>

#include <kapabilities.h>
#include <kdevice.h>
#include <konnector.h>

#include "ksync_configpart.h"
#include "ksync_configuredialog.h"
#include "manipulatorpart.h"
#include "partbar.h"
#include "profiledialog.h"


#include "konnectordialog.h"
#include "ksync_mainwindow.h"
#include "syncalgo.h"

using namespace KSync;

KSyncMainWindow::KSyncMainWindow(QWidget *widget, const char *name, WFlags f)
  :
  KParts::MainWindow( widget, name, f ){

  m_konnector = 0;
  m_syncAlg = 0l;
  m_syncUi = 0l;
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

  connect( m_bar, SIGNAL(activated(ManipulatorPart*) ),
           this, SLOT(slotActivated(ManipulatorPart*) ));

  resize(600,400);

  m_parts.setAutoDelete( true );

//  kdDebug(5210) << "Init konnector" << endl;
  initKonnector();
  initPlugins();

  //statusBar()->insertItem(i18n("Not Connected"), 10, 0, true );
  statusBar()->message(i18n("Not connected") );
  statusBar()->show();

  // show systemtraypart
  initSystray();
  m_tray->show();
  initProfiles();
};

KSyncMainWindow::~KSyncMainWindow()
{
    kdDebug() << "d'tor" << endl;
    m_konprof->save();
    m_prof->save();
    createGUI(0l );
}
void KSyncMainWindow::initActions()
{
  (void)new KAction( i18n("Synchronize" ), "reload", 0,
                     this, SLOT( slotSync() ),
		     actionCollection(), "sync" );

  (void)new KAction( i18n("Backup") ,  "mail_get", 0,
                     this, SLOT( slotBackup() ),
		     actionCollection(), "backup" );

  (void)new KAction( i18n("Restore"),  "mail_send", 0,
                     this, SLOT (slotRestore() ),
		     actionCollection(), "restore" );

  (void)new KAction( i18n("Quit"),  "exit", 0,
                     this, SLOT(slotQuit()),
		     actionCollection(), "quit" );

  (void)new KAction( i18n("Configure Connections"), "configure" , 0,
                     this, SLOT (slotConfigure() ),
                     actionCollection(), "configure" );

  (void)new KAction( i18n("Configure Profiles"),  "configure", 0,
                     this, SLOT(slotConfigProf() ),
                     actionCollection(), "config_profile" );

  (void)new KAction( i18n("Configure current Profile"),  "configure", 0,
                     this, SLOT(slotConfigCur() ),
                     actionCollection(), "config_current" );
  m_konAct = new KSelectAction( i18n("Konnector"),
                                KShortcut(),this,
                                SLOT(slotKonnectorProfile() ),
                                actionCollection(),
                                "select_kon");

  m_profAct = new KSelectAction( i18n("Profile"),  KShortcut(), this,
                                 SLOT(slotProfile() ),
                                 actionCollection(), "select_prof");
}
/*
 * we search for all installed plugins here
 * and add them to the ManPartService List
 * overview is special for us
 */
void KSyncMainWindow::initPlugins()
{
  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KitchenSync/Manipulator"),
						     QString::null);

  for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it){
      ManPartService ser( (*it) );
      m_partsLst.append( ser );
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
//    kdDebug(5210) << "before part insert \n"  ;
    m_stack->addWidget( part->widget(), id );
    /* overview is special for us ;) */
    if( part->type() == QString::fromLatin1("Overview") ){
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

void KSyncMainWindow::slotSync(){
}
void KSyncMainWindow::slotBackup() {
}
void KSyncMainWindow::slotRestore() {
}
void KSyncMainWindow::slotConfigure() {
    KonnectorDialog dlg(m_konprof->list() ,  m_konnector);
    /* clicked ok - now clean up*/
    if ( dlg.exec() ) {
        m_konprof->clear();
        KonnectorProfile::ValueList all = dlg.devices();
        removeDeleted(dlg.removed() );
        loadUnloaded( dlg.toLoad(), all );
        unloadLoaded( dlg.toUnload(), all );

        /* the rest and unchanged items */
        for ( KonnectorProfile::ValueList::Iterator it = all.begin();
              it != all.end(); ++it ) {
            m_konprof->add( (*it) );
        }
        m_konprof->save();
        initKonnectorList();
        slotKonnectorProfile();
    }
}
void KSyncMainWindow::removeDeleted( const KonnectorProfile::ValueList& list ) {
    KonnectorProfile::ValueList::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
        m_konnector->unregisterKonnector( (*it).udi() );
    }
}
/*
 * loadUnloaded items need to load items and remove them from
 * all list so we have appropriate udi and no duplicates
 */
void KSyncMainWindow::loadUnloaded( const KonnectorProfile::ValueList& list,
                                    KonnectorProfile::ValueList& all ) {
    KonnectorProfile::ValueList::ConstIterator itList;
    KonnectorProfile::ValueList::Iterator itAll;
    for ( itList = list.begin(); itList != list.end(); ++itList ) {
        itAll = all.find( (*itList) );
        /* found it */
        if ( itAll != all.end() )  {
            QString udi =m_konnector->registerKonnector( (*itList).device() );
            if (!udi.isEmpty() ) {
                KonnectorProfile prof = (*itList);
                prof.setUdi( udi );
                m_konnector->setCapabilities( udi, prof.kapabilities() );
                m_konprof->add( prof );
            }
            all.remove( itAll );
        }
    }
}
/*
 * unloadLoaded unloads loaded KonnectorPlugins
 * through the KonnectorManager
 */
void KSyncMainWindow::unloadLoaded( const KonnectorProfile::ValueList& list,
                                    KonnectorProfile::ValueList& all ) {
    KonnectorProfile::ValueList::ConstIterator itList;
    KonnectorProfile::ValueList::Iterator itAll;
    for ( itList = list.begin(); itList != list.end(); ++itList ) {
        itAll = all.find( (*itList) );
        /* found it */
        if ( itAll != all.end() )  {
            m_konnector->unregisterKonnector( (*itList).udi() );
            KonnectorProfile prof = (*itList);
            prof.setUdi( QString::null );
            m_konprof->add( prof );
            all.remove( itAll );
        }
    }
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
//    kdDebug(5210) << "init konnector" << endl;
    m_konnector = new KonnectorManager(this,  "Konnector");

    connect(m_konnector,SIGNAL(wantsToSync(const QString&, Syncee::PtrList ) ),
            this, SLOT(slotSync( const QString&,  Syncee::PtrList) ) );

    connect(m_konnector, SIGNAL(stateChanged(const QString&,  bool) ),
            this,  SLOT(slotStateChanged(const QString&,  bool) ) );

    connect(m_konnector, SIGNAL(konnectorError(const QString&,  int,  const QString& ) ),
            this,  SLOT(slotKonnectorError( const QString&,  int, const QString&) ) );

}
/*
 * we're now initializing the ordinary profiles
 */
void KSyncMainWindow::initProfiles() {
    kdDebug(5210) << "Init profiles " << endl;
    m_konprof = new KonnectorProfileManager();
    kdDebug(5210) << "About to Load" << endl;
    m_konprof->load();
    /* now load the wasLoaded() items */
    KonnectorProfile::ValueList list = m_konprof->list();
    KonnectorProfile::ValueList newL;
    KonnectorProfile::ValueList::Iterator konIt;
    for (konIt= list.begin(); konIt != list.end(); ++konIt ) {
        KonnectorProfile prof = (*konIt);
        if ( (*konIt).wasLoaded() ) {
            prof.setUdi( m_konnector->registerKonnector( prof.device() ) );
            m_konnector->setCapabilities( prof.udi(), prof.kapabilities() );
        }
        newL.append( prof);
    }
    m_konprof->setList( newL );
    initKonnectorList();
    slotKonnectorProfile();
    /* end the hack */

    m_prof = new ProfileManager();
    m_prof->load();
    initProfileList();
    slotProfile();
}
Profile KSyncMainWindow::currentProfile()const {
    return m_prof->currentProfile();
}
ProfileManager* KSyncMainWindow::profileManager()const {
    return m_prof;
}
KonnectorProfile KSyncMainWindow::konnectorProfile() const {
    return m_konprof->current();
}
KonnectorProfileManager* KSyncMainWindow::konnectorManager() const {
    return m_konprof;
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
/**
 * check if the state is from the current Konnector
 * if yes update the state
 */
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
/*
 * Show a KmessageBox
 */
void KSyncMainWindow::slotKonnectorError( const QString& udi,
                                          int error,
                                          const QString& id )
{

}
/*
 * configure profiles
 */
void KSyncMainWindow::slotConfigProf() {
    ProfileDialog dlg(m_prof->profiles(), m_partsLst ) ;
    if ( dlg.exec() ) {
        m_prof->setProfiles( dlg.profiles() );
        m_prof->save();
        // switch profile
        initProfileList();
        slotProfile();
    }
}
void KSyncMainWindow::switchProfile( const Profile& prof ) {
    kdDebug() << "switch profile" << endl;
    m_bar->clear();
    m_parts.setAutoDelete( true );
    m_parts.clear();
    ManPartService::ValueList lst = prof.manParts();
    ManPartService::ValueList::Iterator it;
    for (it = lst.begin(); it != lst.end(); ++it ) {
        addPart( (*it) );
    }
    Profile oldprof = m_prof->currentProfile();
    m_prof->setCurrentProfile( oldprof );
    emit profileChanged( prof );

}
void KSyncMainWindow::addPart( const ManPartService& service ) {
    ManipulatorPart *part = KParts::ComponentFactory
                            ::createInstanceFromLibrary<ManipulatorPart>( service.libname().local8Bit(),
                                                         this );
    if (part )
        addModPart( part );
}
void KSyncMainWindow::switchProfile( KonnectorProfile& prof ) {
    KonnectorProfile ole = m_konprof->current();

    if (prof.udi().isEmpty() ) {
        QString udi =m_konnector->registerKonnector( prof.device() );
        prof.setUdi( udi );
        m_konnector->setCapabilities( udi, prof.kapabilities() );
    }

    emit konnectorChanged( ole.udi() );
    emit konnectorChanged( ole );

    m_konprof->setCurrent( prof );
}
/*
 * configure current loaded
 */
void KSyncMainWindow::slotConfigCur() {
    ConfigureDialog *dlg = new ConfigureDialog(this);
    ManipulatorPart *part = 0l;

    for (part = m_parts.first(); part != 0; part = m_parts.next() ) {
        if( part->configIsVisible() )
            dlg->addWidget(part->configWidget(),
                           part->name(),
                           part->pixmap() );
    }
    if (dlg->exec()) {
        for (part = m_parts.first(); part != 0; part = m_parts.next() ) {
            part->slotConfigOk();
        }
    }
    delete dlg;
    m_prof->save();
}

void KSyncMainWindow::slotKonnectorProfile() {
    int item = m_konAct->currentItem();
    if ( item == -1 ) item = 0;
    if (m_konprof->count() == 0 ) return;

    KonnectorProfile cur = m_konprof->profile( item );
    switchProfile( cur );
    m_konprof->setCurrent( cur );
}
/*
 * the Profile was changed in the Profile KSelectAction
 */
void KSyncMainWindow::slotProfile() {
    int item = m_profAct->currentItem();
    if (item == -1) item = 0; // for initialisation
    if ( m_prof->count() == 0 )  return;

    kdDebug() << "Changing profile " << item << endl;
    Profile cur = m_prof->profile( item );

    switchProfile( cur );
    m_prof->setCurrentProfile( cur );
}
void KSyncMainWindow::initProfileList() {
    Profile::ValueList list = m_prof->profiles();
    Profile::ValueList::Iterator it;
    QStringList lst;
    for (it = list.begin(); it != list.end(); ++it ) {
        lst << (*it).name();
    }
    m_profAct->setItems( lst);
}
void KSyncMainWindow::initKonnectorList() {
    KonnectorProfile::ValueList list = m_konprof->list();
    KonnectorProfile::ValueList::Iterator it;
    QStringList lst;

    for ( it = list.begin() ; it != list.end(); ++it ) {
        lst << (*it).name();
    }
    m_konAct->setItems( lst );
}
SyncUi* KSyncMainWindow::syncUi() {
    if (!m_syncUi ) {
        m_syncUi = new SyncUiKde(this);
    }
    return m_syncUi;
}
SyncAlgorithm* KSyncMainWindow::syncAlgorithm() {
    if (!m_syncAlg ) {
        m_syncAlg = new PIMSyncAlg( syncUi() );
    }
    return m_syncAlg;
}

#include "ksync_mainwindow.moc"

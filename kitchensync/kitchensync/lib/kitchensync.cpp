/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002,2003 Maximilian Reiﬂ <harlekin@handhelds.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qwidgetstack.h>
#include <qlayout.h>

#include <klocale.h>
#include <kstatusbar.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kcmultidialog.h>

#include <kparts/componentfactory.h>
#include <kpopupmenu.h>

#include <syncer.h>
#include <syncuikde.h>

#include <konnectormanager.h>
#include <konnectorplugin.h>
#include <error.h>
#include <progress.h>

#include "syncconfig.h"
#include "configuredialog.h"
#include "partbar.h"
#include "profiledialog.h"

#include "konnectorbar.h"
#include "konnectordialog.h"
#include "syncalgo.h"
#include "mainwindow.h"
#include "actionmanager.h"

#include "kitchensync.h"

using namespace KSync;

namespace {

struct MainProgress
{
    static Error noKonnector();
    static Error noPush();
};

Error MainProgress::noKonnector()
{
    return Error(i18n("There is no current Konnector") );
}

Error MainProgress::noPush()
{
    return Error(i18n("The current Konnector does not support pushing") );
}

kdbgstream operator<<( kdbgstream str, const Notify& no )
{
    str << no.code() << " " << no.text();
    return str;
}

}

KitchenSync::KitchenSync( ActionManager *actionManager, QWidget *parent )
  : Core( parent ), mActionManager( actionManager )
{
  m_syncAlg = 0;
  m_syncUi = 0;

  m_partsIt = 0;

  m_isSyncing = false;

#if 0
  initActions();
  setXMLFile("ksyncgui.rc");
  setInstance( KGlobal::instance() );

  createGUI( 0 );
#endif

  QBoxLayout *topLayout = new QHBoxLayout( this );

  m_bar = new PartBar( this, "partBar" );
  topLayout->addWidget( m_bar );
  m_stack = new QWidgetStack( this, "dummy" );
  topLayout->addWidget( m_stack );

  QWidget *test = new QWidget(m_stack);
  test->setBackgroundColor(Qt::red);
  m_stack->addWidget( test, 0 );
  m_stack->raiseWidget( 0 );
  m_bar->setMaximumWidth( 100 );
  m_bar->setMinimumWidth( 100 );

  connect( m_bar, SIGNAL( activated( ManipulatorPart * ) ),
           SLOT( slotActivated( ManipulatorPart * ) ) );

  resize( 600, 400 );

  m_parts.setAutoDelete( true );

  initPlugins();

  // show systemtraypart
  initSystray();
  m_tray->show();
}

KitchenSync::~KitchenSync()
{
  m_prof->save();

#if 0
  createGUI( 0 );
#endif
}

/*
 * we search for all installed plugins here
 * and add them to the ManPartService List
 * overview is special for us
 */
void KitchenSync::initPlugins()
{
  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KitchenSync/Manipulator"),
						     QString::null);

  for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it){
      ManPartService ser( (*it) );
      m_partsLst.append( ser );
  }

}

void KitchenSync::addModPart( ManipulatorPart *part )
{
    /* the usual signal and slots */
    connect( part, SIGNAL(sig_progress(ManipulatorPart*, int ) ),
             SLOT(slotPartProg(ManipulatorPart*, int ) ) );
    connect( part, SIGNAL(sig_progress(ManipulatorPart*, const Progress& ) ),
             SLOT(slotPartProg(ManipulatorPart*, const Progress& ) ) );
    connect( part, SIGNAL(sig_error(ManipulatorPart*, const Error& ) ),
             SLOT(slotPartErr(ManipulatorPart*, const Error& ) ) );
    connect( part, SIGNAL(sig_syncStatus(ManipulatorPart*, int) ),
             SLOT(slotPartSyncStatus( ManipulatorPart*, int ) ) );

    KonnectorManager *m = KonnectorManager::self();
    connect( m, SIGNAL( synceesRead( Konnector *, const SynceeList & ) ),
             part, SLOT( slotSynceesRead( Konnector *, const SynceeList & ) ) );
    connect( m, SIGNAL( synceeReadError( Konnector * ) ),
             part, SLOT( slotSynceeReadError( Konnector * ) ) );
    connect( m, SIGNAL( synceesWritten( Konnector * ) ),
             part, SLOT( slotSynceesWritten( Konnector * ) ) );
    connect( m, SIGNAL( synceeWriteError( Konnector * ) ),
             part, SLOT( slotSynceeWriteError( Konnector * ) ) );

    if ( part->partIsVisible() )  {
        kdDebug(5210) << "Part is Visible " << part->name() << endl;
        int pos = -1;
        m_stack->addWidget( part->widget() );

        /* overview is special for us ;) */
        if ( part->type() == i18n("Overview") ) {
            m_stack->raiseWidget( part->widget() );
            pos = 0;
        }
        m_bar->insertItem( part, pos );
    }
    m_parts.append( part );
}

void KitchenSync::initSystray( void )
{
    m_tray = new KSyncSystemTray( this, "KSyncSystemTray");
    KPopupMenu *popMenu = m_tray->getContextMenu();
    popMenu->insertSeparator();
}

void KitchenSync::slotSync()
{
  emit partProgress( 0, Progress( i18n( "Starting sync" ) ) );

  ManipulatorPart *part;
  for ( part = m_parts.first(); part; part = m_parts.next() ) {
    part->actionSync();
  }
}

void KitchenSync::slotActivated( ManipulatorPart *part )
{
    emit partChanged( part );
    m_stack->raiseWidget( part->widget() );
//    createGUI( part );
}

void KitchenSync::slotQuit()
{
    close();
}

KSyncSystemTray* KitchenSync::tray()
{
    return m_tray;
}

/*
 * we're now initializing the ordinary profiles
 */
void KitchenSync::initProfiles()
{
    kdDebug() << "KitchenSync::initProfiles()" << endl;

    m_prof = new ProfileManager();
    m_prof->load();
    initProfileList();
    slotProfile();

    kdDebug() << "KitchenSync::initProfiles() done" << endl;
}

Profile KitchenSync::currentProfile() const
{
    return m_prof->currentProfile();
}

ProfileManager* KitchenSync::profileManager() const
{
    return m_prof;
}


#if 0
// do we need to change the Konnector first?
// raise overview and then pipe informations
// when switching to KSyncee/KSyncEntry we will make
// it asynchronus
void KitchenSync::slotSync( Konnector *konnector, SynceeList lis)
{
    if ( konnector != currentKonnectorProfile().konnector() ) {
        emit partError( 0, Error(i18n("A Konnector wanted to sync but it's not the current one") ) );
        KonnectorManager::self()->write( konnector, lis );
        return;
    }
    if ( m_isSyncing ) {
        emit partError( 0, Error(i18n("A sync is currently taking place. We will just ignore this request.") ) );
        return;
    }
    m_isSyncing = true;
    emit startSync();
    m_outSyncee.clear();
    m_inSyncee = lis;
    kdDebug(5210) << "KitchenSync::Start sync" << endl;
    m_partsIt = new QPtrListIterator<ManipulatorPart>( m_parts );

    ManipulatorPart *part = m_partsIt->current();
    if ( part ) {
        kdDebug(5210) << "Syncing first " << endl;
        emit startSync( part );
        emit syncProgress( part, 0, 0 );
        part->sync( m_inSyncee, m_outSyncee );
    } else {
        emit partProgress( 0, Progress(i18n("Error could not start syncing with the parts.") ) );
        delete m_partsIt;
        m_partsIt = 0;
        KonnectorManager::self()->write( konnector, lis );
        m_isSyncing = false;
    }
}
#endif

/*
 * configure profiles
 */
void KitchenSync::slotConfigProf()
{
    ProfileDialog dlg( m_prof->profiles(), m_partsLst ) ;
    if ( dlg.exec() ) {
        m_prof->setProfiles( dlg.profiles() );
        m_prof->save();
        // switch profile
        initProfileList();
        slotProfile();
    }
}

void KitchenSync::switchProfile( const Profile &prof )
{
    m_bar->clear();

    m_parts.setAutoDelete( true );
    m_parts.clear();
    delete m_partsIt;
    m_partsIt = 0;

    ManPartService::ValueList lst = prof.manParts();
    ManPartService::ValueList::Iterator it;
    for (it = lst.begin(); it != lst.end(); ++it ) {
        addPart( (*it) );
    }
    Profile oldprof = m_prof->currentProfile();
    m_prof->setCurrentProfile( oldprof );
    emit profileChanged( prof );
}

void KitchenSync::addPart( const ManPartService &service )
{
    ManipulatorPart *part = KParts::ComponentFactory
                            ::createInstanceFromLibrary<ManipulatorPart>( service.libname().local8Bit(),
                                                         this );
    if ( part ) addModPart( part );
}

/*
 * configure current loaded
 * we will hack our SyncAlgo configurator into  that
 * that widget
 */
void KitchenSync::slotConfigCur()
{
    ConfigureDialog *dlg = new ConfigureDialog(this);
    ManipulatorPart *part = 0;
    SyncConfig* conf = new SyncConfig( currentProfile().confirmDelete(), currentProfile().confirmSync() );
    dlg->addWidget( conf, i18n("General"), new QPixmap( KGlobal::iconLoader()->loadIcon("package_settings", KIcon::Desktop, 48 ) ) );

    for (part = m_parts.first(); part != 0; part = m_parts.next() ) {
        if( part->configIsVisible() )
            dlg->addWidget(part->configWidget(),
                           part->name(),
                           part->pixmap() );
    }
    if (dlg->exec()) {
        Profile prof =  currentProfile();
        prof.setConfirmSync( conf->confirmSync() );
        prof.setConfirmDelete( conf->confirmDelete() );
        profileManager()->replaceProfile( prof );
        profileManager()->setCurrentProfile( prof );

        for (part = m_parts.first(); part != 0; part = m_parts.next() ) {
            part->slotConfigOk();
        }
    }
    delete dlg;
    m_prof->save();
}

/*
 * the Profile was changed in the Profile KSelectAction
 */
void KitchenSync::slotProfile()
{
    int item = mActionManager->currentProfile();
    if ( item < 0 ) item = 0; // for initialisation
    if ( m_prof->count() == 0 ) return;

    Profile cur = m_prof->profile( item );

    switchProfile( cur );
    m_prof->setCurrentProfile( cur );
}

void KitchenSync::initProfileList()
{
    Profile::ValueList list = m_prof->profiles();
    Profile::ValueList::Iterator it;
    QStringList lst;
    for (it = list.begin(); it != list.end(); ++it ) {
        lst << (*it).name();
    }
    mActionManager->setProfiles( lst );
}

SyncUi* KitchenSync::syncUi()
{
    m_syncUi = new SyncUiKde( this, currentProfile().confirmDelete(), true );
    return m_syncUi;
}

SyncAlgorithm* KitchenSync::syncAlgorithm()
{
    m_syncAlg = new PIMSyncAlg( syncUi() );

    return m_syncAlg;
}

const QPtrList<ManipulatorPart> KitchenSync::parts() const
{
    return m_parts;
}

void KitchenSync::slotKonnectorProg( Konnector *konnector,
                                         const Progress & prog )
{
    switch( prog.code() ) {
    case Progress::Connected:
//        m_konBar->setState( true );
        m_tray->setState( true );
        break;
    case Progress::Done:
        emit doneSync();
        m_isSyncing = false;
        break;
    default:
        break;
    }
    emit konnectorProgress( konnector, prog );
}

void KitchenSync::slotKonnectorErr( Konnector *konnector,
                                        const Error & prog )
{
    switch( prog.code() ) {
      case Error::ConnectionLost: // fall through
      case Error::CouldNotConnect:
//        m_konBar->setState( false );
        m_tray->setState( false );
        break;
      case Error::CouldNotDisconnect:
//        if ( konnector->isConnected() ) m_konBar->setState( true );
        m_tray->setState( true );
      default:
        break;
    }
    emit konnectorError( konnector, prog );
}

/*
 * emitted when one part is done with syncing
 * go to the next part and continue
 */
void KitchenSync::slotPartProg( ManipulatorPart* par, int prog )
{
    kdDebug(5210) << "PartProg: " << par << " " << prog << endl;
    if (prog != 2 ) return;

}

void KitchenSync::slotPartProg( ManipulatorPart* part, const Progress& prog)
{
    emit partProgress( part, prog );
    emit syncProgress( part, 1, 0 );
}

void KitchenSync::slotPartErr( ManipulatorPart* part, const Error& err )
{
    emit partError( part, err );
    emit syncProgress( part, 3, 0 );
}

void KitchenSync::slotPartSyncStatus( ManipulatorPart* par, int err )
{
    kdDebug(5210) << "SyncStatus: " << par << " " << err << endl;
#if 0
    emit doneSync( par );
    emit syncProgress( par, 2, 0 );
    // done() from ManipulatorPart now go on to the next ManipulatorPart...
    ++(*m_partsIt);
    ManipulatorPart* part = m_partsIt->current();
    if ( part ) {
        kdDebug(5210) << "Syncing " << part->name() << endl;
        emit startSync( part );
        emit syncProgress( part, 0, 0 );
        part->sync( m_inSyncee, m_outSyncee );
    } else { // we're done go write it back
        emit partProgress( 0, Progress(i18n("Going to write the information back now.") ) );
        delete m_partsIt;
        m_partsIt = 0;
        kdDebug(5210) << "Going to write back " << m_outSyncee.count() << endl;
        KonnectorManager::self()->write( currentKonnectorProfile().konnector(),
                                         m_outSyncee );
        // now we only wait for the done
    }
#endif
}

QWidget* KitchenSync::widgetStack()
{
    return m_stack;
}

void KitchenSync::slotKonnectorBar( bool b )
{
    kdDebug(5210) << "slotKonnectorBar " << b << endl;

#if 0
    Konnector *k = currentKonnectorProfile().konnector();
    if ( b ) {
        if ( k->isConnected() ) {
            kdDebug(5210) << "Going to connect " << endl;
            k->connectDevice();
        }
    } else {
        kdDebug(5210) << "disconnecting " << endl;
        k->disconnectDevice();
        m_konBar->setState( b );
        m_tray->setState( b );
    }
#endif
}

void KitchenSync::slotPreferences()
{
  KCMultiDialog *dialog = new KCMultiDialog( "PIM", this );
  connect( dialog, SIGNAL( applyClicked() ), SLOT( updateConfig() ) );
  connect( dialog, SIGNAL( okClicked() ), SLOT( updateConfig() ) );

  dialog->addModule( "Settings/Components/kresources.desktop" );

  dialog->show();
  dialog->raise();
}

void KitchenSync::updateConfig()
{
}

#include "kitchensync.moc"

/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002,2003 Maximilian Reiﬂ <harlekin@handhelds.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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
#include <konnector.h>
#include <error.h>
#include <progress.h>

#include "syncconfig.h"
#include "configuredialog.h"
#include "partbar.h"
#include "profiledialog.h"
#include "engine.h"
#include "konnectorbar.h"
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
  : Core( parent ), mActionManager( actionManager ), m_profileManager( 0 )
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

  QWidget *test = new QWidget( m_stack );
  test->setBackgroundColor( Qt::red );
  m_stack->addWidget( test, 0 );
  m_stack->raiseWidget( 0 );
  m_bar->setMaximumWidth( 100 );
  m_bar->setMinimumWidth( 100 );

  connect( m_bar, SIGNAL( activated( ActionPart * ) ),
           SLOT( slotActivated( ActionPart * ) ) );

  resize( 600, 400 );

  m_parts.setAutoDelete( true );

  initSystray();
  m_tray->show();

  mEngine = new Engine( m_parts );

  KonnectorManager *m = KonnectorManager::self();
  connect( m, SIGNAL( synceesRead( Konnector * ) ),
           mEngine, SLOT( slotSynceesRead( Konnector * ) ) );
  connect( m, SIGNAL( synceeReadError( Konnector * ) ),
           mEngine, SLOT( slotSynceeReadError( Konnector * ) ) );
  connect( m, SIGNAL( synceesWritten( Konnector * ) ),
           mEngine, SLOT( slotSynceesWritten( Konnector * ) ) );
  connect( m, SIGNAL( synceeWriteError( Konnector * ) ),
           mEngine, SLOT( slotSynceeWriteError( Konnector * ) ) );
}

KitchenSync::~KitchenSync()
{
  writeProfileConfig();

  m_profileManager->save();

  delete m_profileManager;
}

void KitchenSync::readProfileConfig()
{
  KConfig *config = KGlobal::config();
  config->setGroup( "Layout_" + currentProfile().uid() );
  m_bar->selectPart( config->readEntry( "CurrentPart" ) );
}

void KitchenSync::writeProfileConfig()
{
  KConfig *config = KGlobal::config();
  config->setGroup( "Layout_" + currentProfile().uid() );
  if ( m_bar->currentItem() && m_bar->currentItem()->part() ) {
    config->writeEntry( "CurrentPart", m_bar->currentItem()->part()->name() );
  
    config->sync();
  }
}

void KitchenSync::addPart( const ActionPartService &service )
{
    kdDebug() << "KitchenSync::addPart() " << service.name() << endl;

    ActionPart *part = KParts::ComponentFactory
      ::createInstanceFromLibrary<ActionPart>( service.libname().local8Bit(),
                                               this );
  
    if ( !part ) {
      kdError() << "Unable to create part '" << service.name() << "'"
                << endl;
      return;
    }

// We don't need this anymore. The Engine class takes care of the signals/slots.
#if 0
    /* the usual signal and slots */
    connect( part, SIGNAL(sig_progress(ActionPart*, int ) ),
             SLOT(slotPartProg(ActionPart*, int ) ) );
    connect( part, SIGNAL(sig_progress(ActionPart*, const Progress& ) ),
             SLOT(slotPartProg(ActionPart*, const Progress& ) ) );
    connect( part, SIGNAL(sig_error(ActionPart*, const Error& ) ),
             SLOT(slotPartErr(ActionPart*, const Error& ) ) );
    connect( part, SIGNAL(sig_syncStatus(ActionPart*, int) ),
             SLOT(slotPartSyncStatus( ActionPart*, int ) ) );

    KonnectorManager *m = KonnectorManager::self();
    connect( m, SIGNAL( synceesRead( Konnector * ) ),
             part, SLOT( slotSynceesRead( Konnector * ) ) );
    connect( m, SIGNAL( synceeReadError( Konnector * ) ),
             part, SLOT( slotSynceeReadError( Konnector * ) ) );
    connect( m, SIGNAL( synceesWritten( Konnector * ) ),
             part, SLOT( slotSynceesWritten( Konnector * ) ) );
    connect( m, SIGNAL( synceeWriteError( Konnector * ) ),
             part, SLOT( slotSynceeWriteError( Konnector * ) ) );
#endif

    if ( part->hasGui() )  {
        kdDebug(5210) << "Part has GUI (" << part->name() << ")" << endl;
        int pos = -1;
        
        QWidget *topWidget = new QWidget( m_stack );

        QBoxLayout *frameLayout = new QHBoxLayout( topWidget );
        frameLayout->addSpacing( KDialog::spacingHint() );

        QBoxLayout *topLayout = new QVBoxLayout( frameLayout );
        topLayout->setSpacing( KDialog::spacingHint() );

        QLabel *label = new QLabel( "<h3>" + part->title() + "</h3>",
                                    topWidget );
        topLayout->addWidget( label );

        QWidget *partWidget = part->widget();
        partWidget->reparent( topWidget, 0, QPoint( 0, 0 ) );
        topLayout->addWidget( partWidget );
        
        m_stack->addWidget( topWidget );

        mActionWidgetMap.insert( part, topWidget );

        /* overview is special for us ;) */
        if ( part->type() == i18n("Overview") ) {
            m_stack->raiseWidget( mActionWidgetMap[ part ] );
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

  mEngine->go();
}

void KitchenSync::slotActivated( ActionPart *part )
{
    emit partChanged( part );
    m_stack->raiseWidget( mActionWidgetMap[ part ] );
//    createGUI( part );
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

    m_profileManager = new ProfileManager();
    m_profileManager->load();
    initProfileList();
    activateProfile();

    kdDebug() << "KitchenSync::initProfiles() done" << endl;
}

Profile KitchenSync::currentProfile() const
{
    return m_profileManager->currentProfile();
}

ProfileManager *KitchenSync::profileManager() const
{
    return m_profileManager;
}

Engine *KitchenSync::engine() const
{
  return mEngine;
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
    m_partsIt = new QPtrListIterator<ActionPart>( m_parts );

    ActionPart *part = m_partsIt->current();
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

void KitchenSync::configureProfiles()
{
    ProfileDialog dlg( m_profileManager->profiles(),
                       ActionPartService::availableParts() );
    if ( dlg.exec() ) {
        m_profileManager->setProfiles( dlg.profiles() );
        m_profileManager->save();
        // switch profile
        initProfileList();
        activateProfile();
    }
}

void KitchenSync::activateProfile()
{
    int item = mActionManager->currentProfile();
    if ( item < 0 ) item = 0; // for initialization
    if ( m_profileManager->count() == 0 ) return;

    Profile currentProfile = m_profileManager->profile( item );

    activateProfile( currentProfile );
    m_profileManager->setCurrentProfile( currentProfile );
}

void KitchenSync::activateProfile( const Profile &prof )
{
    kdDebug() << "KitchenSync::activateProfile(): " << prof.name() << endl;

    if ( prof.uid() == m_profileManager->currentProfile().uid() ) {
      kdDebug() << "Profile already active" << endl;
      return;
    }

    writeProfileConfig();

    m_bar->clear();

    m_parts.setAutoDelete( true );
    m_parts.clear();
    delete m_partsIt;
    m_partsIt = 0;

    ActionPartService::List lst = prof.actionParts();
    ActionPartService::List::Iterator it;
    for (it = lst.begin(); it != lst.end(); ++it ) {
        addPart( (*it) );
    }
    m_profileManager->setCurrentProfile( prof );
    emit profileChanged( prof );

    readProfileConfig();
}

/*
 * configure current loaded
 * we will hack our SyncAlgo configurator into  that
 * that widget
 */
void KitchenSync::configureCurrentProfile()
{
    ConfigureDialog *dlg = new ConfigureDialog(this);
    ActionPart *part = 0;
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
    m_profileManager->save();
}

void KitchenSync::initProfileList()
{
    Profile::List list = m_profileManager->profiles();
    Profile::List::Iterator it;
    QStringList lst;
    for ( it = list.begin(); it != list.end(); ++it ) {
        lst << (*it).name();
    }
    mActionManager->setProfiles( lst );
}

SyncUi *KitchenSync::syncUi()
{
    m_syncUi = new SyncUiKde( this, currentProfile().confirmDelete(), true );
    return m_syncUi;
}

SyncAlgorithm *KitchenSync::syncAlgorithm()
{
    m_syncAlg = new PIMSyncAlg( syncUi() );

    return m_syncAlg;
}

const QPtrList<ActionPart> KitchenSync::parts() const
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
void KitchenSync::slotPartProg( ActionPart *par, int prog )
{
    kdDebug(5210) << "PartProg: " << par << " " << prog << endl;
    if (prog != 2 ) return;

}

void KitchenSync::slotPartProg( ActionPart *part, const Progress &prog )
{
    emit partProgress( part, prog );
    emit syncProgress( part, 1, 0 );
}

void KitchenSync::slotPartErr( ActionPart *part, const Error &err )
{
    emit partError( part, err );
    emit syncProgress( part, 3, 0 );
}

void KitchenSync::slotPartSyncStatus( ActionPart *par, int err )
{
    kdDebug(5210) << "SyncStatus: " << par << " " << err << endl;
#if 0
    emit doneSync( par );
    emit syncProgress( par, 2, 0 );
    // done() from ActionPart now go on to the next ActionPart...
    ++( *m_partsIt );
    ActionPart *part = m_partsIt->current();
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

QWidget *KitchenSync::widgetStack()
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
  KCMultiDialog *dialog = new KCMultiDialog( this );
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

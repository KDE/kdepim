/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>
   by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "kmobiletools_mainpart.h"

// KDE4 includes
#include <KParts/GenericFactory>
#include <KAction>
#include <KStandardAction>
#include <KActionCollection>
#include <KStandardDirs>
#include <KGlobal>
#include <KApplication>
#include <KTar>
#include <KStandardGuiItem>
#include <KSystemTrayIcon>
#include <KMessageBox>
#include <KIconLoader>
#include <KHTMLView>
#include <KRun>
#include <KInputDialog>
#include <KPluginInfo>
#include <KNotifyConfigWidget>
#include <KLocale>

// KDE3 includes
#include <QTreeWidget>

// Qt includes
#include <QSplitter>
#include <QTimer>
#include <QStackedWidget>

// KMobileTools library includes
#include <libkmobiletools/kmobiletools_cfg.h>
#include <libkmobiletools/devicesconfig.h>
#include <libkmobiletools/homepage.h>
#include <libkmobiletools/aboutdata.h>
#include <libkmobiletools/contactslist.h>
#include <libkmobiletools/engineslist.h>
#include <libkmobiletools/engine.h>
#include <libkmobiletools/kmobiletoolshelper.h>

#include "devicemanager.h"
#include "devicehome.h"
#include "errorlogdialog.h"

#define CURCFGVER 20070329

typedef KParts::GenericFactory<kmobiletoolsMainPart> kmobiletoolsMainPartFactory;
K_EXPORT_COMPONENT_FACTORY( libkmobiletoolsmainpart, kmobiletoolsMainPartFactory )

KAboutData *kmobiletoolsMainPart::createAboutData()
{
    return new AboutData();
}


kmobiletoolsMainPart::kmobiletoolsMainPart( QWidget *parentWidget, QObject *parent, const QStringList &args )
    : /*DCOPObject("KMobileTools"),*/ ///@TODO port to dbus
    KParts::ReadOnlyPart( parent ),
    l_devicesList()
{
    Q_UNUSED(args)

    // set our XML-UI resource file
    setXMLFile("kmobiletools_mainpart.rc");

    // setup gui
    setupGUI( parentWidget );

    // setup actions
    setupActions();

    // setup additional dialogs like Device Manager
    setupDialogs();

    // setup homepage part
    p_homepage=new KMobileTools::homepagePart( m_widget );
    m_widget->addWidget( p_homepage->view() );
    connect( p_homepage, SIGNAL(switchDevice(const QString& )), SLOT(switchPart(const QString& ) ) );
    connect( p_homepage, SIGNAL(loadDevice(const QString& )), SLOT(loadDevicePart(const QString&) ) );
    connect( p_homepage, SIGNAL(unloadDevice(const QString& )), SLOT(deleteDevicePart(const QString&) ) );
    connect( p_homepage, SIGNAL(configCmd(const QString& ) ), SLOT(configSlot(const QString& ) ) );
    connect( this, SIGNAL(devicesUpdated()), p_homepage, SLOT(printIndexPage() ));
    connect( this, SIGNAL(devicesUpdated() ), this, SLOT(checkShowDeviceList()) );

    // Placing KMobileTools in systray
    p_sysTray = new KSystemTrayIcon( "kmobiletools", parentWidget );
    p_sysTray->show();
    KMobileTools::KMobiletoolsHelper::instance()->setSystray( p_sysTray );
    connect( p_sysTray, SIGNAL(quitSelected()), SLOT(slotQuit()) );

    // create extended status bar
    p_statusBarExtension = new KParts::StatusBarExtension( this );

    updateStatus();
    switchPart( "homepage" );

    if( !checkConfigVersion() )
        return;

    QTimer::singleShot( 1000, this, SLOT(slotAutoLoadDevices() ) );

    /// @TODO move to AT engine
    /*
    QFile testfile(QString("/var/lock/testLock.%1").arg(KDateTime::currentLocalDateTime().toTime_t() ) );
    if(testfile.open( QIODevice::WriteOnly ) )
    {
        testfile.close();
        testfile.remove();
    } else
    {
        int ret=KMessageBox::questionYesNo( m_widget, i18n("<qt><p>You have no write access to lockfiles directory <b>/var/lock/</b>. Please correct this using the permission fixer wizard, or by hand with \"chmod -R a+rwx /var/lock\"</p><p>Do you want to run the Permission Wizard now?</p></qt>"), i18n("Locking Failed"), KGuiItem(i18n("Run Wizard")), KStandardGuiItem::cancel() );
        if(ret==KMessageBox::Yes)
            KRun::runCommand( "kmtsetup", m_widget->window() );
    }
    */
}



kmobiletoolsMainPart::~kmobiletoolsMainPart()
{
    kDebug() << "kmobiletoolsMainPart::~kmobiletoolsMainPart()\n";
}


void kmobiletoolsMainPart::slotAutoLoadDevices()
{
    QStringList sl_parts = KMobileTools::MainConfig::devicelist();
    for ( QStringList::Iterator it = sl_parts.begin(); it != sl_parts.end(); ++it ) {
        if( DEVCFG( *it )->autoload() )
            loadDevicePart(*it, false );
    }
}


void kmobiletoolsMainPart::loadDevicePart( const QString &deviceName, bool setActive )
{
    kDebug() << "KMobileTools::EnginesList::instance()->locklist(): " << KMobileTools::EnginesList::instance()->locklist() << endl;

    // try to lock device
    if( !KMobileTools::EnginesList::instance()->lock(deviceName) )
        return;

    DeviceHome *newPart = new DeviceHome( m_widget, deviceName, this );
    if( !newPart ) {
        /// @TODO add call to ErrorHandler here
        KMobileTools::EnginesList::instance()->unlock( deviceName );
        return;
    }

    m_widget->addWidget( newPart->widget() );
    l_devicesList.append( newPart );

    connect( newPart, SIGNAL(connected() ), SLOT(deviceConnected() ) );
    connect( newPart, SIGNAL(disconnected() ), SLOT(deviceDisconnected() ) );
    connect( newPart, SIGNAL(setStatusBarText(const QString&) ), this, SIGNAL(setStatusBarText(const QString&) ) );
    connect( newPart, SIGNAL(command( const QString& )), this, SLOT(configSlot( const QString &)) );
    connect( newPart, SIGNAL(deleteThis( const QString &)), this, SLOT(deleteDevicePart( const QString& )) );
    connect( newPart, SIGNAL(phonebookUpdated()), this, SLOT(phonebookUpdated()) );

    DEVCFG(deviceName)->setLoaded( true );
    emit devicesUpdated();
    emit deviceChanged( deviceName );

    if( setActive )
        switchPart( deviceName );
}


void kmobiletoolsMainPart::updateStatus()
{
    p_homepage->printIndexPage();
    checkShowDeviceList();
}

void kmobiletoolsMainPart::checkShowDeviceList()
{
    if( l_devicesList.count() != 0 )
        p_listview->show();
    else
        p_listview->hide();
}


void kmobiletoolsMainPart::switchPart( const QString& partName )
{
    if (! partName.length() )
        return;

    if ( partName == "homepage" ) {
        goHome();
        return;
    }

    int found = l_devicesList.find( partName );
    if ( found == -1 ) {
        // device is currently not loaded, so load now
        loadDevicePart( partName, true );
        return;
    } else {
        // device loaded, switch view
        m_widget->setCurrentWidget( l_devicesList.at( found )->widget() );
    }
}


void kmobiletoolsMainPart::goHome()
{
    m_widget->setCurrentIndex( 0 );
}


void kmobiletoolsMainPart::nextPart()
{
    if ( l_devicesList.isEmpty() )
        return;

    else if( l_devicesList.last()->widget() == m_widget->currentWidget() ) {
        goHome();
        return;
    }

    else if( m_widget->currentWidget() == p_homepage->view() ) {
        m_widget->setCurrentWidget( l_devicesList.first()->widget() );
        return;
    }

    int currentPosition = l_devicesList.find( m_widget->currentWidget() );
    m_widget->setCurrentWidget( l_devicesList.at( currentPosition+1 )->widget() );
}


void kmobiletoolsMainPart::prevPart()
{
    if ( l_devicesList.isEmpty() )
        return;

    else if( l_devicesList.first()->widget() == m_widget->currentWidget() ) {
        goHome();
        return;
    }

    else if( m_widget->currentWidget() == p_homepage->view() ) {
        m_widget->setCurrentWidget( l_devicesList.last()->widget() );
        return;
    }

    int currentPosition = l_devicesList.find( m_widget->currentWidget() );
    m_widget->setCurrentWidget( l_devicesList.at( currentPosition-1 )->widget() );
}


void kmobiletoolsMainPart::configSlot( const QString& command )
{
    if( command == "newDevWiz" ) {
        m_deviceManager->show();
        m_deviceManager->slotNewDevice();
    }
    else if( command=="configDevices" )
        m_deviceManager->show();
    else if(command.contains( "configure:") ) {
        m_deviceManager->show();
        m_deviceManager->showDeviceConfigDialog( command.section( ':',1,1) );
    }
}


void kmobiletoolsMainPart::addDevice( const QString& newDevice )
{
    loadDevicePart( newDevice, false );
    updateStatus();
}


void kmobiletoolsMainPart::delDevice( const QString &delDevice )
{
    deleteDevicePart( delDevice );
    updateStatus();
}


void kmobiletoolsMainPart::deleteDevicePart( const QString& deviceName )
{
    int found = l_devicesList.find( deviceName );
    if( found == -1 )
        return;

    goHome();

    m_widget->removeWidget( l_devicesList.at(found)->widget() );
    KMobileTools::Engine *t_engine = KMobileTools::EnginesList::instance()->find( deviceName );
    if( t_engine ) {
        t_engine->queryClose();
        KMobileTools::EnginesList::instance()->unlock( deviceName );
        delete t_engine;
    }

    QTreeWidgetItemIterator it( p_listview );
    while ( *it ) {
        kDebug() << KMobileTools::DevicesConfig::deviceGroup((*it)->text(0)) << "==" << deviceName << endl;
        if ( KMobileTools::DevicesConfig::deviceGroup((*it)->text(0))==deviceName )
        {
            delete *it;
            break;
        }
        ++it;
    }
//     delete l_devicesList.take(found);
    DEVCFG(deviceName)->setLoaded(false);

    emit devicesUpdated();
    emit deviceChanged(deviceName);
}


void kmobiletoolsMainPart::listviewClicked( QTreeWidgetItem* item, int column )
{
    if( !item )
        return;

    DeviceListViewItem *deviceItem;
    if( item->parent() )
        deviceItem = static_cast<DeviceListViewItem*>( item->parent() );
    else
        deviceItem = static_cast<DeviceListViewItem*>( item );

    switchPart( deviceItem->deviceName() );

    int index = l_devicesList.find( deviceItem->deviceName() );
    if( index != -1 )
        l_devicesList.at(index)->clicked( item );
}


void kmobiletoolsMainPart::slotQuit()
{
    kDebug() << "@@@@@@@@@@@@@@@@@@@ Debugging closing: KMobileTools::EnginesList::instance()->queryClose();\n";
    KMobileTools::EnginesList::instance()->queryClose();
    kDebug() << "@@@@@@@@@@@@@@@@@@@ Debugging closing: delete this;\n";
    delete this;
    kDebug() << "@@@@@@@@@@@@@@@@@@@ Debugging closing: kapp->quit();\n";
    kapp->quit();
}

bool kmobiletoolsMainPart::deviceIsLoaded( const QString &deviceName )
{
    if( l_devicesList.find( deviceName ) == -1 )
        return false;

    return true;
}


void kmobiletoolsMainPart::deviceDisconnected()
{
    /// @todo implement me
    updateStatus();
}


void kmobiletoolsMainPart::deviceConnected()
{
    /// @todo implement me
    updateStatus();
    p_sysTray->parentWidget()->window()->show(); // @TODO does this work?! untested...
}


void kmobiletoolsMainPart::widgetStackItemChanged( int item )
{
    // Find the current visible item, and remove the statusbar if it's not the HomePagePart
    if( m_widget->currentWidget() != p_homepage->view() ) {
        DeviceHome *oldPart = l_devicesList.at( l_devicesList.find(m_widget->currentWidget()) );
        oldPart->clearStatusBar();
        unplugActionList( "kmobiletools_devicepart.rc" );
    }

    if( !item )
        return; // exit if destination widget is the HomePagePart

    DeviceHome *newPart = l_devicesList.at( l_devicesList.find( m_widget->widget( item ) ) );
    newPart->setupStatusBar();
    plugActionList( "kmobiletools_devicepart.rc", newPart->actionList() );
}

#include "kmobiletools_mainpart.moc"

/*
void kmobiletoolsMainPart::newSMS()
{
    if( ! KMobileTools::EnginesList::instance()->count() )
    {
        KMessageBox::error( m_widget, i18n("You should load a mobile phone first.") );
        return;
    }
    if( KMobileTools::EnginesList::instance()->count() == 1 )
    {
//         DeviceIFace_stub( "kmobiletools", KMobileTools::EnginesList::instance()->namesList(false).first().latin1() ).slotNewSMS() ;
        /// @TODO fix this, with a singleton class
        return;
    }
    bool ok;
    QString engine=KInputDialog::getItem( i18nc("Select engine for new sms dialog", "Select engine"), i18n("<qt>You have loaded multiple mobile phones.<br/>Choose the one for the new sms.</qt>"),
    KMobileTools::EnginesList::instance()->namesList(true), 0, false, &ok, m_widget);
    if(!ok) return;
    engine=KMobileTools::EnginesList::instance()->find( engine, true)->objectName();
//     DeviceIFace_stub( "kmobiletools", engine.latin1() ).slotNewSMS() ; @TODO port this
}
*/

bool kmobiletoolsMainPart::checkConfigVersion()
{
    return true; // it doesn't seem to work perfectly in the kde4 port, and we've other stuff to fix first
    // @TODO fix this, anyway
    uint cfgver=KMobileTools::MainConfig::self()->configversion();
    if(cfgver>=CURCFGVER) return true;
    kDebug() << "Checking config version::" << cfgver << endl;
    QDir cfgdir(KGlobal::dirs()->saveLocation("config") );
    QStringList entries=cfgdir.entryList( QStringList() << "*kmobiletools*", QDir::Files );
    if(entries.isEmpty())
    {
        kDebug() << "No config files found, skipping checkConfigVersion;\n";
        return true;
    }
    QString archiveName=KGlobal::dirs()->saveLocation("tmp") + "kmobiletools-" +
            QDate::currentDate().toString(Qt::ISODate) + ".tar.gz";
    KMessageBox::information( widget(), i18n("<qt><p>KMobileTools has found an old or invalid configuration file.</p><p>To work correctly, it needs to delete your configuration files. Your old files will be saved in <b>%1</b></p></qt>", archiveName ) );
    KTar arch(archiveName);
    if(!arch.open( QIODevice::WriteOnly))
    {
        KMessageBox::error( widget(), i18n("<qt><p>KMobileTools could not archive your config files.</p><p>Please remove them manually.</p></qt>") );
        return true;
    }
    for(QStringList::Iterator it=entries.begin(); it!=entries.end(); ++it)
    {
        arch.addLocalFile( cfgdir.path() + QDir::separator() + (*it), (*it));
        QFile::remove( cfgdir.path() + QDir::separator() + (*it) );
        kDebug() << "Entry ::" << cfgdir.path() + QDir::separator() + (*it) << " archived and removed." << endl;
    }
    arch.close();
    KMessageBox::information(widget(), i18n("<qt><p>Your old configuration files were saved in <b>%1</b>.</p><p>KMobileTools will now close. You can restart it.</p></qt>", archiveName));
    KMobileTools::MainConfig::self()->readConfig();
    KMobileTools::MainConfig::self()->setConfigversion(CURCFGVER);
    KMobileTools::MainConfig::self()->writeConfig();
    QTimer::singleShot(300, this, SLOT(slotQuit()) );
    return false;
}


void kmobiletoolsMainPart::phonebookUpdated()
{
    // Some engine got the phonebook updated, updating then all device parts phonebooks.
    for( int i=0; i<l_devicesList.size(); i++ )
        l_devicesList.at(i)->updateAllContacts();
}


void kmobiletoolsMainPart::slotConfigNotify()
{
    KNotifyConfigWidget::configure(m_widget, 0);
}


void kmobiletoolsMainPart::setupGUI( QWidget* parent ) {
    QSplitter *splitter = new QSplitter( parent );
    splitter->setObjectName( QString("kmobiletools-splitter") );

    // create devices list-view
    p_listview = new QTreeWidget( splitter );
    p_listview->setColumnCount( 1 );
    p_listview->setHeaderLabel( i18n( "Devices" ) );
    p_listview->setMinimumWidth( 200 );
    p_listview->setRootIsDecorated( true );

    m_widget = new QStackedWidget( splitter );

    splitter->setStretchFactor( splitter->indexOf( p_listview ), 0 );
    splitter->setStretchFactor( splitter->indexOf( m_widget ), 1 );
    // notify the part that this is our internal widget
    setWidget( splitter );

    connect(m_widget, SIGNAL( currentChanged ( int ) ), this, SLOT(widgetStackItemChanged( int )) );
    connect(p_listview, SIGNAL(itemClicked(QTreeWidgetItem*, int)), 
            this, SLOT(listviewClicked(QTreeWidgetItem*, int)) );
    /// @TODO is there a better way to auto-expand all items on change?
    connect(p_listview, SIGNAL(itemChanged(QTreeWidgetItem*, int)), 
            p_listview, SLOT(expandAll()) );
}


void kmobiletoolsMainPart::setupActions() {
    KAction *curAction=0;

    // "Go home" action
    curAction = new KAction( KIcon("go-home") ,i18n("Homepage"), this );
    connect( curAction, SIGNAL(triggered(bool)), this, SLOT(goHome()) );
    actionCollection()->addAction( "home", curAction );

    // "Next" action
    actionCollection()->addAction( KStandardAction::Next, "next", this, SLOT(nextPart()) );

    // "Previous" action
    actionCollection()->addAction( KStandardAction::Prior, "prev", this, SLOT(prevPart()) );

    // "Quit" action
    actionCollection()->addAction( KStandardAction::Quit, "file_quit", this, SLOT(slotQuit()) );

    // "Configure notifications" action
    actionCollection()->addAction( KStandardAction::ConfigureNotifications, "options_configure_notifications",
                                   this, SLOT(slotConfigNotify() ) );
}

void kmobiletoolsMainPart::setupDialogs() {
    // create device manager
    m_deviceManager=new DeviceManager( m_widget );
    connect( m_deviceManager, SIGNAL(deviceAdded(const QString& ) ), this, SLOT(addDevice(const QString& )) );
    connect( m_deviceManager, SIGNAL(deviceRemoved(const QString& )), this, SLOT(delDevice(const QString& )) );
    connect( m_deviceManager, SIGNAL(loadDevice(const QString& )), this, SLOT(loadDevicePart(const QString&) ) );
    connect( m_deviceManager, SIGNAL(unloadDevice(const QString& )), this, SLOT(deleteDevicePart(const QString&) ) );
    connect( this, SIGNAL(deviceChanged(const QString& ) ), m_deviceManager, SLOT(deviceChanged(const QString& ) ) );

    // create error log dialog
    m_errorLogDialog = new ErrorLogDialog( m_widget );

    KAction *curAction=0;

    // "Device manager" action
    curAction = new KAction( KIcon("package-utilities"), i18n("Device Manager"), this );
    connect( curAction, SIGNAL(triggered(bool)), m_deviceManager, SLOT(show()) );
    actionCollection()->addAction( "device_manager", curAction );

    // "Show error log" action
    curAction = new KAction( KIcon("text-enriched"), i18n("Show error log"), this );
    connect( curAction, SIGNAL(triggered(bool)), m_errorLogDialog, SLOT(show()) );
    actionCollection()->addAction( "error_log", curAction );
}

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

// Qt includes
#include <QSplitter>
#include <QTimer>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QTreeView>
#include <QModelIndex>
#include <QProgressDialog>
#include <QMutex>
#include <QMenu>

// KMobileTools library includes
#include <libkmobiletools/enginexp.h>
#include <libkmobiletools/kmobiletools_cfg.h>
#include <libkmobiletools/homepage.h>
#include <libkmobiletools/aboutdata.h>
#include <libkmobiletools/contactslist.h>
#include <libkmobiletools/engineslist.h>
#include <libkmobiletools/engine.h>
#include <libkmobiletools/kmobiletoolshelper.h>
#include <libkmobiletools/deviceloader.h>
#include <libkmobiletools/serviceloader.h>
#include <libkmobiletools/coreservice.h>
#include <libkmobiletools/ifaces/guiservice.h>
#include <libkmobiletools/ifaces/actionprovider.h>
#include <libkmobiletools/config.h>

#include "devicemanager.h"
#include "devicehome.h"
#include "errorlogdialog.h"
#include "servicemodel/servicemodel.h"
#include "servicemodel/deviceitem.h"
#include "servicemodel/serviceitem.h"

typedef KParts::GenericFactory<kmobiletoolsMainPart> kmobiletoolsMainPartFactory;
K_EXPORT_COMPONENT_FACTORY( libkmobiletoolsmainpart, kmobiletoolsMainPartFactory )

KAboutData *kmobiletoolsMainPart::createAboutData()
{
    return new AboutData();
}


kmobiletoolsMainPart::kmobiletoolsMainPart( QWidget *parentWidget, QObject *parent, const QStringList &args )
    : /*DCOPObject("KMobileTools"),*/ ///@TODO port to dbus
    KParts::ReadOnlyPart( parent )
{
    Q_UNUSED(args)

    // set our XML-UI resource file
    setXMLFile( "kmobiletools_mainpart.rc" );

    // setup gui
    setupGUI( parentWidget );

    // setup actions
    setupActions();

    // setup additional dialogs
    setupDialogs();

    // Placing KMobileTools in systray
    p_sysTray = new KSystemTrayIcon( "kmobiletools", parentWidget );
    p_sysTray->show();
    KMobileTools::KMobiletoolsHelper::instance()->setSystray( p_sysTray );
    connect( p_sysTray, SIGNAL(quitSelected()), SLOT(slotQuit()) );

    // create extended status bar
    p_statusBarExtension = new KParts::StatusBarExtension( this );

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
    kDebug() <<"kmobiletoolsMainPart::~kmobiletoolsMainPart()";
}

bool kmobiletoolsMainPart::openFile() {
    return false;
}


void kmobiletoolsMainPart::slotAutoLoadDevices()
{
    QStringList deviceList = KMobileTools::Config::instance()->deviceList();
    for( int i=0; i<deviceList.size(); i++ ) {
        QString engine = KMobileTools::Config::instance()->engine( deviceList.at( i ) );
        KMobileTools::DeviceLoader::instance()->loadDevice( deviceList.at( i ), engine );
    }
}


void kmobiletoolsMainPart::shutDownSucceeded() {
    if( m_shutDownDialog ) {
        m_mutex.lock();
        m_shutDownDialog->setValue( m_shutDownDialog->value() + 1 );
        m_mutex.unlock();

        if( m_shutDownDialog->value() == -1 )
            slotFinallyQuit();
    }
}

void kmobiletoolsMainPart::slotFinallyQuit() {
    delete this;
    kapp->quit();
}

void kmobiletoolsMainPart::slotQuit()
{
    // shut down dialog is already open, so already shutting down
    if( m_shutDownDialog )
        return;

    QList<DeviceItem*> deviceItems = m_serviceModel->deviceItems();

    // do we need to shut down any device?
    if( deviceItems.size() ) {
        // create shut down dialog for user interaction
        m_shutDownDialog = new QProgressDialog( m_widget );
        m_shutDownDialog->setWindowTitle( i18n( "Disconnecting..." ) );
        m_shutDownDialog->setLabelText( i18n( "Disconnecting devices..." ) );
        m_shutDownDialog->setCancelButtonText( i18n( "Force disconnection" ) );
        m_shutDownDialog->setValue( 0 );
        m_shutDownDialog->setMinimumDuration( 1000 );
        m_shutDownDialog->setMaximum( 0 );
        connect( m_shutDownDialog, SIGNAL(canceled()), this, SLOT(slotFinallyQuit()) );

        for( int i=0; i<deviceItems.size(); i++ ) {
            KMobileTools::EngineXP* engine = KMobileTools::DeviceLoader::instance()->engine(
                        deviceItems.at( i )->data().toString() );
            if( engine ) {
                if( engine->connected() ) {
                    m_shutDownDialog->setMaximum( m_shutDownDialog->maximum() + 1 );
                    connect( engine, SIGNAL(deviceDisconnected()), this, SLOT(shutDownSucceeded()) );
                    QTimer::singleShot( 0, engine, SLOT(disconnectDevice()) );
                }
            }
        }

        if( m_shutDownDialog->maximum() )
            // force shut down after 10 seconds
            QTimer::singleShot( 10*1000, this, SLOT(slotFinallyQuit()) );
        else
            QTimer::singleShot( 0, this, SLOT(slotFinallyQuit()) );

    } else
        QTimer::singleShot( 0, this, SLOT(slotFinallyQuit()) );
}


#include "kmobiletools_mainpart.moc"


bool kmobiletoolsMainPart::checkConfigVersion()
{
    return true; // it doesn't seem to work perfectly in the kde4 port, and we've other stuff to fix first
    /// @TODO fix this
    /*
    uint cfgver=KMobileTools::MainConfig::self()->configversion();
    if(cfgver>=CURCFGVER) return true;
    kDebug() <<"Checking config version::" << cfgver;
    QDir cfgdir(KGlobal::dirs()->saveLocation("config") );
    QStringList entries=cfgdir.entryList( QStringList() << "*kmobiletools*", QDir::Files );
    if(entries.isEmpty())
    {
        kDebug() <<"No config files found, skipping checkConfigVersion;";
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
        kDebug() <<"Entry ::" << cfgdir.path() + QDir::separator() + (*it) <<" archived and removed.";
    }
    arch.close();
    KMessageBox::information(widget(), i18n("<qt><p>Your old configuration files were saved in <b>%1</b>.</p><p>KMobileTools will now close. You can restart it.</p></qt>", archiveName));
    KMobileTools::MainConfig::self()->readConfig();
    KMobileTools::MainConfig::self()->setConfigversion(CURCFGVER);
    KMobileTools::MainConfig::self()->writeConfig();
    QTimer::singleShot(300, this, SLOT(slotQuit()) );
    return false;
    */
}


void kmobiletoolsMainPart::slotConfigNotify()
{
    KNotifyConfigWidget::configure( m_widget, 0 );
}


void kmobiletoolsMainPart::treeItemClicked( const QModelIndex& index ) {
    // avoid flickering
    if( m_lastIndex == index )
        return;

    m_lastIndex = index;

    //unplugActionList( "serviceactions" );
    //unplugActionList( "deviceactions" );

    TreeItem* item = static_cast<TreeItem*>( index.internalPointer() );

    // service item clicked?
    ServiceItem* serviceItem = qobject_cast<ServiceItem*>( item );
    if( serviceItem )
        handleServiceItem( serviceItem );
    else
        emit showServiceToolBar(false);

    // device item clicked?
    DeviceItem* deviceItem = qobject_cast<DeviceItem*>( item );
    if( deviceItem ) {
        handleDeviceItem( deviceItem );
        emit showDeviceToolBar( true );
    }
    else
        emit showDeviceToolBar( false );
}

void kmobiletoolsMainPart::handleDeviceItem( DeviceItem* deviceItem ) {
    /// @TODO open homepage part here
    unplugActionList( "deviceactions" );
    plugActionList( "deviceactions", deviceItem->actionList() );
}

void kmobiletoolsMainPart::handleServiceItem( ServiceItem* serviceItem ) {
    QString deviceName = serviceItem->parent()->data().toString();

    //
    // Handle core services
    //
    KMobileTools::CoreService* coreService =
            qobject_cast<KMobileTools::CoreService*>( serviceItem->service() );

    if( !coreService ) {
        /// @TODO add call to error handler here
        KMessageBox::information( widget(), QString( "service is not a core service.. wrong iface impl ;-)" ) );
        return;
    }


    //
    // Handle Gui services
    //
    KMobileTools::Ifaces::GuiService* guiService =
            qobject_cast<KMobileTools::Ifaces::GuiService*>( serviceItem->service() );

    if( guiService ) {
        if( m_widget->indexOf( guiService->widget() ) == -1 ) {
            m_widget->addWidget( guiService->widget() );

            m_loadedWidgets.insert( deviceName, guiService->widget() );
        }

        m_widget->setCurrentWidget( guiService->widget() );
    }

    //
    // Handle action providers
    //
    KMobileTools::Ifaces::ActionProvider* actionProvider =
            qobject_cast<KMobileTools::Ifaces::ActionProvider*>( serviceItem->service() );

    if( actionProvider ) {
        // append service actions
        unplugActionList( "serviceactions" );
        plugActionList( "serviceactions", actionProvider->actionList() );
        if( actionProvider->actionList().size() )
            emit showServiceToolBar(true);
    } else
        emit showServiceToolBar(false);
}

void kmobiletoolsMainPart::unloadDeviceActions( const QString& deviceName ) {
    /// @TODO unplug the action list only if the currently loaded action list if from deviceName
    unplugActionList( "deviceactions" );
}

void kmobiletoolsMainPart::removeServiceWidget( const QString& deviceName, KMobileTools::CoreService* service ) {
    KMobileTools::Ifaces::GuiService* gui =
            qobject_cast<KMobileTools::Ifaces::GuiService*>( service );
    if( gui ) {
        QWidget* widget = gui->widget();
                if( m_widget->currentWidget() == widget )
                    unplugActionList( "serviceactions" );

        QWidgetList widgetList = m_loadedWidgets.values( deviceName );
        if( !widgetList.isEmpty() ) {
            if( widgetList.contains( widget ) ) {
                m_widget->removeWidget( widget );
                delete widget;
            }
        }
        m_loadedWidgets.remove( deviceName, widget );
    }
}

void kmobiletoolsMainPart::treeViewContextMenu( const QPoint& position ) {
    QMenu menu;

    // look if there's a device item at the clicked position
    TreeItem* item = static_cast<TreeItem*>( m_treeView->indexAt( position ).internalPointer() );
    if( item ) {
        DeviceItem* device = dynamic_cast<DeviceItem*>( item );
        if( device ) {
            QList<QAction*> actionList = device->actionList();
            for( int i=0; i<actionList.size(); i++ )
                menu.addAction( actionList.at( i ) );

            if( actionList.size() )
                menu.exec( m_treeView->mapToGlobal( position ) );
        }
    }
}

void kmobiletoolsMainPart::setupGUI( QWidget* parent ) {
    QSplitter *splitter = new QSplitter( parent );

    // create devices/services list-view
    m_treeView = new QTreeView( splitter );
    m_treeView->setMinimumWidth( 200 );
    m_treeView->setRootIsDecorated( true );
    m_treeView->setAnimated( true );

    m_treeView->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( m_treeView, SIGNAL(customContextMenuRequested(const QPoint&)),
             this, SLOT(treeViewContextMenu(const QPoint&)) );

    // set data model for list-view
    m_serviceModel = new ServiceModel( this );

    connect( KMobileTools::ServiceLoader::instance(),
             SIGNAL( aboutToUnloadService(const QString&, KMobileTools::CoreService*) ),
             this,
             SLOT( removeServiceWidget(const QString&, KMobileTools::CoreService*) ) );

    connect( KMobileTools::DeviceLoader::instance(),
             SIGNAL( aboutToUnloadDevice(const QString&) ),
             this,
             SLOT( deviceUnloaded(const QString&) ) );

    m_treeView->setModel( m_serviceModel );

    connect( m_treeView, SIGNAL( activated(const QModelIndex&) ),
             this, SLOT( treeItemClicked(const QModelIndex&) ) );

    connect( m_treeView, SIGNAL( clicked(const QModelIndex&) ),
             this, SLOT( treeItemClicked(const QModelIndex&) ) );

    m_widget = new QStackedWidget( splitter );

    splitter->setStretchFactor( splitter->indexOf( m_treeView ), 0 );
    splitter->setStretchFactor( splitter->indexOf( m_widget ), 1 );

    // notify the part that this is our internal widget
    setWidget( splitter );
}


void kmobiletoolsMainPart::setupActions() {
    // "Quit" action
    actionCollection()->addAction( KStandardAction::Quit, "quit", this, SLOT(slotQuit()) );

    // "Configure notifications" action
    actionCollection()->addAction( KStandardAction::ConfigureNotifications, "options_configure_notifications",
                                   this, SLOT(slotConfigNotify() ) );
}

void kmobiletoolsMainPart::setupDialogs() {
    // create device manager
    m_deviceManager=new DeviceManager( m_widget );

    // create error log dialog
    m_errorLogDialog = new ErrorLogDialog( m_widget );

    m_shutDownDialog = 0; // created on exit

    KAction *curAction=0;

    // "Device manager" action
    curAction = new KAction( KIcon("package-utilities"), i18n("Device Manager"), this );
    connect( curAction, SIGNAL(triggered(bool)), m_deviceManager, SLOT(show()) );
    actionCollection()->addAction( "device_manager", curAction );

    // "Show error log" action
    curAction = new KAction( KIcon("text-enriched"), i18n("Show error log..."), this );
    connect( curAction, SIGNAL(triggered(bool)), m_errorLogDialog, SLOT(show()) );
    actionCollection()->addAction( "error_log", curAction );
}

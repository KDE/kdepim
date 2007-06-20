/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

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

#include <kcomponentdata.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <kglobal.h>
#include <klocale.h>
#include <kapplication.h>
#include <kparts/partmanager.h>
#include <kparts/part.h>
#include <kparts/genericfactory.h>
#include <ktar.h>
// #include <kparts/statusbarextension.h>
//#include <dcopclient.h>
#include <ksystemtrayicon.h>
#include <kactioncollection.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <q3widgetstack.h>
#include <qsplitter.h>
#include <khtmlview.h>
#include <k3listview.h>
#include <kstatusbar.h>
#include <qtimer.h>
#include <krun.h>
#include <kinputdialog.h>
#include <kplugininfo.h>
#include <q3sqlpropertymap.h>
#include <knotifyconfigwidget.h>

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
// #include "devicehome.h"
//#include "deviceIFace_stub.h"


#define CURCFGVER 20070329

#define TAG_SPACING "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
typedef KParts::GenericFactory<kmobiletoolsMainPart> kmobiletoolsMainPartFactory;
K_EXPORT_COMPONENT_FACTORY( libkmobiletoolsmainpart, kmobiletoolsMainPartFactory )

KAboutData *kmobiletoolsMainPart::createAboutData()
{
    return new AboutData();
}

kmobiletoolsMainPart *kmobiletoolsMainPart::m_mainpart=0;

kmobiletoolsMainPart::kmobiletoolsMainPart( QWidget *parentWidget, QObject *parent, const QStringList &args )
    : /*DCOPObject("KMobileTools"),*/ ///@TODO port to dbus
    KParts::ReadOnlyPart(parent),
    l_devicesList()
{
    m_mainpart=this;
//     new KMobileTools::EnginesList();

    // we need an instance
//     setInstance( kmobiletoolsMainPartFactory::instance() );
    // set our XML-UI resource file
    setXMLFile("kmobiletools_mainpart.rc");
    // this should be your custom internal widget
//     if ( QString(kapp->name() ) != "kmobiletools" )
//     {
//         p_dcopClient=new DCOPClient();
//         kDebug() << "DCopClient registration as 'kmobiletools'..: " <<
//                 p_dcopClient->registerAs("kmobiletools", false) << endl;
//     } else p_dcopClient=kapp->dcopClient();

//     m_statusBarExt=new KParts::StatusBarExtension(this, "StatusBarExtension");
    QSplitter *splitter=new QSplitter( parentWidget );
    splitter->setObjectName(QLatin1String("kmobiletools-splitter"));
    p_listview=new K3ListView(splitter);
    p_listview->addColumn( i18n("Devices") );
    p_listview->setAutoOpen( true );
    p_listview->setResizeMode(K3ListView::AllColumns);
    p_listview->setRootIsDecorated(true);
    p_listview->setSizePolicy (QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred));
    p_listview->setMinimumWidth(200);
    p_listview->resize(200,p_listview->height() );
    m_widget = new Q3WidgetStack( splitter );
    // notify the part that this is our internal widget
    splitter->setResizeMode(p_listview, QSplitter::KeepSize);
    setWidget(splitter);
//     setView( m_widget );
    // create our actions
    KAction *curAction=0;
    curAction=new KAction( KIcon("package-utilities"), i18n("Device Manager"), this);
    connect(curAction, SIGNAL(triggered(bool)), this,SLOT(deviceManager()));
    actionCollection()->addAction("device_manager", curAction);
    curAction=new KAction( KIcon("go-home") ,i18n("Homepage"), this );
    connect(curAction, SIGNAL(triggered(bool)),this,SLOT(goHome()) );
    actionCollection()->addAction("home", curAction);

    curAction=KStandardAction::next(this, SLOT(nextPart() ), this);
    actionCollection()->addAction(KStandardAction::Next, "next", this, SLOT(nextPart() ) );
    actionCollection()->addAction(KStandardAction::Prior, "prev", this, SLOT(prevPart() ) );
//     actionCollection()->addAction(KStandardAction::Preferences, "options_configure", this, SLOT(showPreference() ) );
    actionCollection()->addAction(KStandardAction::Quit, "file_quit", this, SLOT(slotQuit() ) );
    actionCollection()->addAction(KStandardAction::ConfigureNotifications, "options_configure_notifications", this, SLOT(slotConfigNotify() ) );



//     partManager=new KParts::PartManager(m_widget, this);
    p_homepage=new KMobileTools::homepagePart(m_widget);
    m_widget->addWidget( p_homepage->view(), 0 );
//     partManager->addPart( p_homepage );
//     connect(partManager, SIGNAL(activePartChanged (KParts::Part *) ), this, SLOT(activePartChanged(KParts::Part* ) ) );
    // Loading startup devices

    p_sysTray=new KSystemTrayIcon("kmobiletools", parentWidget);
//     p_sysTray->setPixmap( p_sysTray->loadIcon("kmobiletools") );
    p_sysTray->show();
    KMobileTools::KMobiletoolsHelper::instance()->setSystray( p_sysTray );
    connect(p_homepage, SIGNAL(switchDevice(const QString& )), SLOT(switchPart(const QString& ) ) );
    connect(p_homepage, SIGNAL(loadDevice(const QString& )), SLOT(loadDevicePart(const QString&) ) );
    connect(p_homepage, SIGNAL(unloadDevice(const QString& )), SLOT(deleteDevicePart(const QString&) ) );
    connect(p_homepage, SIGNAL(configCmd(const QString& ) ), SLOT(configSlot(const QString& ) ) );
    connect(m_widget, SIGNAL( aboutToShow ( int ) ), this, SLOT(widgetStackItemChanged( int )) );
    connect(this, SIGNAL(devicesUpdated()), p_homepage, SLOT(printIndexPage() ));
    connect(p_listview, SIGNAL(clicked(Q3ListViewItem *)), SLOT(listviewClicked(Q3ListViewItem* ) ) );
    connect(this, SIGNAL(devicesUpdated() ), this, SLOT(devicesChanged()) );
//     connect(partManager, SIGNAL(activePartChanged (KParts::Part *)), SLOT(partChanged(KParts::Part* ) ) );
//     connect(p_sysTray, SIGNAL(quitSelected() ), kapp, SLOT(quit() ) );
    connect(p_sysTray, SIGNAL(quitSelected() ), SLOT(slotQuit() ) );
//     p_statusbar=new KStatusBar();
    p_statusBarExtension=new KParts::StatusBarExtension(this);
//     p_statusBarExtension->setStatusBar(p_statusbar);

    updateStatus();
    switchPart("homepage");
    if(!checkConfigVersion()) return;
    QTimer::singleShot( 3000, this, SLOT(slotAutoLoadDevices() ) );

    if(qApp->isSessionRestored() )
        QTimer::singleShot( 100, this, SLOT(slotHide()) );
    QFile testfile(QString("/var/lock/testLock.%1").arg(KDateTime::currentLocalDateTime().toTime_t() ) );
    if(testfile.open( QIODevice::WriteOnly ) )
    {
        testfile.close();
        testfile.remove();
    } else
    {
        int ret=KMessageBox::questionYesNo( m_widget, i18n("<qt>You have no write access to lockfiles directory <b>/var/lock/</b>. Please correct this using the permission fixer wizard, or by hand with \"chmod -R a+rwx /var/lock\"<br>Do you want to run the Permission Wizard now?</qt>") );
        if(ret==KMessageBox::Yes) KRun::runCommand("kmtsetup", m_widget->topLevelWidget());
    }
}


void kmobiletoolsMainPart::slotHide()
{
//     p_sysTray->setInactive(); @TODO port this
}

kmobiletoolsMainPart::~kmobiletoolsMainPart()
{
    kDebug() << "kmobiletoolsMainPart::~kmobiletoolsMainPart()\n";
}

void kmobiletoolsMainPart::slotAutoLoadDevices()
{
    QStringList sl_parts=KMobileTools::MainConfig::devicelist();
    for ( QStringList::Iterator it = sl_parts.begin(); it != sl_parts.end(); ++it ) {
        if( DEVCFG( *it )->autoload() )
            loadDevicePart(*it, false );
    }
}

/*!
    \fn kmobiletoolsMainPart::activePartChanged(KParts::Part *newPart)
 */
void kmobiletoolsMainPart::activePartChanged(KParts::Part *newPart)
{
    if(!newPart) return;
    m_widget->raiseWidget( newPart->widget() );
}


/*!
    \fn kmobiletoolsMainPart::loadDevicePart(const QString &devicename)
 */
void kmobiletoolsMainPart::loadDevicePart(const QString &deviceName, bool setActive)
{
    kDebug() << "KMobileTools::EnginesList::instance()->locklist(): " << KMobileTools::EnginesList::instance()->locklist() << endl;
    if(!KMobileTools::EnginesList::instance()->lock(deviceName) )return;
//     if( !sl_toloadList.isEmpty() && sl_toloadList.contains(deviceName) ) return;
    DeviceHome *newPart=new DeviceHome(m_widget, deviceName, this);
    if(!newPart) { KMobileTools::EnginesList::instance()->unlock(deviceName); return; }
    m_widget->addWidget( (QWidget*) newPart->widget() );
    l_devicesList.append(newPart);
    connect( newPart, SIGNAL(connected() ), SLOT(deviceConnected() ) );
    connect( newPart, SIGNAL(disconnected() ), SLOT(deviceDisconnected() ) );
    connect( newPart, SIGNAL(setStatusBarText(const QString&) ), this, SIGNAL(setStatusBarText(const QString&) ) );
    connect( newPart, SIGNAL(command( const QString& )), this, SLOT(configSlot( const QString &)));
    connect( newPart, SIGNAL(deleteThis( const QString &)), this, SLOT(deleteDevicePart( const QString& )) );
    connect( newPart, SIGNAL(phonebookUpdated()), this, SLOT(phonebookUpdated()));

    DEVCFG(deviceName)->setLoaded(true);
    emit devicesUpdated();
    emit deviceChanged(deviceName);
    if(setActive) switchPart( deviceName );
}


/*!
    \fn kmobiletoolsMainPart::updateStatus()
 */
void kmobiletoolsMainPart::updateStatus()
{
    p_homepage->printIndexPage();
//     devicesUpdated();
    devicesChanged();
}

void kmobiletoolsMainPart::devicesChanged()
{
    if (l_devicesList.count() != 0 )
        p_listview->show();
    else p_listview->hide();
    return;
}



/*!
    \fn kmobiletoolsMainPart::switchPart( QString  &partName )
 */
void kmobiletoolsMainPart::switchPart( const QString  &partName )
{
    kDebug() << "kmobiletoolsMainPart::switchPart( const QString  &partName == " << partName << " )\n";
    if (! partName.length() ) return;
    if ( partName == "homepage")
    {
        goHome();
        return;
    }
/*    KParts::Part* cur_part;
    QPtrList<KParts::Part>list( *(partManager->parts() ) );
    bool found=false;
    for( cur_part=list.first(); cur_part; cur_part=list.next() )
    {
        if( partName == cur_part->name() )
        {
            partManager->setActivePart( cur_part, cur_part->widget() );
            found=true;
        }
    }
    if (!found && partName!="homepage") loadDevicePart( partName, true);*/
    int found=0;
    found = l_devicesList.find(partName);
    if ( found== -1)
    {
        loadDevicePart(partName, true);
        return;
    }
    DeviceHome *curDev=l_devicesList.at(found);
    if(!curDev) return;
    QWidget *widget=curDev->widget();
    if(!widget) return;
    m_widget->raiseWidget( widget );
}


/*!
    \fn kmobiletoolsMainPart::nextPart()
 */
void kmobiletoolsMainPart::nextPart()
{

    if (l_devicesList.isEmpty() ) return;
    DeviceHome *curPart;
    if( l_devicesList.last()->widget() == m_widget->visibleWidget() )
    {
        goHome();
        return;
    }    if(m_widget->visibleWidget() == p_homepage->view() )
    {
        m_widget->raiseWidget( l_devicesList.first()->widget() );
        return;
    }
    curPart=l_devicesList.at(l_devicesList.find(m_widget->visibleWidget()));
    curPart=l_devicesList.next();
    m_widget->raiseWidget( curPart->widget() );
}

/*!
    \fn kmobiletoolsMainPart::goHome()
 */
void kmobiletoolsMainPart::goHome()
{
    m_widget->raiseWidget( 0 );
}

/*!
    \fn kmobiletoolsMainPart::prevPart()
 */
void kmobiletoolsMainPart::prevPart()
{
    if (l_devicesList.isEmpty() ) return;
    DeviceHome *curPart;
    if( l_devicesList.first()->widget() == m_widget->visibleWidget() )
    {
        goHome();
        return;
    }
    if(m_widget->visibleWidget() == p_homepage->view() )
    {
        m_widget->raiseWidget( l_devicesList.last()->widget() );
        return;
    }
    curPart=l_devicesList.at(l_devicesList.find(m_widget->visibleWidget()));
    curPart=l_devicesList.prev();
    m_widget->raiseWidget( curPart->widget() );
}

DeviceManager *kmobiletoolsMainPart::deviceManager()
{
    DeviceManager* devman=new DeviceManager(m_widget);
    devman->show();
    connect(devman, SIGNAL(deviceAdded(const QString& ) ), this, SLOT(addDevice(const QString& )) );
    connect(devman, SIGNAL(deviceRemoved(const QString& )), this, SLOT(delDevice(const QString& )) );
    connect(devman, SIGNAL(loadDevice(const QString& )), this, SLOT(loadDevicePart(const QString&) ) );
    connect(devman, SIGNAL(unloadDevice(const QString& )), this, SLOT(deleteDevicePart(const QString&) ) );
//     connect(this, SIGNAL(devicesUpdated() ), devman, SLOT(updateView() ) );
    connect(this, SIGNAL(deviceChanged(const QString& ) ), devman, SLOT(deviceChanged(const QString& ) ) );
    return devman;
}

void kmobiletoolsMainPart::showPreference()
{
    configSlot("configDevices");
}


/*!
    \fn kmobiletoolsMainPart::newDevice()
 */
void kmobiletoolsMainPart::configSlot(const QString &command)
{
    kDebug() << "kmobiletoolsMainPart::configSlot(" << command << ")\n";
    if(command == "newDevWiz")
    {
/*        DeviceManager* devman=new DeviceManager(m_widget);
        devman->show();
        connect(devman, SIGNAL(devicesAdded(const QStringList& ) ), this, SLOT(addDevices(const QStringList& )) );
        connect(devman, SIGNAL(devicesRemoved(const QStringList& )), this, SLOT(delDevices(const QStringList& )) );
        connect(devman, SIGNAL(loadDevice(const QString& )), this, SLOT(loadDevicePart(const QString&) ) );
        connect(devman, SIGNAL(unloadDevice(const QString& )), this, SLOT(deleteDevicePart(const QString&) ) );
        connect(this, SIGNAL(devicesUpdated() ), devman, SLOT(updateView() ) );
        devman*/ deviceManager()->slotNewDevice();
        return;
    }
    if(command=="configDevices")
    {
        deviceManager();
        return;
    }
    if(command.contains( "configure:") )
    {
        kDebug() << "trying to configure device " << command.section( ':',1,1) << endl;
        deviceManager()->showDeviceConfigDialog( command.section( ':',1,1) );
    }
}

void kmobiletoolsMainPart::addDevice(const QString &newDevice)
{
  loadDevicePart(newDevice, false);
  updateStatus();
}

void kmobiletoolsMainPart::delDevice(const QString &delDevice)
{
  deleteDevicePart(delDevice);
  updateStatus();
}



/*!
    \fn kmobiletoolsMainPart::deleteDevicePart(const QString &deviceName)
 */
void kmobiletoolsMainPart::deleteDevicePart(const QString &deviceName)
{
    int found=l_devicesList.find( deviceName );
    if( found==-1) return;
    goHome();
    l_devicesList.dump();
    m_widget->removeWidget( l_devicesList.at(found)->widget() );
    KMobileTools::Engine *t_engine=KMobileTools::EnginesList::instance()->find( deviceName );
    if(t_engine)
    {
        kDebug() << "****** removing engine " << t_engine->objectName() << endl;
        t_engine->queryClose();
        delete t_engine;
    }
//     l_devicesList.at(found)->engine()->queryClose();
    Q3ListViewItemIterator it( p_listview );
    while ( it.current() ) {
        kDebug() << KMobileTools::DevicesConfig::deviceGroup(it.current()->text(0)) << "==" << deviceName << endl;
        if ( KMobileTools::DevicesConfig::deviceGroup(it.current()->text(0))==deviceName )
        {
            delete it.current();
            break;
        }
        ++it;
    }
//     delete l_devicesList.take(found);
    DEVCFG(deviceName)->setLoaded(false);

    emit devicesUpdated();
    emit deviceChanged(deviceName);
}


/*!
    \fn kmobiletoolsMainPart::listviewClicked(QListViewItem *)
 */
void kmobiletoolsMainPart::listviewClicked(Q3ListViewItem *i)
{
    if(!i) return;
    DeviceListViewItem *item;
    kDebug() << "kmobiletoolsMainPart::listviewClicked(); i->depth()=" << i->depth() << endl;
    if(i->depth() )
        item=static_cast<DeviceListViewItem*>(i->parent());
    else item=static_cast<DeviceListViewItem*>(i);
    switchPart( item->deviceName() );
    l_devicesList.current()->clicked(i);
}


/*!
    \fn kmobiletoolsMainPart::activePartChanged(KParts::Part *newPart)
 */
// void kmobiletoolsMainPart::partChanged(KParts::Part *newPart)
// {
//     if(QString("DeviceHome") == newPart->className() )
//     {
//         emit setStatusBarText( "" );
// //         m_statusBarExt->setStatusBar (( (DeviceHome* ) (newPart) )->statusbar() );
//     } else emit setStatusBarText( "KMobileTools" );
// }


/*!
    \fn kmobiletoolsMainPart::slotQuit()
 */
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
    if( l_devicesList.find( deviceName ) == -1 ) return false;
    return true;
}



/*!
    \fn kmobiletoolsMainPart::deviceDisconnected()
 */
void kmobiletoolsMainPart::deviceDisconnected()
{
    /// @todo implement me
    updateStatus();
}


/*!
    \fn kmobiletoolsMainPart::deviceConnected()
 */
void kmobiletoolsMainPart::deviceConnected()
{
    /// @todo implement me
    updateStatus();
//     p_sysTray->setActive(); // @TODO fix this
}
void kmobiletoolsMainPart::guiActivateEvent( KParts::GUIActivateEvent * )
{
//     if (! partManager->activePart() ) return;
//     QApplication::postEvent(partManager->activePart(),new KParts::GUIActivateEvent( event->activated() ) );
//     KParts::ReadOnlyPart::guiActivateEvent(event);
//     if(QString("DeviceHome") == partManager->activePart()->className() )
//         emit setStatusBarText( "" );
//     else emit setStatusBarText( "KMobileTools" );
}



/*!
    \fn kmobiletoolsMainPart::widgetStackItemChanged(int item)
 */
void kmobiletoolsMainPart::widgetStackItemChanged(int item)
{
    // Find the current visible item, and remove the statusbar if it's not the HomePagePart
    if( m_widget->visibleWidget() != p_homepage->view() )
    {
        DeviceHome *oldPart=l_devicesList.at( l_devicesList.find(m_widget->visibleWidget()) );
        oldPart->clearStatusBar();
        unplugActionList("kmobiletools_devicepart.rc");
    }
    if(!item) return; // exit if destination widget is the HomePagePart
    DeviceHome *newPart=l_devicesList.at(l_devicesList.find( m_widget->widget( item ) ));
    newPart->setupStatusBar();
    plugActionList("kmobiletools_devicepart.rc", newPart->actionList() );
}

#include "kmobiletools_mainpart.moc"


/*!
    \fn kmobiletoolsMainPart::newSMS()
 */
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
    QString engine=KInputDialog::getItem( i18nc("Select engine for new sms dialog", "Select engine"), i18n("<qt>You have loaded multiple mobile phones.<br>Choose the one for the new sms.</qt>"),
    KMobileTools::EnginesList::instance()->namesList(true), 0, false, &ok, m_widget);
    if(!ok) return;
    engine=KMobileTools::EnginesList::instance()->find( engine, true)->objectName();
//     DeviceIFace_stub( "kmobiletools", engine.latin1() ).slotNewSMS() ; @TODO port this
}


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
    KMessageBox::information( widget(), i18n("<qt>KMobileTools has found an old or invalid configuration file.<br>To work correctly, it needs to delete your configuration files.<br>Your old files will be saved in <b>%1</b></qt>", archiveName ) );
    KTar arch(archiveName);
    if(!arch.open( QIODevice::WriteOnly))
    {
        KMessageBox::error( widget(), i18n("<qt>KMobileTools could not archive your config files.<br>Please remove them manually.</qt>") );
        return true;
    }
    for(QStringList::Iterator it=entries.begin(); it!=entries.end(); ++it)
    {
        arch.addLocalFile( cfgdir.path() + QDir::separator() + (*it), (*it));
        QFile::remove( cfgdir.path() + QDir::separator() + (*it) );
        kDebug() << "Entry ::" << cfgdir.path() + QDir::separator() + (*it) << " archived and removed." << endl;
    }
    arch.close();
    KMessageBox::information(widget(), i18n("<qt>Your old configuration files were saved in <b>%1</b>.<br>KMobileTools will now close. You can restart it.</qt>", archiveName));
    KMobileTools::MainConfig::self()->readConfig();
    KMobileTools::MainConfig::self()->setConfigversion(CURCFGVER);
    KMobileTools::MainConfig::self()->writeConfig();
    QTimer::singleShot(300, this, SLOT(slotQuit()) );
    return false;
}


/*!
    \fn kmobiletoolsMainPart::loadedEngines()
 */
QStringList kmobiletoolsMainPart::loadedEngines(bool friendly)
{
    return KMobileTools::EnginesList::instance()->namesList( friendly );
}



/*!
    \fn kmobiletoolsMainPart::phonebookUpdated()
 */
void kmobiletoolsMainPart::phonebookUpdated()
{
    // Some engine got the phonebook updated, updating then all device parts phonebooks.
    DeviceHome *curPart;
    Q3PtrListIterator<DeviceHome> it(l_devicesList);
    while( (curPart=it.current() ) )
    {
        ++it;
        kDebug() << "Updating device part " << curPart->objectName() << endl;
        curPart->updateAllContacts();
    }
}


/*!
    \fn kmobiletoolsMainPart::slotConfigNotify()
 */
void kmobiletoolsMainPart::slotConfigNotify()
{
    KNotifyConfigWidget::configure(m_widget, 0);
}

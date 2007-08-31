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
#include "devicemanager.h"

#include <KDE/KLocale>
#include <KDE/KListWidget>
#include <KDE/KPushButton>
#include <KDE/KStandardGuiItem>
#include <KDE/KIcon>
#include <KDE/KMessageBox>

#include <QtGui/QGridLayout>
#include <QtCore/QTimer>

#include <libkmobiletools/deviceloader.h>
#include <libkmobiletools/kmobiletools_cfg.h>
#include <libkmobiletools/devicesconfig.h>

#include "deviceconfigdialog.h"
#include "newdevicewizard/devicewizard.h"

#define COL_DEVNAME 3

DeviceManager::DeviceManager( QWidget *parent )
  : KDialog( parent )
{
    setupGUI();

    setCaption( i18n( "Device manager" ) );
    setButtons( KDialog::Close );
    setInitialSize( QSize( 450,300 ) );
    setModal( false );

    QTimer::singleShot( 0, this, SLOT(populateDeviceList()) );
    QTimer::singleShot( 0, this, SLOT(setupDialogs()) );

    connect( KMobileTools::DeviceLoader::instance(), SIGNAL(deviceLoaded(const QString&)),
             this, SLOT(addDeviceItem(const QString&)) );
    connect( KMobileTools::DeviceLoader::instance(), SIGNAL(deviceUnloaded(const QString&)),
             this, SLOT(removeDeviceItem(const QString&)) );
}


DeviceManager::~DeviceManager()
{
}

void DeviceManager::setupGUI() {
    // setup widgets
    m_deviceList = new KListWidget( this );
    m_deviceList->setIconSize( QSize( K3Icon::SizeLarge, K3Icon::SizeLarge ) );

    m_addDevice = new KPushButton( this );
    m_addDevice->setGuiItem( KStandardGuiItem::Add );

    m_removeDevice = new KPushButton( this );
    m_removeDevice->setGuiItem( KStandardGuiItem::Remove );
    m_removeDevice->setEnabled( false );

    m_deviceProperties = new KPushButton( this );
    m_deviceProperties->setGuiItem( KStandardGuiItem::Properties );
    m_deviceProperties->setEnabled( false );

    // setup signal-slot connections
    connect( m_deviceList, SIGNAL(itemSelectionChanged()), this, SLOT(checkEnableButtons()) );
    connect( m_deviceList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(deviceProperties()) );

    connect( m_addDevice, SIGNAL(clicked()), this, SLOT(addDevice()) );
    connect( m_removeDevice, SIGNAL(clicked()), this, SLOT(removeDevice()) );
    connect( m_deviceProperties, SIGNAL(clicked()), this, SLOT(deviceProperties()) );

    // setup layout
    QGridLayout* layout = new QGridLayout();
    layout->addWidget( m_deviceList, 0, 0, 4, 1 );
    layout->addWidget( m_addDevice, 0, 1 );
    layout->addWidget( m_removeDevice, 1, 1 );
    layout->addWidget( m_deviceProperties, 2, 1 );
    layout->addItem( new QSpacerItem( 1, 1 ), 3, 1 );
    layout->setRowStretch( 3, 1 );

    QWidget* dummyWidget = new QWidget( this );
    dummyWidget->setLayout( layout );

    setMainWidget( dummyWidget );
}

void DeviceManager::checkEnableButtons() {
    if( m_deviceList->currentItem() ) {
        m_removeDevice->setEnabled( true );
        m_deviceProperties->setEnabled( true );
    } else {
        m_removeDevice->setEnabled( false );
        m_deviceProperties->setEnabled( false );
    }
}

void DeviceManager::populateDeviceList() {
    QStringList deviceList = KMobileTools::DeviceLoader::instance()->loadedDevices();
    for( int i=0; i<deviceList.size(); i++ )
        addDeviceItem( deviceList.at( i ) );
}

void DeviceManager::addDeviceItem( const QString& deviceName ) {
    QListWidgetItem* item = new QListWidgetItem( deviceName, m_deviceList );
    item->setIcon( KIcon( "phone" ) );
}

void DeviceManager::removeDeviceItem( const QString& deviceName ) {
    for( int i=0; i<m_deviceList->count(); i++ ) {
        QListWidgetItem* item = m_deviceList->item( i );
        if( item->data( Qt::DisplayRole ).toString() == deviceName ) {
            delete item;
            break;
        }
    }
}


void DeviceManager::addDevice() {
}


void DeviceManager::removeDevice() {
    QString deviceName = m_deviceList->currentItem()->data( Qt::DisplayRole ).toString();

    int result = KMessageBox::warningYesNo( this, i18n( "Do you really want to remove the device \"%1\"?",
                                                  deviceName ),
                                            i18n( "Removing device" ) );
    if( result == KMessageBox::Yes ) {
        QStringList devices = KMobileTools::MainConfig::devicelist();
        QString internalDeviceName = KMobileTools::DevicesConfig::deviceGroup( deviceName );

        if( !devices.contains( internalDeviceName ) ) {
            /// @TODO add call to error handler here
            return;
        }

        // take the device from the list
        devices.removeAll( internalDeviceName );

        // FIXME: deletePrefs does not work correctly
        KMobileTools::DevicesConfig::deletePrefs( internalDeviceName );

        KMobileTools::MainConfig::setDevicelist( devices );
        KMobileTools::MainConfig::self()->writeConfig();

        // unload the device
        KMobileTools::DeviceLoader::instance()->unloadDevice( deviceName );
    }
}

void DeviceManager::deviceProperties() {

}


void DeviceManager::slotRemoveDevice() /// Remove device
{
/*
    if (ui.deviceListView->selectedItem() == NULL ) return;
    QStringList sl_devices = KMobileTools::MainConfig::devicelist();
    QString deviceName = KMobileTools::DevicesConfig::deviceGroup( ui.deviceListView->selectedItem()->text(0) );

    // If the device is not in the list, well, just skip it
    if ( ! sl_devices.contains(deviceName) ) {
      kDebug() <<"Asked to remove the non-present device" << deviceName;
      return;
    }

    // Remove the entry from the list of devices
    int index;
    QStringList::iterator end = sl_devices.end();
    while ( (index = sl_devices.indexOf(deviceName)) != -1 )
      sl_devices.removeAt(index);

    KMobileTools::DevicesConfig::deletePrefs( deviceName );

    KMobileTools::MainConfig::setDevicelist( sl_devices );
    KMobileTools::MainConfig::self()->writeConfig();

    emit deviceRemoved(deviceName);

    updateView();
*/
}

void DeviceManager::slotDeviceProperties() /// Modify device
{
/*
    if (ui.deviceListView->selectedItem() == NULL ) return;
    showDeviceConfigDialog( KMobileTools::DevicesConfig::deviceGroup(ui.deviceListView->selectedItem()->text(0) ) );
*/
}

void DeviceManager::slotNewDevice()
{
/*
    /// Add device button
    QString deviceName=KMobileTools::DevicesConfig::firstFreeGroup();
    if( ! showDeviceConfigDialog(deviceName, true) )
      return;

    QStringList sl_devices = KMobileTools::MainConfig::devicelist();
    sl_devices += deviceName;
    KMobileTools::MainConfig::setDevicelist( sl_devices );
    KMobileTools::MainConfig::self()->writeConfig();
    deviceChanged(deviceName);

    updateView();

    emit deviceAdded(deviceName);
*/
}


#include "devicemanager.moc"

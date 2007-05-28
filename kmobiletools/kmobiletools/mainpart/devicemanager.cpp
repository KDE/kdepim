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
#include "devicemanager.h"

#include <k3listview.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kconfigdialog.h>
#include <kpushbutton.h>

#include <libkmobiletools/kmobiletools_cfg.h>
#include <libkmobiletools/devicesconfig.h>

#include "deviceconfigdialog.h"
#include "newdevicewizard/newdevicewizard.h"

#define COL_DEVNAME 3

DeviceManager::DeviceManager(QWidget *parent, const char *name)
  : KDialog(parent)
{
    setCaption(i18n("Device Manager"));
    setButtons(Close);
    ui.setupUi(mainWidget());
//     iconview=new K3IconView(this,"deviceiconview");
    setInitialSize( QSize(450,300) );
    setModal(false);
/*    setButtonGuiItem(User3, KGuiItem( i18n("Add"), "bookmark_add") );
    setButtonGuiItem(User2, KGuiItem( i18n("Edit"), "editclear") );
    setButtonGuiItem(User1, KGuiItem( i18n("Remove"), "edittrash") );*/
    connect(ui.addButton, SIGNAL(clicked()), this, SLOT(slotNewDevice() ) );
    connect(ui.cfgButton, SIGNAL(clicked()), this, SLOT(slotDeviceProperties() ) );
    connect(ui.remButton, SIGNAL(clicked()), this, SLOT(slotRemoveDevice() ) );
    connect(ui.deviceListView, SIGNAL(doubleClicked ( Q3ListViewItem *, const QPoint &, int ) ),
            this, SLOT(doubleClickedItem(Q3ListViewItem* ) ) );
    connect(ui.deviceListView, SIGNAL(itemRenamed ( Q3ListViewItem *, int, const QString &) ),
            this, SLOT(slotItemRenamed ( Q3ListViewItem *, int, const QString &) ) );
    connect(ui.deviceListView, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(ui.actButton, SIGNAL(toggled(bool) ), this, SLOT(deviceToggled(bool ) ) );

    updateView();
}


DeviceManager::~DeviceManager()
{
}


#include "devicemanager.moc"


void DeviceManager::updateView()
{
    selectionChanged();
    ui.deviceListView->clear();
    QStringList sl_devices=KMobileTools::MainConfig::devicelist();
    K3ListViewItem* cur_item;
    for ( QStringList::Iterator it = sl_devices.begin(); it != sl_devices.end(); ++it ) {
        cur_item= new K3ListViewItem( /*
                iconview, DEVCFG(*it)->devicename(), 
                KIconLoader::global()->loadIcon("kmobiletools",K3Icon::NoGroup, K3Icon::SizeMedium) */
                ui.deviceListView,
                DEVCFG(*it)->devicename(),
                KMobileTools::DevicesConfig::engineTypeName(DEVCFG(*it)->engine() ),
                (DEVCFG(*it)->loaded() ? i18n("Yes") : i18n("No") ),
                                *it );
        cur_item->setPixmap(0, KMobileTools::DevicesConfig::deviceTypeIcon(*it,K3Icon::NoGroup, K3Icon::SizeSmall) );
        cur_item->setRenameEnabled(0, true);
    }
}

void DeviceManager::slotRemoveDevice() /// Remove device
{
    if (ui.deviceListView->selectedItem() == NULL ) return;
    QStringList sl_devices = KMobileTools::MainConfig::devicelist();
    QString deviceName = KMobileTools::DevicesConfig::deviceGroup( ui.deviceListView->selectedItem()->text(0) );

    /* If the device is not in the list, well, just skip it */
    if ( ! sl_devices.contains(deviceName) ) {
      kDebug() << "Asked to remove the non-present device " << deviceName << endl;
      return;
    }

    /* Remove the entry from the list of devices */
    int index;
    QStringList::iterator end = sl_devices.end();
    while ( (index = sl_devices.indexOf(deviceName)) != -1 )
      sl_devices.removeAt(index);

    KMobileTools::DevicesConfig::deletePrefs( deviceName );

    KMobileTools::MainConfig::setDevicelist( sl_devices );
    KMobileTools::MainConfig::self()->writeConfig();

    emit deviceRemoved(deviceName);

    updateView();
}

void DeviceManager::slotDeviceProperties() /// Modify device
{
    if (ui.deviceListView->selectedItem() == NULL ) return;
    showDeviceConfigDialog( KMobileTools::DevicesConfig::deviceGroup(ui.deviceListView->selectedItem()->text(0) ) );
}

void DeviceManager::slotNewDevice()
{
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
}

int DeviceManager::showDeviceConfigDialog(const QString &deviceName, bool newdevice)
{
    if(newdevice)
    {
        newDeviceWizard *newDeviceConfigDialog=new newDeviceWizard(deviceName, this);
        int ret=newDeviceConfigDialog->exec();
        return ret;
    } else
    {
        deviceConfigDialog *cfg_device=new deviceConfigDialog(this, deviceName.toLatin1(), DEVCFG(deviceName));
        int ret=cfg_device->exec();
        deviceChanged(deviceName);
        return ret;
    }
}


void DeviceManager::doubleClickedItem(Q3ListViewItem *item)
{
    if ( item == NULL ) return;
    showDeviceConfigDialog( KMobileTools::DevicesConfig::deviceGroup(item->text(0) ) );
}

void DeviceManager::slotItemRenamed ( Q3ListViewItem * item, int col, const QString & text )
{
    if(col!=0) return;
    KMobileTools::DevicesConfig *wconfig=KMobileTools::DevicesConfig::prefs( item->text(COL_DEVNAME) );
    wconfig->setDevicename( text );
    wconfig->writeConfig();
}


/*!
    \fn DeviceManager::selectionChanged ( )
 */
void DeviceManager::selectionChanged ()
{
    Q3ListViewItem *item=ui.deviceListView->selectedItem ();
    if(item==NULL)
    {
        ui.cfgButton->setEnabled(false);
        ui.remButton->setEnabled(false);
        ui.actButton->setEnabled(false);
        return;
    }
    ui.cfgButton->setEnabled(true);
    ui.remButton->setEnabled(true);
    ui.actButton->setEnabled(true);
    ui.actButton->setChecked( DEVCFG(item->text(COL_DEVNAME))->loaded() );
}


/*!
    \fn DeviceManager::deviceToggled(bool)
 */
void DeviceManager::deviceToggled(bool b)
{
    Q3ListViewItem *item=ui.deviceListView->selectedItem ();
    if(item==NULL) return;
    if ( DEVCFG(item->text(COL_DEVNAME))->loaded() != b)
    {
        if(b)
            emit  loadDevice( item->text(COL_DEVNAME) );
        else
            emit unloadDevice( item->text(COL_DEVNAME) );
    }
}


/*!
    \fn DeviceManager::deviceChanged(const QString &)
 */
void DeviceManager::deviceChanged(const QString &deviceName)
{
    QString deviceText=DEVCFG(deviceName)->devicename();
    // Try to find our device in the listview
    Q3ListViewItemIterator it( ui.deviceListView );
    while ( it.current() ) {
        if(it.current()->text(COL_DEVNAME) == deviceName)
        {
            Q3ListViewItem *item = it.current();
            item->setText(0, deviceText);
            item->setText(1, KMobileTools::DevicesConfig::engineTypeName(DEVCFG(deviceName)->engine()) );
            item->setText(2, (DEVCFG(deviceName)->loaded() ? i18n("Yes") : i18n("No") ) );
            item->setPixmap(0, KMobileTools::DevicesConfig::deviceTypeIcon(deviceName,K3Icon::NoGroup, K3Icon::SizeSmall) );
        }
        ++it;

    }
}

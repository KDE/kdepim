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
#include "at_connectionspage.h"

#include <QListWidgetItem>
#include <klocalizedstring.h>
#include <kdebug.h>
#include <keditlistbox.h>

#include "at_engine.h"

#include "bluetooth.h"
#include "userconnection.h"

AT_ConnectionsPage::AT_ConnectionsPage(QWidget *parent)
    : ConnectionsPage(parent), blueWidget(NULL), usrConnWidget(NULL)
{
    QListWidgetItem *tempitem;
    tempitem=new QListWidgetItem(i18nc("Connections list", "USB Cable"), connections(), AT_Engine::ConnectionUSB + QListWidgetItem::UserType);
    tempitem=new QListWidgetItem(i18nc("Connections list", "Serial Cable"), connections(), AT_Engine::ConnectionSerial + QListWidgetItem::UserType);
    tempitem=new QListWidgetItem(i18nc("Connections list", "Bluetooth"), connections(), AT_Engine::ConnectionBluetooth + QListWidgetItem::UserType);
    tempitem=new QListWidgetItem(i18nc("Connections list", "InfraRed"), connections(), AT_Engine::ConnectionIrDA + QListWidgetItem::UserType);
    tempitem=new QListWidgetItem(i18nc("Connections list", "User Defined"), connections(), AT_Engine::ConnectionUser + QListWidgetItem::UserType);
}

void AT_ConnectionsPage::initializePage() {
    registerField("connections*", this, "connections", SIGNAL(selectedConnectionsChanged()) );
    connect(connections(), SIGNAL(itemSelectionChanged()), this, SLOT(selChanged()) );
    connect(this, SIGNAL(selectedConnectionsChanged()), this, SIGNAL(completeChanged() ) );
}

int AT_ConnectionsPage::selectedConnections() {
    QList<QListWidgetItem *> sel=connections()->selectedItems();
    int ret=0;
    for(int i=0; i<sel.count(); i++)
        ret|=(sel[i]->type()-QListWidgetItem::UserType);
    kDebug() << "selectedConnections now is " << ret << endl;
    i_connections=ret;
    return ret;
}

void AT_ConnectionsPage::setSelectedConnections(int) {
    // @TODO implement me
}


void AT_ConnectionsPage::selChanged() {
    int sel=selectedConnections();
    // Bluetooth
    if( (sel & AT_Engine::ConnectionBluetooth) && ! blueWidget ) {
        blueWidget=new BluetoothWidget();
        connect(blueWidget, SIGNAL(completeChanged() ), this, SIGNAL(completeChanged() ) );
        details()->addTab(blueWidget, i18nc("Bluetooth Connection details tab title in the wizard", "Bluetooth") );
    } else if (! (sel & AT_Engine::ConnectionBluetooth) ) {
        if (blueWidget) details()->removeTab(details()->indexOf(blueWidget));
        delete blueWidget;
        blueWidget=NULL;
    }
    if( (sel & AT_Engine::ConnectionUser) && ! usrConnWidget ) {
        usrConnWidget=new UserConnectionWidget(this);
        connect(usrConnWidget, SIGNAL(completeChanged() ), this, SIGNAL(completeChanged() ) );
        details()->addTab(usrConnWidget, i18nc("User Defined Connection details tab title in the wizard", "User Defined") );
    } else if (! (sel & AT_Engine::ConnectionUser) ) {
        if (usrConnWidget) {
            details()->removeTab(details()->indexOf(usrConnWidget));
            disconnect(usrConnWidget);
        }
        delete usrConnWidget;
        usrConnWidget=NULL;
    }
    emit selectedConnectionsChanged();
}

bool AT_ConnectionsPage::validatePage() {
    ATDevicesConfig *cfg=(ATDevicesConfig*) DEVCFG(wizard()->objectName() );
    kDebug() << "Saving settings to " << wizard()->objectName() << endl;
    cfg->setAt_connections(field("connections").toInt());
    if(usrConnWidget) cfg->setAt_userdevices( usrConnWidget->devicePaths()->items() );
    kDebug() << "Connections: " << field("connections").toInt() << endl;
    cfg->writeConfig();
    return true;
}

bool AT_ConnectionsPage::isComplete() const {
    kDebug() << "AT_ConnectionsPage::isComplete()" << endl;
//     if(!QWizardPage::isComplete() ) return false; it seems to not work...
    if(i_connections==0) return false;
    if(blueWidget && (! blueWidget->isComplete()) ) return false;
    if(usrConnWidget && (! usrConnWidget->isComplete() ) ) return false;
    kDebug() << "true" << endl;
    return true;
}


#include "at_connectionspage.moc"


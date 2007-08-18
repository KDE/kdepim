/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

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

#include "servicemodel.h"

// KMobileTools includes
#include <libkmobiletools/deviceloader.h>
#include <libkmobiletools/serviceloader.h>
#include <libkmobiletools/ifaces/guiservice.h>
#include <libkmobiletools/enginexp.h>
#include "deviceitem.h"
#include "treeitem.h"
#include "serviceitem.h"

// Qt includes
#include <QModelIndex>

// KDE includes
#include <KLocale>
#include <KPluginInfo>
#include <KIcon>

ServiceModel::ServiceModel( QObject* parent )
 : QAbstractItemModel( parent )
{
    // header item
    m_rootItem = new TreeItem( i18n( "Devices" ) );

    // let the model receive notifications upon device loading
    connect( KMobileTools::DeviceLoader::instance(), SIGNAL( deviceLoaded(const QString&) ),
             this, SLOT( deviceLoaded(const QString&) ) );

    // let the model receive notifications upon device unloading
    connect( KMobileTools::DeviceLoader::instance(), SIGNAL( deviceUnloaded(const QString&) ),
             this, SLOT( deviceUnloaded(const QString&) ) );

    // let the model receive notifications upon service loading
    connect( KMobileTools::ServiceLoader::instance(),
             SIGNAL( serviceLoaded(const QString&, KMobileTools::CoreService*) ),
             this,
             SLOT( serviceLoaded(const QString&, KMobileTools::CoreService*) ) );

    // let the model receive notifications upon device unloading
    connect( KMobileTools::ServiceLoader::instance(),
             SIGNAL( serviceUnloaded(const QString&, KMobileTools::CoreService*) ),
             this,
             SLOT( serviceUnloaded(const QString&, KMobileTools::CoreService*) ) );
}


ServiceModel::~ServiceModel() {
    delete m_rootItem;
}

QModelIndex ServiceModel::index( int row, int column, const QModelIndex& parent ) const {
    if( !hasIndex( row, column, parent ) )
        return QModelIndex();

    TreeItem *parentItem;

    if( !parent.isValid() )
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeItem*>( parent.internalPointer() );

    TreeItem *childItem = parentItem->child( row );
    if( childItem )
        return createIndex( row, column, childItem );
    else
        return QModelIndex();
}

QModelIndex ServiceModel::parent( const QModelIndex& index ) const {
    if( !index.isValid() )
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>( index.internalPointer() );
    TreeItem *parentItem = childItem->parent();

    if( parentItem == m_rootItem )
        return QModelIndex();

    return createIndex( parentItem->row(), 0, parentItem );
}

Qt::ItemFlags ServiceModel::flags( const QModelIndex &index ) const {
    if( !index.isValid() )
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

}

QVariant ServiceModel::data( const QModelIndex& index, int role ) const {
    if( !index.isValid() )
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>( index.internalPointer() );

    if( role == Qt::DisplayRole )
        return item->data();
    else if( role == Qt::DecorationRole )
        return item->icon();
    else
        return QVariant();
}

QVariant ServiceModel::headerData( int section, Qt::Orientation orientation, int role ) const {
    Q_UNUSED(section)
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
        return m_rootItem->data();

    return QVariant();
}

int ServiceModel::columnCount( const QModelIndex& parent ) const {
    Q_UNUSED(parent)
    return 1;
}

int ServiceModel::rowCount( const QModelIndex& parent ) const {
    TreeItem *parentItem;
    if( parent.column() > 0 )
        return 0;

    if( !parent.isValid() )
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeItem*>( parent.internalPointer() );

    return parentItem->childCount();
}

void ServiceModel::deviceLoaded( const QString& deviceName ) {
    DeviceItem* deviceItem = new DeviceItem( deviceName, m_rootItem );

    // set icon for deviceItem
    KPluginInfo deviceInformation = KMobileTools::DeviceLoader::instance()->engineInformation( deviceName );
    deviceItem->setIcon( KIcon( deviceInformation.icon() ) );

    // append our item as last child of our root item
    int row = m_rootItem->childCount( TreeItem::AllChildren );
    // inform the views about the appending of an item
    beginInsertRows( QModelIndex(), row, row );

    m_rootItem->appendChild( deviceItem );

    endInsertRows();
}

void ServiceModel::deviceUnloaded( const QString& deviceName ) {
    // look at which row the device is located
    int row = 0;
    bool deviceNameFound = false;
    for( ; row<m_rootItem->childCount(); row++ ) {
        if( m_rootItem->child( row )->data() == deviceName ) {
            deviceNameFound = true;
            break;
        }
    }

    if( deviceNameFound ) {
        // get the QModelIndex for our device
        QModelIndex deviceIndex = index( row, 0 );

        // inform the views about the upcoming removal of our device
        beginRemoveRows( QModelIndex(), row, row );

        // finally remove it and free memory
        Q_CHECK_PTR( m_rootItem->child( row ) );
        delete m_rootItem->child( row );
        m_rootItem->removeChild( m_rootItem->child( row ) );

        endRemoveRows();
    }
}

DeviceItem* ServiceModel::deviceItemFromName( const QString& deviceName ) const {
    // look at which row the device is located
    int row = 0;
    bool deviceNameFound = false;
    for( ; row<m_rootItem->childCount(); row++ ) {
        if( m_rootItem->child( row )->data() == deviceName ) {
            deviceNameFound = true;
            break;
        }
    }

    if( deviceNameFound ) {
        DeviceItem* deviceItem = qobject_cast<DeviceItem*>( m_rootItem->child( row ) );
        if( deviceItem )
            return deviceItem;
    }
    return 0;
}

void ServiceModel::serviceLoaded( const QString& deviceName, KMobileTools::CoreService* service ) {
    DeviceItem* deviceItem = deviceItemFromName( deviceName );
    if( deviceItem ) {
        ServiceItem* serviceItem = new ServiceItem( service->name(), deviceItem );
        serviceItem->setService( service );

        // now check if it's a gui service.. only gui services are worth being displayed ;-)
        KMobileTools::Ifaces::GuiService* guiService =
                    qobject_cast<KMobileTools::Ifaces::GuiService*>( service );

        if( guiService )
            serviceItem->setIcon( guiService->icon() );
        else
            serviceItem->setVisible( false );

        deviceItem->appendChild( serviceItem );
    }
}


void ServiceModel::serviceUnloaded( const QString& deviceName, KMobileTools::CoreService* service ) {
    DeviceItem* deviceItem = deviceItemFromName( deviceName );
    if( !deviceItem )
        return ;

    // look at which row the device is located
    int deviceRow = 0;
    bool deviceNameFound = false;
    for( ; deviceRow<m_rootItem->childCount(); deviceRow++ ) {
        if( m_rootItem->child( deviceRow )->data() == deviceName ) {
            deviceNameFound = true;
            break;
        }
    }

    if( !deviceNameFound )
        return;

    // get the QModelIndex for our device
    QModelIndex deviceIndex = index( deviceRow, 0 );

    // look at which row the service is located
    int serviceRow = 0;
    bool serviceFound = false;
    for( ; serviceRow<deviceItem->childCount(); serviceRow++ ) {
        ServiceItem* serviceItem = qobject_cast<ServiceItem*>( deviceItem->child( serviceRow ) );

        if( serviceItem->service() == service ) {
            serviceFound = true;
            break;
        }
    }

    if( serviceFound ) {
        // get the QModelIndex for our service item
        QModelIndex serviceIndex = index( serviceRow, 0, deviceIndex );

        // inform the views about the upcoming removal of our service item
        beginRemoveRows( deviceIndex, serviceRow, serviceRow );

        // finally remove it and free memory
        Q_CHECK_PTR( deviceItem->child( serviceRow ) );
        delete deviceItem->child( serviceRow );
        deviceItem->removeChild( deviceItem->child( serviceRow ) );

        endRemoveRows();
    }
}

QList<DeviceItem*> ServiceModel::deviceItems() const {
    QList<TreeItem*> treeItems;
    for( int i=0; i<m_rootItem->childCount(); i++ )
        treeItems.append( m_rootItem->child( i ) );

    // cast tree items to device items
    QList<DeviceItem*> deviceItems;
    for( int i=0; i<treeItems.size(); i++ ) {
        DeviceItem* deviceItem = dynamic_cast<DeviceItem*>( treeItems.at( i ) );
        if( deviceItem )
            deviceItems.append( deviceItem );
    }

    return deviceItems;
}

#include "servicemodel.moc"

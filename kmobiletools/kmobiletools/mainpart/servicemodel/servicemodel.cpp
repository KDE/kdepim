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
#include <libkmobiletools/enginexp.h>
#include "deviceitem.h"
#include "treeitem.h"

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
#include "servicemodel.moc"

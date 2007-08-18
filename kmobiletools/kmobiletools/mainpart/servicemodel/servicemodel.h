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

#ifndef SERVICEMODEL_H
#define SERVICEMODEL_H

#include <QtCore/QAbstractItemModel>
#include <libkmobiletools/coreservice.h>

class DeviceItem;
class TreeItem;
/**
 * This class provides a data model for KMobileTools' core services
 *
 * @author Matthias Lechner <matthias@lmme.de>
 */
class ServiceModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    /**
     * Constructs a new service model that represents a tree structure containing device nodes
     * and services nodes below
     *
     * @param parent the parent object
     */
    ServiceModel( QObject* parent = 0 );
    ~ServiceModel();

    /**
     * Returns the device item associated with the given @p deviceName
     *
     * @param deivceName the device name
     *
     * @return the device item if available, else null
     */
    DeviceItem* deviceItemFromName( const QString& deviceName ) const;

    QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex& index ) const;

    Qt::ItemFlags flags( const QModelIndex& index ) const;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    int columnCount( const QModelIndex& parent = QModelIndex() ) const;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;

    QList<DeviceItem*> deviceItems() const;

public Q_SLOTS:
    /**
     * This slot adds a new DeviceItem node and should be called whenever a new device was loaded
     *
     * @param deviceName the name of the added device
     */
    void deviceLoaded( const QString& deviceName );

    /**
     * This slot removes a DeviceItem node and should be called whenever a device was removed
     *
     * @param deviceName the name of the removed device
     */
    void deviceUnloaded( const QString& deviceName );

    /**
     * This slot adds a new ServiceItem node and should be called whenever a new service was loaded
     *
     * @param deviceName the device associated with the service
     * @param service the service that was added
     */
    void serviceLoaded( const QString& deviceName, KMobileTools::CoreService* service );

    /**
     * This slot removes a ServiceItem node and should be called whenever a service was removed
     *
     * @param deviceName the device associated with the service
     * @param service the service that was removed
     */
    void serviceUnloaded( const QString& deviceName, KMobileTools::CoreService* service );

private:
    TreeItem* m_rootItem;
};

#endif

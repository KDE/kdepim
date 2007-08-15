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

    QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex& index ) const;

    Qt::ItemFlags flags( const QModelIndex& index ) const;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    int columnCount( const QModelIndex& parent = QModelIndex() ) const;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;

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

private:
    TreeItem* m_rootItem;
};

#endif

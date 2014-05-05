/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef LOCATIONMODEL_H
#define LOCATIONMODEL_H

#include <KABC/kabc/Address>

#include <QtCore/QAbstractTableModel>

class LocationModel : public QAbstractTableModel
{
  public:
    explicit LocationModel( QObject *parent = 0 );
    ~LocationModel();

    void setLocations( const KABC::Address::List &locations );
    KABC::Address::List locations() const;

    /**
     * @reimplemented
     */
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );
    Qt::ItemFlags flags( const QModelIndex &index ) const;

    bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() );
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );

  private:
    KABC::Address::List mLocations;
};

#endif /* LOCATIONMODEL_H */

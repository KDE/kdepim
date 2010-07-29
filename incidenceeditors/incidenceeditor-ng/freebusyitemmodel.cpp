/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include "freebusyitemmodel.h"

#include <KCal/Attendee>

using namespace IncidenceEditorsNG;

FreeBusyItemModel::FreeBusyItemModel( QObject* parent ): QAbstractListModel( parent )
{

}

FreeBusyItemModel::~FreeBusyItemModel()
{

}


QVariant FreeBusyItemModel::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() )
      return QVariant();

    int row = index.row();

    if( row > mFreeBusyItems.size()-1 )
      return QVariant();

    switch( role ) {
      case Qt::DisplayRole:
        return mFreeBusyItems.at( row )->attendee().fullName();
      case FreeBusyItemModel::AttendeeRole:
        return mFreeBusyItems.at( row )->attendee();
      case FreeBusyItemModel::AttendeeRole:
      default:
        return QVariant();
    }
}

int FreeBusyItemModel::rowCount( const QModelIndex& parent ) const
{
    return mFreeBusyItems.size();
}


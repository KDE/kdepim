/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#ifndef LISTPROXY_H
#define LISTPROXY_H

#include <QSortFilterProxyModel>

#include "mobileui_export.h"

/** Proxy model to provide roles for accessing Akonadi::Items properties from QML. */
class MOBILEUI_EXPORT ListProxy : public QSortFilterProxyModel
{
  Q_OBJECT
  Q_PROPERTY( int itemCount READ rowCount )

  public:
    explicit ListProxy( QObject* parent = 0 );

    /** Make sure that reimplementing classes implement data for their own needs */
    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const = 0;

    virtual void setSourceModel(QAbstractItemModel* sourceModel);

    Q_INVOKABLE qint64 itemId( int row ) const;
};

#endif

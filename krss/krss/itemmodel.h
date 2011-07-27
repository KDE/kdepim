/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KRSS_ITEMMODEL_H
#define KRSS_ITEMMODEL_H

#include "krss_export.h"
#include "item.h"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QMetaType>

#include <boost/shared_ptr.hpp>

class KJob;

namespace KRss {

class ItemListing;

class ItemModelPrivate;

class KRSS_EXPORT ItemModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    enum Column {
        ItemTitleColumn = 0,
        AuthorsColumn,
        DateColumn,
        FeedTitleColumn,
        ColumnCount
    };

    enum Role {
        ItemRole = Qt::UserRole,
        ItemStatusRole,
        SortRole,
        IsNewRole,
        IsUnreadRole,
        IsReadRole,
        IsDeletedRole, //PENDING(frank) transitional Akregator compat, review
        IsImportantRole, //PENDING(frank) transitional Akregator compat, review
        LinkRole  //PENDING(frank) transitional Akregator compat, review
    };

public:

    explicit ItemModel( const boost::shared_ptr<ItemListing>& itemCollection, QObject *parent = 0 );
    ~ItemModel();

    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

protected:

    Item itemForIndex( const QModelIndex &index ) const;

private:

    Q_DISABLE_COPY(ItemModel)
    friend class ::KRss::ItemModelPrivate;
    ItemModelPrivate * const d;
};

} // namespace KRss

Q_DECLARE_METATYPE(KRss::Item)

#endif // KRSS_ITEMMODEL_H

/*
    This file is part of KDE.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef KXFORMS_LISTMODEL_H
#define KXFORMS_LISTMODEL_H

#include "reference.h"

#include <QList>
#include <QDomElement>
#include <QAbstractTableModel>

namespace KXForms {

class ListModel : public QAbstractListModel
{
  public:
    ListModel( QObject *parent );
    ~ListModel();

    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex & index,
      int role = Qt::DisplayRole ) const;
    QVariant headerData ( int section, Qt::Orientation orientation,
      int role = Qt::DisplayRole ) const;
    bool removeRows( int row, int count,
      const QModelIndex &parent = QModelIndex() );

    void recalculateSegmentCounts();

    struct Item
    {
      QString label;
      Reference ref;
      QDomElement element;
    };

    struct visibleElement {
        QString label;
        Reference ref;
    };
    
    void addItem( const QString &label, const Reference &ref, QDomElement element );
    Item *item( const QModelIndex &index );

    int itemCount( const QString &itemClass );

    void setLabel( const QString & );
    QString label() const;

    void setVisibleElements( QList<visibleElement> headers ) { mVisibleElements = headers; }

    void clear();

    QModelIndex moveItem( int from, int to );

  private:
    QList<Item *> mItems;
    QString mLabel;
    QList<visibleElement> mVisibleElements;
};

}

#endif

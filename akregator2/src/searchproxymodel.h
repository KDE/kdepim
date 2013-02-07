/*
    This file is part of Akregator2.

    Copyright (C) 2013 Dan Vrátil <dvratil@redhat.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/


#ifndef SEARCHPROXYMODEL_H
#define SEARCHPROXYMODEL_H

#include <Akonadi/EntityMimeTypeFilterModel>


namespace Akregator2 {

/**
 * This proxy model reduces the whole source EntityTreeModel to a simple flat
 * model, containing only items that are children of a collection set by
 * setCollection().
 */
class SearchProxyModel : public Akonadi::EntityMimeTypeFilterModel
{
    Q_OBJECT

  public:
    explicit SearchProxyModel(QObject* parent = 0);
    virtual ~SearchProxyModel();

    void setCollection( const Akonadi::Collection &collection );
    virtual void setSourceModel(QAbstractItemModel* sourceModel);

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    virtual QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex& child ) const;

    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;

    virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;

  private:
    class Private;
    Private * const d;
};

} // namespace

#endif // SEARCHPROXYMODEL_H

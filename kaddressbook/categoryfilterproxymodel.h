/*
    Copyright (c) 2014 Jonathan Marten <jjm@keelhaul.me.uk>

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

#ifndef CATEGORYFILTERPROXYMODEL_H
#define CATEGORYFILTERPROXYMODEL_H

#include <qsortfilterproxymodel.h>

#include <AkonadiCore/tag.h>

class CategoryFilterProxyModelPrivate;


/**
 * @short A proxy model to filter contacts by categories (tags).
 *
 * @since 4.14
 * @author Jonathan Marten
 **/

class CategoryFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CategoryFilterProxyModel);

public:
    /**
     * Constructor.
     *
     * @param parent The parent object
     **/
    explicit CategoryFilterProxyModel(QObject *parent = 0);

    /**
     * Destructor.
     **/
    virtual ~CategoryFilterProxyModel();

public slots:
    /**
     * Set the categories to be accepted by the filter.
     *
     * @param idList A list of @c Akonadi::Tag::Id's of the categories
     * which are to be accepted by the filter.
     * @see CategorySelectModel::filterChanged
     **/
    void setFilterCategories(const QList<Akonadi::Tag::Id> &idList);

    /**
     * Enable or disable the filter.
     *
     * @param enable If @c true, enable the filter to accept only those categories
     * set by @c setFilterCategories().  If @false, disable the filter so that all
     * entries are accepted.
     *
     * The default state is that the filter is disabled.
     **/
    void setFilterEnabled(bool enable);

protected:
    /**
     * @reimp
     **/
    virtual bool filterAcceptsRow(int row, const QModelIndex &parent) const;

private:
    CategoryFilterProxyModelPrivate * const d_ptr;
};

#endif							// CATEGORYFILTERPROXYMODEL_H

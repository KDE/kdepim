/*
    This file is part of KJots.

    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

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

#ifndef KJOTSSORTPROXYMODEL_H
#define KJOTSSORTPROXYMODEL_H

#include <QSortFilterProxyModel>

#include <AkonadiCore/collection.h>

class KJotsSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit KJotsSortProxyModel(QObject *parent = 0);
    ~KJotsSortProxyModel();

    void sortChildrenAlphabetically(const QModelIndex &parent);
    void sortChildrenByCreationTime(const QModelIndex &parent);

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
    Akonadi::Collection::Id collectionId(const QModelIndex &parent) const;

private:
    QSet<Akonadi::Collection::Id> m_alphaSorted;
    QSet<Akonadi::Collection::Id> m_dateTimeSorted;
};

#endif

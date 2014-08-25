/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef TODOCHECKABLEPROXYMODEL_H
#define TODOCHECKABLEPROXYMODEL_H

#include <kcheckableproxymodel.h>

class TodoCheckableProxyModel : public KCheckableProxyModel
{
    Q_OBJECT
public:
    TodoCheckableProxyModel(QObject *parent = 0);

    virtual void setSourceModel(QAbstractItemModel *sourceModel);

private slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    void setChecked(const QItemSelection &selection, bool checked);
};

#endif

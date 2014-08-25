/*
    This file is part of Akonadi.

    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

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

#ifndef CATEGORIZEDENTITYMODEL_H
#define CATEGORIZEDENTITYMODEL_H

#include "mixedtreemodel.h"
#include <kcategorizedsortfilterproxymodel.h>

class CategorisedEntityModel : public MixedTreeModel
{
public:
    CategorisedEntityModel(Akonadi::ChangeRecorder *monitor, QObject *parent = 0)
        : MixedTreeModel(monitor, parent)
    {

    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (role == KCategorizedSortFilterProxyModel::CategorySortRole) {
            return index.data(MimeTypeRole);
        }
        if (role == KCategorizedSortFilterProxyModel::CategoryDisplayRole) {
            QString mimetype = index.data(MimeTypeRole).toString();
            if (mimetype == QLatin1String("message/rfc822")) {
                return QLatin1String("Email");
            }
            if (mimetype == QLatin1String("text/directory")) {
                return QLatin1String("Addressee");
            }
            if (mimetype == QLatin1String("text/x-vnd.akonadi.note")) {
                return QLatin1String("Note");
            }

        }
        return Akonadi::EntityTreeModel::data(index, role);
    }
};

#endif

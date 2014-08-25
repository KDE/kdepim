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

// READ THE README FILE

#ifndef MIXEDTREEMODEL_H
#define MIXEDTREEMODEL_H

#include <AkonadiCore/entitytreemodel.h>

namespace Akonadi
{
class ChangeRecorder;
}

class MixedTreeModel : public Akonadi::EntityTreeModel
{
    Q_OBJECT
public:
    MixedTreeModel(Akonadi::ChangeRecorder *monitor, QObject *parent = 0);

    /* reimp */ int entityColumnCount(HeaderGroup headerGroup) const;
    /* reimp */ QVariant entityHeaderData(int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup) const;

    /* reimp */ QVariant entityData(const Akonadi::Item &item, int column, int role = Qt::DisplayRole) const;
    /* reimp */ QVariant entityData(const Akonadi::Collection &collection, int column, int role = Qt::DisplayRole) const;

};

#endif

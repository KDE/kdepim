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

#ifndef TRIPMODEL_H
#define TRIPMODEL_H

#include "coisceim_widget_export.h"
#include "mixedtreemodel.h"

class CreateTripWidget;
class Trip;
class TripWidget;

class COISCEIM_WIDGET_EXPORT TripModel : public MixedTreeModel
{
    Q_OBJECT
    Q_ENUMS(CustomRoles)
public:
    enum CustomRoles {
        TripRole = MixedTreeModel::UserRole + 1,
        WidgetRole
    };
    explicit TripModel(Akonadi::ChangeRecorder *monitor, QObject *parent = 0);

    virtual bool removeRows(int , int , const QModelIndex & = QModelIndex());
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

private:
    Trip *createTrip(const QModelIndex &index, Akonadi::Item::Id id) const;
    void createWidget(const QModelIndex &index, Akonadi::Item::Id id) const;

private slots:
    void thisRowsRemoved(const QModelIndex &index, int start, int end);
    void thisModelReset();

    void repopulate();

private:
    mutable QHash<Akonadi::Item::Id, Trip *> m_trips;
    mutable QHash<Akonadi::Item::Id, TripWidget *> m_tripWidgets;
    CreateTripWidget *m_createWidget;
    Akonadi::ChangeRecorder *m_monitor;
};

#endif

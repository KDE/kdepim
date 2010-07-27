/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef FREEPERIODMODEL_H
#define FREEPERIODMODEL_H

#include <kcalcore/period.h>

#include <QAbstractListModel>

namespace IncidenceEditorsNG
{

class FreePeriodModel : public QAbstractListModel
{
Q_OBJECT
public:
    FreePeriodModel(QObject* parent = 0);

    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
public slots:
    void slotNewFreePeriods( const KCalCore::Period::List & freePeriods );
private:
    KCalCore::Period::List mPeriodList;
};

}

#endif // FREEPERIODMODEL_H

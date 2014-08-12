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

#ifndef INCIDENCEEDITOR_FREEPERIODMODEL_H
#define INCIDENCEEDITOR_FREEPERIODMODEL_H

#include "incidenceeditors_ng_export.h"

#include <KCalCore/Period>

#include <QAbstractTableModel>

namespace IncidenceEditorNG {

class INCIDENCEEDITORS_NG_EXPORT FreePeriodModel : public QAbstractTableModel
{
  Q_OBJECT
  public:
    enum Roles {
      PeriodRole = Qt::UserRole
    };
    explicit FreePeriodModel( QObject *parent = 0 );
    virtual ~FreePeriodModel();

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole ) const;

  public slots:
    void slotNewFreePeriods( const KCalCore::Period::List &freePeriods );

  private:
    /** Splits period blocks in the provided list, so that each period occurs on one day */
    KCalCore::Period::List splitPeriodsByDay( const KCalCore::Period::List &freePeriods );

    QString day( int index ) const;
    QString date( int index ) const;
    QString stringify( int index ) const;
    QString tooltipify( int index ) const;

    KCalCore::Period::List mPeriodList;
    friend class FreePeriodModelTest;
};

}

#endif

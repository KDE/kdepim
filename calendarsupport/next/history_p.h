/*
  This file is part of CalendarSupport

  Copyright (c) 2010 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef CALENDARSUPPORT_HISTORY_P_H
#define CALENDARSUPPORT_HISTORY_P_H

#include "history.h"
#include <KCalCore/Incidence>

#include <QPointer>
#include <QStack>

using namespace Akonadi;

namespace CalendarSupport {

  class History;

  enum OperationType {
    TypeNone,
    TypeUndo,
    TypeRedo
  };

  struct Entry {
    // An history entry is something that can be done/undone
    Akonadi::Item oldItem;
    Akonadi::Item newItem;
    Akonadi::Item::Id itemId;
    History::ChangeType changeType;
    IncidenceChanger::WhatChanged whatChanged;
    uint atomicOperationId;
  };

  class History::Private : public QObject {
    Q_OBJECT
    public:
      Private( History *qq ) : q( qq ) {}
      ~Private(){}
      void updateWidgets();
      void doIt( const Entry &entry, OperationType type );
      void updateIds( Item::Id oldId, Item::Id newId );
      void finishOperation( History::ResultCode resultCode );

      bool isUndoAvailable() const;
      bool isRedoAvailable() const;

      IncidenceChanger *mChanger;
      QList<QPointer<QWidget> > mUndoWidgets;
      QList<QPointer<QWidget> > mRedoWidgets;

      QStack<Entry> mUndoStack;
      QStack<Entry> mRedoStack;

      Entry mEntryInProgress;
      OperationType mOperationTypeInProgress;

      QString mLastErrorString;

      QWidget *mParent;

    public Q_SLOTS:
      // to catch incidenceChanger signals
      void addFinished( const Akonadi::Item &, bool );
      void deleteFinished( const Akonadi::Item &, bool );

      void editFinished( const Akonadi::Item &oldinc,
                         const Akonadi::Item &newInc,
                         CalendarSupport::IncidenceChanger::WhatChanged,
                         bool success );

    private:
      History *q;
  };
}

#endif

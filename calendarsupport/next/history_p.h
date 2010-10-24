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
#include "incidencechanger2.h"
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
    IncidenceChanger2::ChangeType changeType;
    uint atomicOperationId;
  };

  class History::Private : public QObject {
    Q_OBJECT
    public:
      Private( History *qq ) : q( qq ) {}
      ~Private(){}
      void updateWidgets();
      bool doIt( const Entry &entry, OperationType, QWidget *parent = 0 );
      void updateIds( Item::Id oldId, Item::Id newId );
      void finishOperation( History::ResultCode, const QString &errorString );
      QStack<Entry>& destinationStack();
      QStack<Entry>& stack();

      void emitDone( OperationType, History::ResultCode );

      bool isUndoAvailable() const;
      bool isRedoAvailable() const;

      IncidenceChanger2 *mChanger;
      QList<QPointer<QWidget> > mUndoWidgets;
      QList<QPointer<QWidget> > mRedoWidgets;

      QStack<Entry> mUndoStack;
      QStack<Entry> mRedoStack;

      Entry mEntryInProgress;
      OperationType mOperationTypeInProgress;

      QString mLastErrorString;
      QHash<Akonadi::Item::Id,int> mLatestRevisionByItemId;

      QWidget *mParent;

    public Q_SLOTS:
      // to catch incidenceChanger signals
      void createFinished( int changeId,
                           const Akonadi::Item &item,
                           const Akonadi::Collection &collectionUsed,
                           CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                           const QString &errorMessage );

      void deleteFinished( int changeId,
                           Akonadi::Item::Id itemId,
                           CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                           const QString &errorMessage );

      void modifyFinished( int changeId,
                           const Akonadi::Item &item,
                           CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                           const QString &errorMessage );

    private:
      History *q;
  };
}

#endif

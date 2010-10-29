/*
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Sergio Martins, <sergio.martins@kdab.com>

  Copyright (C) 2010 Sérgio Martins <iamsergio@gmail.com>

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
#ifndef CALENDARSUPPORT_INCIDENCECHANGER2_P_H
#define CALENDARSUPPORT_INCIDENCECHANGER2_P_H

#include "incidencechanger2.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <QSet>
#include <QObject>
#include <QPointer>

class KJob;
class QWidget;

namespace CalendarSupport {

  class History;

  struct Change {
    Akonadi::Item originalItem;
    Akonadi::Item newItem;

    int changeId;
    uint atomicOperationId;
    bool recordToHistory;
    QPointer<QWidget> parent;

    Akonadi::Collection usedCollection;

    Change(){}
    Change( int id, uint atomicOperId, bool recToHistory, QWidget *p ) :
    changeId( id ), atomicOperationId( atomicOperId ), recordToHistory( recToHistory ), parent( p ){}
  };

class IncidenceChanger2::Private : public QObject
{
  Q_OBJECT
  public:
    explicit Private( IncidenceChanger2 *mIncidenceChanger );
    ~Private();

    /**
       Returns true if, for a specific item, an ItemDeleteJob is already running, or if one already run successfully.
    */
    bool deleteAlreadyCalled( Akonadi::Item::Id id ) const;

    // Does a queued emit, with QMetaObject::invokeMethod
    void emitCreateFinished( int changeId,
                             const Akonadi::Item &item,
                             CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                             const QString &errorString );

    // Does a queued emit, with QMetaObject::invokeMethod
    void emitModifyFinished( int changeId,
                             const Akonadi::Item &item,
                             CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                             const QString &errorString );

    // Does a queued emit, with QMetaObject::invokeMethod
    void emitDeleteFinished( int changeId,
                             const QVector<Akonadi::Item::Id> &itemIdList,
                             CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                             const QString &errorString );

    bool hasRights( const Akonadi::Collection &collection, IncidenceChanger2::ChangeType ) const;
    void queueModification( const Change &change );
    void performModification( Change change );

  public Q_SLOTS:
    void handleCreateJobResult( KJob * );
    void handleModifyJobResult( KJob * );
    void handleDeleteJobResult( KJob * );
    void performNextModification( Akonadi::Item::Id id );
  public:
    int mLatestOperationId;
    QHash<const KJob*,Change> mChangeForJob;
    bool mShowDialogsOnError;
    Akonadi::Collection mDefaultCollection;
    DestinationPolicy mDestinationPolicy;
    QSet<Akonadi::Item::Id> mDeletedItemIds;
    History *mHistory;
    bool mHistoryEnabled;

    /** Queue modifications by ID. We can only send a modification to akonadi when the previous
        one ended.

        The container doesn't look like a queue because of an optimization: if there's a modification
        A in progress, a modification B waiting (queued), and then a new one C comes in, we just discard
        B, and queue C. The queue always has 1 element max.
    */
    QHash<Akonadi::Item::Id,Change> mQueuedModifications;

    /**
       So we know if there's already a modification in progress
    */
    QHash<Akonadi::Item::Id,Change> mModificationsInProgress;

  private:
    IncidenceChanger2 *q;
};

}

#endif

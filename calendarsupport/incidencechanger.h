/*
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef CALENDARSUPPORT_INCIDENCECHANGER_H
#define CALENDARSUPPORT_INCIDENCECHANGER_H

#include "calendarsupport_export.h"

#include <Akonadi/Item>

#include <KCalCore/Incidence>
#include <KCalCore/ScheduleMessage>

#include <QObject>

class KJob;

namespace CalendarSupport {

class Calendar;

class CALENDARSUPPORT_EXPORT IncidenceChanger : public QObject
{
  Q_OBJECT
  public:
    IncidenceChanger( CalendarSupport::Calendar *cal,
                      QObject *parent,
                      Akonadi::Entity::Id defaultCollectionId = -1 );
    ~IncidenceChanger();

    enum HowChanged {
      INCIDENCEADDED,
      INCIDENCEEDITED,
      INCIDENCEDELETED,
      NOCHANGE
    };

    enum WhatChanged {
      PRIORITY_MODIFIED,
      COMPLETION_MODIFIED,
      CATEGORY_MODIFIED,
      DATE_MODIFIED,
      RELATION_MODIFIED,
      ALARM_MODIFIED,
      DESCRIPTION_MODIFIED,
      SUMMARY_MODIFIED,
      COMPLETION_MODIFIED_WITH_RECURRENCE,
      RECURRENCE_MODIFIED_ONE_ONLY,
      RECURRENCE_MODIFIED_ALL_FUTURE,
      UNKNOWN_MODIFIED,
      NOTHING_MODIFIED
    };

    enum DestinationPolicy {
      USE_DEFAULT_DESTINATION,   // the default collection is used, unless it's invalid
      ASK_DESTINATION          // user is asked in which collection
    };

    // TODO: Remove this from public api. If classes outside changes realy need
    //       to send ICal message, they should use InvitationHandler.
    bool sendGroupwareMessage( const Akonadi::Item &incidence,
                               KCalCore::iTIPMethod method,
                               HowChanged action,
                               QWidget *parent,
                               uint atomicOperationId = 0 );

    // returns true if the add job was created
    bool addIncidence( const KCalCore::Incidence::Ptr &incidence,
                       QWidget *parent,
                       Akonadi::Collection &selectedCollection,
                       int &dialogCode,
                       uint atomicOperationId = 0 );

    // returns true if the add job was created
    bool addIncidence( const KCalCore::Incidence::Ptr &incidence,
                       const Akonadi::Collection &collection,
                       QWidget *parent,
                       uint atomicOperationId = 0 );

    bool changeIncidence( const KCalCore::Incidence::Ptr &oldinc,
                          const Akonadi::Item &newItem,
                          WhatChanged,
                          QWidget *parent,
                          uint atomicOperationId = 0 );

    // returns true if the delete job was created
    // TODO: true/false isn't enough for the API, if the user deletes the same
    // item twice (very quickly), and deleteIncidence() detects that there's an ongoing
    // deletion, what do we return here?
    // If we return false, the application will probably show an error.
    // If we return true, the application will assume success.. but there's still the
    // chance that the running deletion isn't successful
    // So we need a third return value that says "ignore me".
    bool deleteIncidence( const Akonadi::Item &incidence, uint atomicOperationId = 0,
                          QWidget *parent = 0 );

    bool cutIncidences( const Akonadi::Item::List &incidences, QWidget *parent );
    bool cutIncidence( const Akonadi::Item &incidence, QWidget *parent );

    void setDefaultCollectionId( Akonadi::Entity::Id );

    void setDestinationPolicy( DestinationPolicy destinationPolicy );

    DestinationPolicy destinationPolicy() const;

    /**
       Sets the akonadi calendar.
       @param calendar the calendar.
    */
    void setCalendar( CalendarSupport::Calendar *calendar );

    /*
     * Returns false if the item is being deleted by a job
     * or was deleted already.
     *
     * This is more accurate than querying the ETM because when a delete
     * job ends the ETM still has the item for a short period of time.
     */
    bool isNotDeleted( Akonadi::Item::Id ) const;


    /**
       Returns true if there's a modify job in progress for the specified item.
    */
    bool changeInProgress( Akonadi::Item::Id );

    /**
       Some incidence operations require more than one change. Like dissociating
       occurrences, which needs an incidence add, and an incidence change.

       If you want the prevent that the same dialogs are presented multiple times
       use this function, which returns an id for your atomic operation.

       Use that id on all addIncidence()/changeIncidence()/deleteIncidence() calls
       that belong to the same atomic operation.

       TODO: Would be nice to have undo support, in case one operation,
             (other than the first) fails.
    */
    uint startAtomicOperation();

    /**
       Tells IncidenceChanger you won't be doing more changes with atomic operation
       id @p atomicOperationId

       (Internaly, this function only does cleanup.)
       @see startAtomicOperation()
    */
    void endAtomicOperation( uint atomicOperationId );

  public Q_SLOTS:
    void cancelAttendees( const Akonadi::Item &incidence );

  Q_SIGNALS:
    // Signals emitted by the Item*Job, the bool parameter is the success of the operation
    void incidenceAddFinished( const Akonadi::Item &, bool );

    void incidenceChangeFinished( const Akonadi::Item &oldinc,
                                  const Akonadi::Item &newInc,
                                  CalendarSupport::IncidenceChanger::WhatChanged,
                                  bool );

    void incidenceDeleteFinished( const Akonadi::Item &, bool );

    void incidenceToBeDeleted( const Akonadi::Item & );
    void schedule( KCalCore::iTIPMethod method, const Akonadi::Item &incidence );

  private Q_SLOTS:
    void addIncidenceFinished( KJob *job );
    void deleteIncidenceFinished( KJob *job );

  private:
    class Private;
    Private *const d;
};
}

#endif

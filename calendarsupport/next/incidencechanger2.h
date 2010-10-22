/*
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
#ifndef CALENDARSUPPORT_INCIDENCECHANGER2_H
#define CALENDARSUPPORT_INCIDENCECHANGER2_H

#include "calendarsupport_export.h"

#include <Akonadi/Collection>

#include <QWidget>
#include <QObject>

namespace CalendarSupport {

class Calendar;

class CALENDARSUPPORT_EXPORT IncidenceChanger2 : public QObject
{
  Q_OBJECT
  public:

    enum ResultCode {
      ResultCodeSuccess = 0
    }

    enum DestinationPolicy {
      DestinationPolicyDefault, ///< The default collection is used, if it's invalid, the user is prompted. @see setDefaultCollection().
      DestinationPolicyDefaultOnly ///< The default collection is used, if it's invalid, an error is returned, and the incidence isn't added.
      DestinationPolicyAsk      ///< User is always asked which collection to use.
    };

    /**
       This enum describes change types.
       TODO: delete this enum from history.h
    */
    enum ChangeType {
      ChangeTypeNone,     ///> Nothing happened.
      ChangeTypeCreate,   ///> Represents an incidence creation.
      ChangeTypeModify,   ///> Represents an incidence modification.
      ChangeTypeDelete    ///> Represents an incidence deletion.
    };

    explicit IncidenceChanger2( CalendarSupport::Calendar *calendar );
    ~IncidenceChanger2();

    int createIncidence( const KCalCore::Incidence::Ptr &incidence,
                         const Akonadi::Collection &collection = Akonadi::Collection(),
                         uint atomicOperationId = 0,
                         QWidget *parent = 0 );

    int deleteIncidence( const Akonadi::Item &item,
                         uint atomicOperationId = 0,
                         QWidget *parent = 0 );

    int modifyIncidence( const Akonadi::Item &changedItem,
                         const Akonadi::Item &originalItem = Akonadi::Item(),
                         uint atomicOperationId = 0,
                         QWidget *parent = 0 );

    /**
       Some incidence operations require more than one change. Like dissociating
       occurrences, which needs an incidence add, and an incidence change.

       If you want to prevent that the same dialogs are presented multiple times
       use this function, which returns an id for your atomic operation.

       Use that id on all createIncidence()/modifyIncidence()/deleteIncidence() calls
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

    void setDefaultCollectionId( Akonadi::Collection::Id id );
    Akonadi::Collection::Id defaultCollectionId() const;

    void setDestinationPolicy( DestinationPolicy destinationPolicy );
    DestinationPolicy destinationPolicy() const;

    //TODO: how to id which change?
    QString lastErrorString() const;

  Q_SIGNALS:
    void createFinished( CalendarSupport::IncidenceChanger2::ResultCode resultCode );
    void modifyFinished( CalendarSupport::IncidenceChanger2::ResultCode resultCode );
    void deleteFinished( CalendarSupport::IncidenceChanger2::ResultCode resultCode );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif

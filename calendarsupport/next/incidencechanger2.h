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

#include <Akonadi/Item>
#include <Akonadi/Collection>

#include <KCalCore/Incidence>

#include <QWidget>
#include <QObject>

namespace CalendarSupport {

class History;

class CALENDARSUPPORT_EXPORT IncidenceChanger2 : public QObject
{
  Q_OBJECT
  public:

    /**
       This enum describes result codes which are returned by createFinished(), modifyfinished() and
       deleteFinished() signals.
     */
    enum ResultCode {
      ResultCodeSuccess = 0,
      ResultCodeJobError,
      ResultCodeAlreadyDeleted, ///< That calendar item was already deleted, or currently being deleted.
      ResultCodeInvalidDefaultCollection, ///< Default collection is invalid and DestinationPolicyNeverAsk was used
      ResultCodeRollback
    };

    /**
       This enum describes destination policies.
       Destination policies control how the createIncidence() method chooses the collection where
       the item will be created.
     */
    enum DestinationPolicy {
      DestinationPolicyDefault, ///< The default collection is used, if it's invalid, the user is prompted. @see setDefaultCollection().
      DestinationPolicyAsk,      ///< User is always asked which collection to use.
      DestinationPolicyNeverAsk ///< The default collection is used, if it's invalid, an error is returned, and the incidence isn't added.
    };

    /**
       This enum describes change types.
    */
    enum ChangeType {
      ChangeTypeNone,     ///> Nothing happened.
      ChangeTypeCreate,   ///> Represents an incidence creation.
      ChangeTypeModify,   ///> Represents an incidence modification.
      ChangeTypeDelete    ///> Represents an incidence deletion.
    };

    /**
       Creates a new IncidenceChanger instance.
     */
    IncidenceChanger2();

    /**
       Destroys this IncidenceChanger instance.
     */
    ~IncidenceChanger2();

    int createIncidence( const KCalCore::Incidence::Ptr &incidence,
                         const Akonadi::Collection &collection = Akonadi::Collection(),
                         uint atomicOperationId = 0,
                         bool recordToHistory = true,
                         QWidget *parent = 0 );

    int deleteIncidence( const Akonadi::Item &item,
                         uint atomicOperationId = 0,
                         bool recordToHistory = true,
                         QWidget *parent = 0 );

    int deleteIncidences( const Akonadi::Item::List &items,
                          uint atomicOperationId = 0,
                          bool recordToHistory = true,
                          QWidget *parent = 0 );

    // todo: explain that if originalItem is invalid, it won't be recorded to history
    int modifyIncidence( const Akonadi::Item &changedItem,
                         const Akonadi::Item &originalItem = Akonadi::Item(),
                         uint atomicOperationId = 0,
                         bool recordToHistory = true,
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

    /**
       Returns a pointer to the History object. It's always a valid pointer,
       only freed in ~IncidenceChanger.

       @code
           mIncidenceChanger->createIncidence( incidence, collection );

           CalendarSupport::History *history = mIncidenceChanger->history();
           connect( history, SIGNAL(undone(CalendarSupport:History::ResultCode)),
                    SLOT(undone(CalendarSupport::History::ResultCode)) )
           history->undo();
       @endcode
    */
    CalendarSupport::History *history() const;

    void setDefaultCollection( const Akonadi::Collection &collection );


    /**
       Returns the defaultCollection.
       If none is set, an invalid Collection is returned.
       @see setDefaultCollection()
     */
    Akonadi::Collection defaultCollection() const;

    void setDestinationPolicy( DestinationPolicy destinationPolicy );

    /**
       Returns the current destination policy.
       If none is set, DestinationPolicyDefault is returned.
       @see setDestinationPolicy()
    */
    DestinationPolicy destinationPolicy() const;

    void setShowDialogsOnError( bool enable );


    /**
       Returns if error dialogs are shown by IncidenceChanger when an ItemModify|Create|DeleteJob()
       fails.
       The dialog is shown right before emitting create|delete|modifyFinshed() with ResultCode equal
       to ResultCodeJobError.

       The default is true.

       @see setShowDialogsOnError
     */
    bool showDialogsOnError() const;

    //TODO: document what happens to items with invalid parent collections
    void setRespectsCollectionRights( bool respect );
    bool respectsCollectionRights() const;

  Q_SIGNALS:
    void createFinished( int changeId,
                         const Akonadi::Item &item,
                         CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                         const QString &errorString );

    void modifyFinished( int changeId,
                         const Akonadi::Item &item,
                         CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                         const QString &errorString );

    void deleteFinished( int changeId,
                         const QVector<Akonadi::Item::Id> &itemIdList,
                         CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                         const QString &errorString );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif

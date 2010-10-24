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


#ifndef CALENDARSUPPORT_HISTORY_H
#define CALENDARSUPPORT_HISTORY_H

#include "incidencechanger2.h"

#include <KCalCore/Incidence>

#include <Akonadi/Item>

#include <QWidget>

namespace CalendarSupport {

class IncidenceChanger2;

/**
   @short History class for implementing undo/redo in your application.

   Keeps a stack of all changes ( incidence adds, edits and deletes ) and
   will fire Item{Modify|Add|Delete}Jobs when undo() and redo() slots are
   called.

   Doesn't really use Item*Jobs directly, it uses IncidenceChanger, so invitation
   e-mails are sent.

   TODO: Talk about atomic operations.

   @code
      TODO:
   @endcode

   @author Sérgio Martins <iamsergio@gmail.com>
*/

class CALENDARSUPPORT_EXPORT History : public QObject {
  Q_OBJECT
  public:

    /**
       This enum describes the possible result codes (success/error values)
       for an undo or redo operation
       @see undone()
       @see redone()
    */
    enum ResultCode {
      ResultCodeSuccess = 0, ///< Success
      ResultCodeError, ///< An error occurred. Call lastErrorString() for the error message. This isn't very verbose because IncidenceChanger hasn't been refactored yet.
      ResultCodeIncidenceChangerError ///< IncidenceChanger returned false and didn't even create the job. This error is temporary. IncidenceChanger needs to be refactored.

    };

    /**
       Creates an History instance.
       @param changer a valid pointer to an IncidenceChanger. Ownership is not taken.
    */
    explicit History( IncidenceChanger2 *changer );

    /**
       Destroys the History instance.
    */
    ~History();

    /**
       Pushes an incidence creation into the undo stack. The creation can be undone calling
       undo().

       @param incidence The item that was created. Must be valid.
       @param atomicOperation If != 0, specifies which group of changes this change belongs too.
              Will be useful for atomic undoing/redoing, not implemented yet.
     */
    void recordCreation( const Akonadi::Item &item,
                         const uint atomicOperationId = 0 );

    /**
       Pushes an incidence modification into the undo stack. The modification can be undone calling
       undo().

       @param oldItem item containing the payload before the change. Must be valid.
       @param newitem item containing the new payload. Must be valid.
       @param atomicOperation If != 0, specifies which group of changes this change belongs too.
              Will be useful for atomic undoing/redoing, not implemented yet.
     */
    void recordModification( const Akonadi::Item &oldItem,
                             const Akonadi::Item &newItem,
                             const uint atomicOperationId = 0 );

    /**
       Pushes an incidence deletion into the undo stack. The deletion can be undone calling
       undo().

       @param item The item to delete. Must be valid.
       @param atomicOperation If != 0, specifies which group of changes this change belongs too.
              Will be useful for atomic undoing/redoing, not implemented yet.
     */
    void recordDeletion( const Akonadi::Item &item,
                         const uint atomicOperationId = 0 );

    /**
       Convenience method for disabling the undo/redo button when jobs are in progress,
       so callers don't have to write signal/slot/connect logic for this.

       Each widget registed here will be disabled/enabled depending if there are jobs
       running, or the undo/redo stack contains data.

       @param widget The widget pointer to register. Ownership is not taken.
                     You are free to delete the widget, QPointers are used internally.
     */
    void registerUndoWidget( QWidget *widget );

    /**
       @copydoc registerUndoWidget()
    */
    void registerRedoWidget( QWidget *widget );

    /**
       Returns the last error message.

       Call this immediately after catching the undone()/redone() signal
       with an ResultCode != ResultCodeSuccess.

       The message is translated.
    */
    QString lastErrorString() const;

    /**
       Reverts the change that's ontop of the undo stack.
       Can't be called if there's an undo/redo operation running, Q_ASSERTs.
       Can be called if the stack is empty, in this case, nothing happens.
       This function is async, listen to signal undone() to know when the operation finishes.

       @param parent will be passed to dialogs created by IncidenceChanger, for example
              those which ask if you want to send invitations.

       @return true if the job was started, otherwise false is returned and the undone() signal
               won't be emitted.

       @see redo()
       @see undone()
     */
   bool undo( QWidget *parent = 0 );

    /**
       Reverts the change that's ontop of the redo stack.
       Can't be called if there's an undo/redo operation running, Q_ASSERTs.
       Can be called if the stack is empty, in this case, nothing happens.
       This function is async, listen to signal redone() to know when the operation finishes.

       @param parent will be passed to dialogs created by IncidenceChanger, for example
              those which ask if you want to send invitations.

       @return true if the job was started, otherwise false is returned and the redone() signal
               won't be emitted.

       @see undo()
       @see redone()
     */
    bool redo( QWidget *parent = 0 );

  public Q_SLOTS:
    /**
       Clears the undo and redo stacks.
       Won't do anything if there's a undo/redo job currently running.

       @return true if the stacks were cleared, false if there was a job running
     */
    bool clear();

  Q_SIGNALS:
    /**
       This signal is emitted when an undo operation finishes.
       @param resultCode History::ResultCodeSuccess on success.
       @see lastErrorString()
     */
    void undone( CalendarSupport::History::ResultCode resultCode );

    /**
       This signal is emitted when an redo operation finishes.
       @param resultCode History::ResultCodeSuccess on success.
       @see lastErrorString()
     */
    void redone( CalendarSupport::History::ResultCode resultCode );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
  };
}

#endif

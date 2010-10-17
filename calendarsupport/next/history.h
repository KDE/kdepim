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

#include "incidencechanger.h"

#include <KCalCore/Incidence>

#include <Akonadi/Item>

#include <QWidget>

namespace CalendarSupport {

class IncidenceChanger;

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
      ResultCodeError ///< An error occurred. Call lastErrorString() for the error message. This isn't very verbose because IncidenceChanger hasn't been refactored yet.
    };

    /**
       This enum describes change types.
    */
    enum ChangeType {
      ChangeTypeAdd,   ///> Represents an incidence creation.
      ChangeTypeEdit,  ///> Represents an incidence modification.
      ChangeTypeDelete ///> Represents an incidence deletion.
    };

    /**
       Creates an History instance.
       @param changer a valid pointer to an IncidenceChanger. Ownership is not taken.
       @param parent will be passed to dialogs created by IncidenceChanger, for example
              those which ask if you want to send invitations.
    */
    explicit History( IncidenceChanger *changer, QWidget *parent = 0 );

    /**
       Destroys the History instance.
    */
    ~History();

    /**
       Pushes a change into the undo stack. The change can be undone calling
       undo().

       @param oldItem item containing the payload before the change. Pass an invalid item
              if this is an incidence addition.
       @param newitem item containing the new payload. Pass an invalid item if this is an
              incidence deletion.
       @param changeType Specifies if we added, deleted or edited an incidence.
       @param whatChanged Specifies which fields were changed. Only useful when editing incidences.
              The invitation handler will use this information to craft the i18n strings to show
              in the dialogs.
       @param atomicOperation If != 0, specifies which group of changes this change belongs too.
              Will be useful for atomic undoing/redoing, not implemented yet.
     */
    void recordChange( const Akonadi::Item &oldItem,
                       const Akonadi::Item &newItem,
                       History::ChangeType changeType,
                       IncidenceChanger::WhatChanged whatChanged = IncidenceChanger::NOTHING_MODIFIED,
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

  public Q_SLOTS:

    /**
       Reverts the change that's ontop of the undo stack.
       Can't be called if there's an undo/redo operation running, Q_ASSERTs.
       Can be called if the stack is empty, in this case, nothing happens.
       This function is async, listen to signal undone() to know when it finishes.
       @see redo()
       @see undone()
     */
    void undo();

    /**
       Reverts the change that's ontop of the redo stack.
       Can't be called if there's an undo/redo operation running, Q_ASSERTs.
       Can be called if the stack is empty, in this case, nothing happens.
       This function is async, listen to signal redone() to know when it finishes.
       @see undo()
       @see redone()
     */
    void redo();

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

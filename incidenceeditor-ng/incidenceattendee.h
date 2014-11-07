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

#ifndef INCIDENCEEDITOR_INCIDENCEATTENDEE_H
#define INCIDENCEEDITOR_INCIDENCEATTENDEE_H

#include "incidenceeditor-ng.h"

namespace Ui {
  class EventOrTodoDesktop;
  class EventOrTodoMore;
}

namespace KPIM {
  class MultiplyingLine;
}

namespace KContacts {
  class Addressee;
}

class KJob;

namespace IncidenceEditorNG {

class AttendeeEditor;
class ConflictResolver;
class IncidenceDateTime;

class INCIDENCEEDITORS_NG_EXPORT IncidenceAttendee : public IncidenceEditor
{
  Q_OBJECT
  public:
#ifdef KDEPIM_MOBILE_UI
    IncidenceAttendee( QWidget *parent, IncidenceDateTime *dateTime, Ui::EventOrTodoMore *ui );
#else
    IncidenceAttendee( QWidget *parent, IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui );
#endif
    ~IncidenceAttendee();

    virtual void load( const KCalCore::Incidence::Ptr &incidence );
    virtual void save( const KCalCore::Incidence::Ptr &incidence );
    virtual bool isDirty() const;
    virtual void printDebugInfo() const;

  signals:
    void attendeeCountChanged( int );

  public slots:
    /// If the user is attendee of the loaded event, one of the following slots
    /// can be used to change the status.
    void acceptForMe();
    void declineForMe();

  private slots:
    void checkIfExpansionIsNeeded( KPIM::MultiplyingLine * );
    void expandResult( KJob *job );
    void groupSearchResult( KJob *job );
    void slotSelectAddresses();
    void slotSolveConflictPressed();
    void slotUpdateConflictLabel( int );
    void slotAttendeeChanged( const KCalCore::Attendee::Ptr &oldAttendee,
                              const KCalCore::Attendee::Ptr &newAttendee );
    void slotOrganizerChanged( const QString &organizer );

    // wrapper for the conflict resolver
    void slotEventDurationChanged();

  private:
    void changeStatusForMe( KCalCore::Attendee::PartStat );

    /** Returns if I was the organizer of the loaded event */
    bool iAmOrganizer() const;

    /** Reads values from a KContacts::Addressee and inserts a new Attendee
     * item into the listview with those items. Used when adding attendees
     * from the addressbook and expanding distribution lists.
     * The optional Attendee parameter can be used to pass in default values
     * to be used by the new Attendee.
     */
    void insertAttendeeFromAddressee( const KContacts::Addressee &a );
    void fillOrganizerCombo();

#ifdef KDEPIM_MOBILE_UI
    Ui::EventOrTodoMore *mUi;
#else
    Ui::EventOrTodoDesktop *mUi;
#endif
    QWidget *mParentWidget;
    AttendeeEditor *mAttendeeEditor;
    ConflictResolver *mConflictResolver;
    QMap<KJob *,QWeakPointer<KPIM::MultiplyingLine> > mMightBeGroupLines;
    IncidenceDateTime *mDateTime;
    QString mOrganizer;
};

}

#endif

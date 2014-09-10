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
#include "attendeetablemodel.h"

#include <KCalCore/FreeBusy>

namespace Ui {
  class EventOrTodoDesktop;
  class EventOrTodoMore;
}

namespace KPIM {
  class MultiplyingLine;
}

namespace KABC {
  class Addressee;
  class ContactGroup;
}

class KJob;

namespace IncidenceEditorNG {

class AttendeeEditor;
class AttendeeComboBoxDelegate;
class AttendeeLineEditDelegate;
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

    AttendeeTableModel * dataModel();
    AttendeeComboBoxDelegate *stateDelegate();
    AttendeeComboBoxDelegate *roleDelegate();
    AttendeeComboBoxDelegate *responseDelegate();
    AttendeeLineEditDelegate *attendeeDelegate();

    int attendeeCount() const;

  signals:
    void attendeeCountChanged( int );

  public slots:
    /// If the user is attendee of the loaded event, one of the following slots
    /// can be used to change the status.
    void acceptForMe();
    void declineForMe();

  private slots:
    // cheks if row is a group,  that can/should be expanded
    void checkIfExpansionIsNeeded(KCalCore::Attendee::Ptr attendee);

    // results of the group search job
    void groupSearchResult( KJob *job );
    void expandResult( KJob *job );
    void slotSelectAddresses();
    void slotSolveConflictPressed();
    void slotUpdateConflictLabel( int );
    void slotOrganizerChanged( const QString &organizer );
    void slotGroupSubstitutionPressed();

    // wrapper for the conflict resolver
    void slotEventDurationChanged();

    void filterLayoutChanged();
    void updateCount();

    void slotConflictResolverAttendeeAdded(const QModelIndex &index, int first, int last);
    void slotConflictResolverAttendeeRemoved(const QModelIndex &index, int first, int last);
    void slotConflictResolverAttendeeChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void slotConflictResolverLayoutChanged();
    void slotFreeBusyAdded(const QModelIndex &index, int first, int last);
    void slotFreeBusyChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void updateFBStatus();
    void updateFBStatus(const KCalCore::Attendee::Ptr &attendee, const KCalCore::FreeBusy::Ptr &fb);

    void slotGroupSubstitutionAttendeeAdded(const QModelIndex &index, int first, int last);
    void slotGroupSubstitutionAttendeeRemoved(const QModelIndex &index, int first, int last);
    void slotGroupSubstitutionAttendeeChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void slotGroupSubstitutionLayoutChanged();

private:
    void updateGroupExpand();

    void changeStatusForMe( KCalCore::Attendee::PartStat );

    /** Returns if I was the organizer of the loaded event */
    bool iAmOrganizer() const;

    /** Reads values from a KABC::Addressee and inserts a new Attendee
     * item into the listview with those items. Used when adding attendees
     * from the addressbook and expanding distribution lists.
     * The optional Attendee parameter can be used to pass in default values
     * to be used by the new Attendee.
     * pos =-1 means insert attendee before empty line
     */
    void insertAttendeeFromAddressee( const KABC::Addressee &a, int pos = -1);
    void fillOrganizerCombo();
    void setActions( KCalCore::Incidence::IncidenceType actions );

#ifdef KDEPIM_MOBILE_UI
    Ui::EventOrTodoMore *mUi;
#else
    Ui::EventOrTodoDesktop *mUi;
#endif
    QWidget *mParentWidget;
    ConflictResolver *mConflictResolver;

    IncidenceDateTime *mDateTime;
    QString mOrganizer;

    /** used dataModel to rely on*/
    AttendeeTableModel *mDataModel;
    AttendeeLineEditDelegate *mAttendeeDelegate;
    AttendeeComboBoxDelegate *mStateDelegate;
    AttendeeComboBoxDelegate *mRoleDelegate;
    AttendeeComboBoxDelegate *mResponseDelegate;

    QMap<KCalCore::Attendee::Ptr, KABC::ContactGroup> mGroupList;
    QMap<KJob *, KCalCore::Attendee::Ptr> mMightBeGroupJobs;
    QMap<KJob *, KCalCore::Attendee::Ptr> mExpandGroupJobs;
};

}

#endif

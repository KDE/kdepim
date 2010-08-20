/*
  This file is part of KOrganizer.
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef EDITORDETAILS_H
#define EDITORDETAILS_H

#include "customlistviewitem.h"
#include "attendeeeditor.h"

#include <kcalcore/incidence.h>
#include <kcalcore/attendee.h>

#include <K3ListView>

namespace IncidenceEditors {
  typedef CustomListViewItem<KCalCore::Attendee::Ptr > AttendeeListItem;
}

class AttendeeListView;

/** AttendeeListView is a child class of K3ListView  which supports
 *  dropping of attendees (e.g. from kaddressbook) onto it. If an attendeee
 *  was dropped, the signal dropped(Attendee*)  is emitted.
 */
class AttendeeListView : public K3ListView
{
  Q_OBJECT
  public:
    AttendeeListView( QWidget *parent=0 );
    virtual ~AttendeeListView();
    virtual void addAttendee( const QString &newAttendee );

#ifndef KORG_NODND
  public slots:
    virtual void contentsDragEnterEvent( QDragEnterEvent *e );
    virtual void dragEnterEvent( QDragEnterEvent *e );
    virtual void contentsDropEvent( QDropEvent *e );
    virtual void dropEvent( QDropEvent *e );
    virtual void contentsDragMoveEvent( QDragMoveEvent *e );

  signals:
    void dropped( const KCalCore::Attendee::Ptr & );
#endif
};

class EditorDetails : public AttendeeEditor
{
  Q_OBJECT
  public:
    explicit EditorDetails( int spacing = 8, QWidget *parent = 0 );
    virtual ~EditorDetails();

    /** Set widgets to default values */
    void setDefaults();

    /** Read incidence and setup widgets accordingly */
    void readIncidence( const KCalCore::Incidence::Ptr &incidence );

    /** Write settings to incidence */
    void fillIncidence( KCalCore::Incidence::Ptr &incidence );

    /** Check if the input is valid. */
    bool validateInput();

    /** Returns whether at least one attendee was added */
    bool hasAttendees();

    void insertAttendee( const KCalCore::Attendee::Ptr & , bool goodEmailAddress=true );

  protected slots:
    void removeAttendee();
    void slotInsertAttendee( const KCalCore::Attendee::Ptr &a );

  protected:
    void changeStatusForMe( KCalCore::Attendee::PartStat status );

    KCalCore::Attendee::Ptr currentAttendee() const;
    /* reimpl */
    Q3ListViewItem *hasExampleAttendee() const;
    void updateCurrentItem();

  private:
    bool mDisableItemUpdate;

    AttendeeListView *mListView;
};

#endif

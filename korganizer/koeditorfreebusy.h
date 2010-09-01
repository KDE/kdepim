/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOEDITORFREEBUSY_H
#define KOEDITORFREEBUSY_H

#include "koattendeeeditor.h"

#include <tqwidget.h>
#include <tqdatetime.h>
#include <tqtimer.h>

class KDIntervalColorRectangle;
class TQLabel;
class KDGanttView;
class KDGanttViewItem;
class FreeBusyItem;

namespace KCal {
  class FreeBusy;
  class Attendee;
}


class KOEditorFreeBusy : public KOAttendeeEditor
{
    Q_OBJECT
  public:
    KOEditorFreeBusy( int spacing = 8, TQWidget *parent = 0,
                      const char *name = 0 );
    virtual ~KOEditorFreeBusy();

    void setUpdateEnabled( bool enabled );
    bool updateEnabled() const;

    void insertAttendee( KCal::Attendee *, bool readFBList = true );
    void removeAttendee( KCal::Attendee * );
    void clearAttendees();

    void readEvent( KCal::Event * );
    void writeEvent( KCal::Event *event );

    void triggerReload();
    void cancelReload();

  signals:
    void dateTimesChanged( const TQDateTime &, const TQDateTime & );

  public slots:
    void slotInsertFreeBusy( KCal::FreeBusy *fb, const TQString &email );

    void setDateTimes( const TQDateTime &, const TQDateTime & );

    void editFreeBusyUrl( KDGanttViewItem *item );

  protected slots:
    void slotUpdateGanttView( const TQDateTime &, const TQDateTime & );
    void slotScaleChanged( int );
    void slotCenterOnStart() ;
    void slotZoomToTime();
    void slotPickDate();
    void showAttendeeStatusMenu();

    // Force the download of FB informations
    void manualReload();
    // Only download FB if the auto-download option is set in config
    void autoReload();
    void slotIntervalColorRectangleMoved( const TQDateTime& start, const TQDateTime& end );

    void removeAttendee();
    void listViewClicked( int button, KDGanttViewItem* item );

  protected:
    void timerEvent( TQTimerEvent* );
    KCal::Attendee* currentAttendee() const;
    /* reimpl */
    TQListViewItem* hasExampleAttendee() const;
    void updateCurrentItem();
    void clearSelection() const;
    void setSelected ( int index );
    int selectedIndex();
    void changeStatusForMe( KCal::Attendee::PartStat status );
    virtual bool eventFilter( TQObject *watched, TQEvent *event );

  private slots:
    void slotOrganizerChanged( const TQString &newOrganizer );
  private:
    void updateFreeBusyData( FreeBusyItem * );

    bool findFreeSlot( TQDateTime &dtFrom, TQDateTime &dtTo );
    bool tryDate( TQDateTime &tryFrom, TQDateTime &tryTo );
    bool tryDate( FreeBusyItem *attendee,
                  TQDateTime &tryFrom, TQDateTime &tryTo );
    void updateStatusSummary();
    void reload();
    KDGanttView *mGanttView;
    KDIntervalColorRectangle* mEventRectangle;
    TQLabel *mStatusSummaryLabel;
    bool mIsOrganizer;
    TQComboBox *scaleCombo;

    TQDateTime mDtStart, mDtEnd;

    TQTimer mReloadTimer;

    bool mForceDownload;

    TQString mCurrentOrganizer;
};

#endif

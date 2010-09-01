/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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
*/

#ifndef KORG_MULTIAGENDAVIEW_H_H
#define KORG_MULTIAGENDAVIEW_H_H

#include "agendaview.h"
#include "calendarview.h"

class TQScrollView;
class TQHBox;
class TQSplitter;
class KOAgendaView;
class TimeLabels;
class TQScrollBar;

namespace KCal {
  class ResourceCalendar;
}

namespace KOrg {

/**
  Shows one agenda for every resource side-by-side.
*/
class MultiAgendaView : public AgendaView
{
  Q_OBJECT
  public:
    explicit MultiAgendaView( Calendar* cal, CalendarView *calendarView,
                              TQWidget *parent = 0, const char *name = 0 );
    ~MultiAgendaView();

    KOAgendaView *selectedAgendaView();
    void deSelectAgendaView() { mSelectedAgendaView = 0; }
    Incidence::List selectedIncidences();
    DateList selectedIncidenceDates();
    int currentDateCount();
    int maxDatesHint();

    bool eventDurationHint(TQDateTime &startDt, TQDateTime &endDt, bool &allDay);

    void setTypeAheadReceiver( TQObject *o );

  public slots:
    void showDates( const TQDate &start, const TQDate &end );
    void showIncidences( const Incidence::List &incidenceList, const TQDate &date );
    void updateView();
    void changeIncidenceDisplay( Incidence *incidence, int mode );
    void updateConfig();

    void setIncidenceChanger( IncidenceChangerBase *changer );

    void finishTypeAhead();

    void show();

    void resourcesChanged();

  protected:
    void resizeEvent( TQResizeEvent *ev );
    bool eventFilter( TQObject *obj, TQEvent *event );

  private:
    void addView( const TQString &label, KCal::ResourceCalendar *res, const TQString &subRes = TQString::null );
    void deleteViews();
    void recreateViews();
    void setupViews();
    void resizeScrollView( const TQSize &size );
    void installSplitterEventFilter( TQSplitter *splitter );

  private slots:
    void slotSelectionChanged();
    void slotClearTimeSpanSelection();
    void resizeSplitters();
    void resizeSpacers( int );
    void setupScrollBar();
    void zoomView( const int delta, const TQPoint &pos, const Qt::Orientation ori );
    void slotResizeScrollView();

  private:
    KOAgendaView *mSelectedAgendaView;
    TQValueList<KOAgendaView*> mAgendaViews;
    TQValueList<TQWidget*> mAgendaWidgets;
    TQHBox *mTopBox;
    TQScrollView *mScrollView;
    TimeLabels *mTimeLabels;
    TQSplitter *mLeftSplitter, *mRightSplitter;
    TQSplitter *mLastMovedSplitter;
    TQScrollBar *mScrollBar;
    TQWidget *mLeftTopSpacer, *mRightTopSpacer;
    TQWidget *mLeftBottomSpacer, *mRightBottomSpacer;
    TQDate mStartDate, mEndDate;
    bool mUpdateOnShow;
    bool mPendingChanges;
    CalendarView *mCalendarView;
};

}

#endif

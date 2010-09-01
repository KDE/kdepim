/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KOAGENDAVIEW_H
#define KOAGENDAVIEW_H

#include <tqscrollview.h>
#include <tqlabel.h>

#include <libkcal/calendar.h>

#include "calprinter.h"
#include "calendarview.h"

#include "agendaview.h"

class TQHBox;
class TQPushButton;
class TQBoxLayout;

class KOAgenda;
class KOAgendaItem;
class TimeLabels;
class KConfig;

namespace KOrg {
  class IncidenceChangerBase;
}

class EventIndicator : public QFrame
{
    Q_OBJECT
  public:
    enum Location { Top, Bottom };
    EventIndicator( Location loc = Top, TQWidget *parent = 0,
                    const char *name = 0 );
    virtual ~EventIndicator();

    void changeColumns( int columns );

    void enableColumn( int column, bool enable );

  protected:
    void drawContents( TQPainter * );

  private:
    int mColumns;
    Location mLocation;
    TQPixmap mPixmap;
    TQMemArray<bool> mEnabled;
};

class KOAlternateLabel : public QLabel
{
    Q_OBJECT
  public:
    KOAlternateLabel( const TQString &shortlabel, const TQString &longlabel,
                      const TQString &extensivelabel = TQString::null,
                      TQWidget *parent = 0, const char *name = 0 );
    ~KOAlternateLabel();

    virtual TQSize minimumSizeHint() const;

    enum TextType { Short = 0, Long = 1, Extensive = 2 };
    TextType largestFittingTextType() const;
    void setFixedType( TextType type );

  public slots:
    void useShortText();
    void useLongText();
    void useExtensiveText();
    void useDefaultText();

  protected:
    virtual void resizeEvent( TQResizeEvent * );
    virtual void squeezeTextToLabel();
    bool mTextTypeFixed;
    TQString mShortText, mLongText, mExtensiveText;
};

/**
  KOAgendaView is the agenda-like view used to display events in a single one or
  multi-day view.
*/
class KOAgendaView : public KOrg::AgendaView, public KCal::Calendar::Observer
{
    Q_OBJECT
  public:
    KOAgendaView( Calendar *cal,
                  CalendarView *calendarView,
                  TQWidget *parent = 0,
                  const char *name = 0,
                  bool isSideBySide = false );
    virtual ~KOAgendaView();

    /** Returns maximum number of days supported by the koagendaview */
    virtual int maxDatesHint();

    /** Returns number of currently shown dates. */
    virtual int currentDateCount();

    /** returns the currently selected events */
    virtual Incidence::List selectedIncidences();

    /** returns the currently selected events */
    virtual DateList selectedIncidenceDates();

    /** return the default start/end date/time for new events   */
    virtual bool eventDurationHint(TQDateTime &startDt, TQDateTime &endDt, bool &allDay);

    /** Remove all events from view */
    void clearView();

    KOrg::CalPrinterBase::PrintType printType();

    /** start-datetime of selection */
    TQDateTime selectionStart() { return mTimeSpanBegin; }
    /** end-datetime of selection */
    TQDateTime selectionEnd() { return mTimeSpanEnd; }
    /** returns true if selection is for whole day */
    bool selectedIsAllDay() { return mTimeSpanInAllDay; }
    /** make selected start/end invalid */
    void deleteSelectedDateTime();
    /** returns if only a single cell is selected, or a range of cells */
    bool selectedIsSingleCell();

    void setTypeAheadReceiver( TQObject * );

    KOAgenda* agenda() const { return mAgenda; }
    TQSplitter* splitter() const { return mSplitterAgenda; }
    TQFrame *dayLabels() const { return mDayLabels; }

    /* reimplmented from KCal::Calendar::Observer */
    void calendarIncidenceAdded( Incidence *incidence );
    void calendarIncidenceChanged( Incidence *incidence );
    void calendarIncidenceDeleted( Incidence *incidence );

  public slots:
    virtual void updateView();
    virtual void updateConfig();
    virtual void showDates( const TQDate &start, const TQDate &end );
    virtual void showIncidences( const Incidence::List &incidenceList, const TQDate &date );

    void insertIncidence( Incidence *incidence, const TQDate &curDate );
    void changeIncidenceDisplayAdded( Incidence *incidence );
    void changeIncidenceDisplay( Incidence *incidence, int mode );

    void clearSelection();

    void startDrag( Incidence * );

    void readSettings();
    void readSettings( KConfig * );
    void writeSettings( KConfig * );

    void setContentsPos( int y );

    void setExpandedButton( bool expanded );

    void finishTypeAhead();

    /** reschedule the todo  to the given x- and y- coordinates. Third parameter determines all-day (no time specified) */
    void slotTodoDropped( Todo *, const TQPoint &, bool );

    void enableAgendaUpdate( bool enable );
    void setIncidenceChanger( KOrg::IncidenceChangerBase *changer );

    void zoomInHorizontally( const TQDate& date=TQDate() );
    void zoomOutHorizontally( const TQDate& date=TQDate() );

    void zoomInVertically( );
    void zoomOutVertically( );

    void zoomView( const int delta, const TQPoint &pos,
      const Qt::Orientation orient=Qt::Horizontal );

    void clearTimeSpanSelection();

    void resourcesChanged();

  signals:
    void toggleExpand();
    void zoomViewHorizontally(const TQDate &, int count );

    void timeSpanSelectionChanged();

  protected:
    /** Fill agenda beginning with date startDate */
    void fillAgenda( const TQDate &startDate );

    /** Fill agenda using the current set value for the start date */
    void fillAgenda();

    void connectAgenda( KOAgenda*agenda, TQPopupMenu*popup, KOAgenda* otherAgenda );

    /** Create labels for the selected dates. */
    void createDayLabels( bool force );

    /**
      Set the masks on the agenda widgets indicating, which days are holidays.
    */
    void setHolidayMasks();

    void removeIncidence( Incidence * );
    /**
      Updates the event indicators after a certain incidence was modified or
      removed.
    */
    void updateEventIndicators();

    void updateTimeBarWidth();

    virtual void resizeEvent( TQResizeEvent *resizeEvent );

  protected slots:
    /** Update event belonging to agenda item */
    void updateEventDates( KOAgendaItem *item );
    /** update just the display of the given incidence, called by a single-shot timer */
    void doUpdateItem();

    void updateEventIndicatorTop( int newY );
    void updateEventIndicatorBottom( int newY );

    /** Updates data for selected timespan */
    void newTimeSpanSelected( const TQPoint &start, const TQPoint &end );
    /** Updates data for selected timespan for all day event*/
    void newTimeSpanSelectedAllDay( const TQPoint &start, const TQPoint &end );

    void updateDayLabelSizes();

  private:
    bool filterByResource( Incidence *incidence );
    void displayIncidence( Incidence *incidence );

  private:
    // view widgets
    TQFrame *mDayLabels;
    TQHBox *mDayLabelsFrame;
    TQBoxLayout *mLayoutDayLabels;
    TQPtrList<KOAlternateLabel> mDateDayLabels;
    TQFrame *mAllDayFrame;
    KOAgenda *mAllDayAgenda;
    KOAgenda *mAgenda;
    TimeLabels *mTimeLabels;
    TQWidget *mDummyAllDayLeft;
    TQSplitter *mSplitterAgenda;
    TQPushButton *mExpandButton;

    DateList mSelectedDates;  // List of dates to be displayed
    DateList mSaveSelectedDates; // Save the list of dates between updateViews
    int mViewType;

    KOEventPopupMenu *mAgendaPopup;
    KOEventPopupMenu *mAllDayAgendaPopup;

    EventIndicator *mEventIndicatorTop;
    EventIndicator *mEventIndicatorBottom;

    TQMemArray<int> mMinY;
    TQMemArray<int> mMaxY;

    TQMemArray<bool> mHolidayMask;

    TQPixmap mExpandedPixmap;
    TQPixmap mNotExpandedPixmap;

    TQDateTime mTimeSpanBegin;
    TQDateTime mTimeSpanEnd;
    bool mTimeSpanInAllDay;
    bool mAllowAgendaUpdate;

    Incidence *mUpdateItem;

    bool mIsSideBySide;
    bool mPendingChanges;

    // the current date is inserted into mSelectedDates in the constructor
    // however whe should only show events when setDates is called, otherwise
    // we see day view with current date for a few milisecs, then we see something else
    // because someone called setDates with the real dates that should be displayed.
    // Other solution would be not initializing mSelectedDates in the ctor, but that requires
    // lots of changes in koagenda.cpp and koagendaview.cpp
    bool mAreDatesInitialized;
};

#endif

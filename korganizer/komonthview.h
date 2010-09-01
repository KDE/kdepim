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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef _KOMONTHVIEW_H
#define _KOMONTHVIEW_H

#include <tqlistbox.h>
#include <tqptrvector.h>
#include <tqtooltip.h>
#include "koeventview.h"

class KNoScrollListBox;

class KOMonthCellToolTip : public QToolTip
{
  public:
    KOMonthCellToolTip (TQWidget* parent, Calendar *calendar, const TQDate &date, KNoScrollListBox* lv );

  protected:
    void maybeTip( const TQPoint & pos);

  private:
    Calendar *mCalendar;
    TQDate mDate;
    KNoScrollListBox* eventlist;
};


class KNoScrollListBox: public QListBox
{
    Q_OBJECT
  public:
    KNoScrollListBox(TQWidget *parent=0, const char *name=0);
    ~KNoScrollListBox() {}

    void setBackground( bool primary, bool workday );

  signals:
    void shiftDown();
    void shiftUp();
    void rightClick();

  protected slots:
    void keyPressEvent(TQKeyEvent *);
    void keyReleaseEvent(TQKeyEvent *);
    void mousePressEvent(TQMouseEvent *);
    void resizeEvent(TQResizeEvent *);
    void contentsMouseDoubleClickEvent( TQMouseEvent * e );

  private:
    bool mSqueezing;
};


class MonthViewItem: public QListBoxItem
{
  public:
    MonthViewItem( Incidence *, const TQDateTime &qd, const TQString & title );

    void setEvent(bool on) { mEvent = on; }
    void setTodo(bool on)  { mTodo  = on; }
    void setTodoDone(bool on) { mTodoDone = on; }
    void setRecur(bool on) { mRecur = on; }
    void setAlarm(bool on) { mAlarm = on; }
    void setReply(bool on) { mReply = on; }

    void setPalette(const TQPalette &p) { mPalette = p; }
    TQPalette palette() const { return mPalette; }

    Incidence *incidence() const { return mIncidence; }
    TQDateTime incidenceDateTime() { return mDateTime; }

    void setResourceColor( TQColor& color ) { mResourceColor = color; }
    TQColor &resourceColor() { return mResourceColor; }
  protected:
    virtual void paint(TQPainter *);
    virtual int height(const TQListBox *) const;
    virtual int width(const TQListBox *) const;
    //Color of the resource
    TQColor mResourceColor;
  private:
    bool mEvent;
    bool mTodo;
    bool mTodoDone;
    bool mRecur;
    bool mAlarm;
    bool mReply;

    TQPixmap mEventPixmap;
    TQPixmap mBirthdayPixmap;
    TQPixmap mAnniversaryPixmap;
    TQPixmap mTodoPixmap;
    TQPixmap mTodoDonePixmap;
    TQPixmap mAlarmPixmap;
    TQPixmap mRecurPixmap;
    TQPixmap mReplyPixmap;

    TQPalette mPalette;
    TQDateTime mDateTime;

    Incidence *mIncidence;
    TQColor catColor() const;
};


class KOMonthView;

/** This class represents one day in KOrganizer's month view.

@see KOMonthView
*/
class MonthViewCell : public QWidget
{
    Q_OBJECT
  public:
    class CreateItemVisitor;
    MonthViewCell( KOMonthView * );

    /** Sets the date of the cell */
    void setDate( const TQDate & );
    /** @return Date of cell */
    TQDate date() const;

    /** @return MonthView parent */
    KOMonthView *monthView() { return mMonthView; }

    /**
      Set this cell as primary if @p primary is true. A primary cell belongs
      to the current month. A non-primary cell belongs to the month before or
      after the current month.
      @param primary If true, the cell will be set as primary. Else it will be
      set as non-primary.
    */
    void setPrimary( bool primary );
    /**
      @return True if this cell is primary, else false.
    */
    bool isPrimary() const;

    /** Make this cell show as a holiday */
    void setHoliday( bool );
    /**
      Sets the holiday name to this cell. This will not call
      setHoliday( true ).
      @param name The name of the holiday.
    */
    void setHolidayString( const TQString &name );

    void updateCell();
    /** Adds an incidence to the cell.
        Sets the right text and icons for this incidence.
        @param incidence The incidence to be added.
        @param v A visitor which creates a MonthViewItem for the given @p incidence
        @param multiDay Specifies which day of a multi-day event is added to the
        cell. The first day is 0 (default).
    */
    void addIncidence( Incidence *incidence, MonthViewCell::CreateItemVisitor&v, int multiDay = 0 );
    /** Removes an incidence from the cell.
        @return True if successful, false if deletion failed
       (e.g. when given incidence doesn't exist in the cell.
    */
    void removeIncidence( Incidence * );

    void updateConfig();

    void enableScrollBars( bool );

    Incidence *selectedIncidence();
    TQDate selectedIncidenceDate();

    void deselect();

  signals:
    void defaultAction( Incidence * );
    /**
      Notify the view manager that we want to create a new event, so an editor
      will pop up.
      @param date The date of the event we want create.
    */
    void newEventSignal( ResourceCalendar *res,const TQString &subResource,
                         const TQDate &date );

  public slots:
    void select();

  protected:
    void setFrameWidth();
    void resizeEvent( TQResizeEvent * );

  protected slots:
    void defaultAction( TQListBoxItem * );
    void contextMenu( TQListBoxItem * );

  private:
    KOMonthView *mMonthView;

    TQDate mDate;
    bool mPrimary;
    bool mHoliday;
    TQString mHolidayString;

    TQLabel *mLabel;
    KNoScrollListBox *mItemList;

    bool isSelected;

    TQSize mLabelSize;
//    TQPalette mOriginalPalette;
    TQPalette mHolidayPalette;
    TQPalette mStandardPalette;
    TQPalette mTodayPalette;
};

/**
  The class KOMonthView represents the monthly view in KOrganizer.
  It holds several instances of the class MonthViewCell.

  @short KOMonthview represents the montly view in KOrganizer.
  @see KOBaseView, KODayListView, MonthViewCell
*/
class KOMonthView: public KOEventView
{
    Q_OBJECT
  public:
    KOMonthView(Calendar *cal, TQWidget *parent = 0, const char *name = 0 );
    ~KOMonthView();

    /** Returns maximum number of days supported by the komonthview */
    virtual int maxDatesHint();

    /** Returns number of currently shown dates. */
    virtual int currentDateCount();

    /** Returns the currently selected events */
    virtual Incidence::List selectedIncidences();

    /** Returns dates of the currently selected events */
    virtual DateList selectedIncidenceDates();

    virtual TQDateTime selectionStart();

    virtual TQDateTime selectionEnd();

    virtual bool eventDurationHint(TQDateTime &startDt, TQDateTime &endDt, bool &allDay);

  public slots:
    virtual void updateView();
    virtual void updateConfig();
    virtual void showDates(const TQDate &start, const TQDate &end);
    virtual void showIncidences( const Incidence::List &incidenceList, const TQDate &date );

    void changeIncidenceDisplay(Incidence *, int);
    void changeIncidenceDisplayAdded(Incidence *, MonthViewCell::CreateItemVisitor&);

    void clearSelection();

    void showEventContextMenu( Calendar *, Incidence *, const TQDate & );
    void showGeneralContextMenu();

    void setSelectedCell( MonthViewCell * );

  protected slots:
    void processSelectionChange();

  protected:
    void resizeEvent(TQResizeEvent *);

    void viewChanged();
    void updateDayLabels();

  private:
    void showLabel( bool show );
      
    class GetDateVisitor;
    int mDaysPerWeek;
    int mNumWeeks;
    int mNumCells;
    int mWeekStartDay;

    TQPtrVector<MonthViewCell> mCells;
    TQMap<TQDate,MonthViewCell *> mDateToCell;
    TQPtrVector<TQLabel> mDayLabels;

    bool mShortDayLabels;
    int mWidthLongDayLabel;

    TQDate mStartDate;
    TQDate mSelectedDate;

    MonthViewCell *mSelectedCell;

    KOEventPopupMenu *mEventContextMenu;
    TQLabel *mLabel;
};

#endif

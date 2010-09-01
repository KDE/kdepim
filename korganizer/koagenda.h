/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef KOAGENDA_H
#define KOAGENDA_H

#include <tqscrollview.h>
#include <tqtimer.h>
#include <tqguardedptr.h>
#include <libkcal/incidencebase.h>

#include "calendarview.h"

class TQPopupMenu;
class TQTime;
class TQLabel;
class KConfig;
class KOAgenda;
class KOAgendaItem;

namespace KOrg {
  class IncidenceChangerBase;
}
using namespace KOrg;

namespace KCal {
  class Event;
  class Todo;
  class Calendar;
}
using namespace KCal;

class MarcusBains : public TQFrame
{
  Q_OBJECT
  public:
    MarcusBains( KOAgenda *agenda = 0, const char *name = 0 );
    void updateLocationRecalc( bool recalculate = false );
    virtual ~MarcusBains();

  public slots:
    void updateLocation();

  private:
    int todayColumn();
    TQTimer *minutes;
    TQLabel *mTimeBox;
    KOAgenda *agenda;
    TQTime mOldTime;
    int mOldToday;
};

class KOAgenda : public TQScrollView
{
  Q_OBJECT
  public:
    KOAgenda ( int columns, int rows, int columnSize, CalendarView *calendarView,
               TQWidget *parent=0, const char *name = 0, WFlags f = 0 );

    KOAgenda ( int columns, CalendarView *calendarView, TQWidget *parent = 0,
               const char *name = 0, WFlags f = 0 );
    virtual ~KOAgenda();

    Incidence *selectedIncidence() const;
    TQDate selectedIncidenceDate() const;
    /**
     * Returns the uid of the last incidence that was selected. This
     * persists across reloads and clear, so that if the same uid
     * reappears, it can be reselected. */
    const TQString lastSelectedUid() const;

    virtual bool eventFilter ( TQObject *, TQEvent * );

    TQPoint contentsToGrid ( const TQPoint &pos ) const;
    TQPoint gridToContents ( const TQPoint &gpos ) const;

    int timeToY ( const TQTime &time );
    TQTime gyToTime ( int y );

    TQMemArray<int> minContentsY();
    TQMemArray<int> maxContentsY();

    int visibleContentsYMin();
    int visibleContentsYMax();

    void setStartTime( const TQTime &startHour );

    KOAgendaItem *insertItem ( Incidence *incidence, const TQDate &qd, int X, int YTop,
                               int YBottom, int itemPos, int itemCount );
    KOAgendaItem *insertAllDayItem ( Incidence *event, const TQDate &qd, int XBegin,
                                     int XEnd );
    void insertMultiItem ( Event *event, const TQDate &qd, int XBegin, int XEnd,
                           int YTop, int YBottom );

    /** remove an event and all its multi-items from the agenda.
     *  This function removes the items from the view, but doesn't delete them immediately.
     *  Instead, they are queued in mItemsToDelete and later deleted by
     *  the slot deleteItemsToDelete() (called by TQTimer::singleShot ) */
    void removeIncidence( Incidence *incidence );

    void changeColumns( int columns );

    int columns() { return mColumns; }
    int rows() { return mRows; }

    double gridSpacingX() const { return mGridSpacingX; }
    double gridSpacingY() const { return mGridSpacingY; }

//    virtual TQSizePolicy sizePolicy() const;

    void clear();

    /** Calculates the minimum width */
    virtual int minimumWidth() const;
    /** Update configuration from preference settings */
    void updateConfig();

    void checkScrollBoundaries();

    void setHolidayMask( TQMemArray<bool> * );

    void setDateList( const DateList &selectedDates );
    DateList dateList() const;

    void setTypeAheadReceiver( TQObject * );
    TQObject *typeAheadReceiver() const;
    void finishTypeAhead();

    void setCalendar( Calendar*cal ) { mCalendar = cal; }
    void setIncidenceChanger( IncidenceChangerBase *changer ) { mChanger = changer; }

  public slots:
    void scrollUp();
    void scrollDown();

    void checkScrollBoundaries( int );

    /** Deselect selected items. This function does not emit any signals. */
    void deselectItem();

    void clearSelection();

    /**
      Select item. If the argument is 0, the currently selected item gets
      deselected. This function emits the itemSelected(bool) signal to inform
      about selection/deselection of events.
    */
    void selectItem( KOAgendaItem * );
    /**
      Select the item associated with a given uid. Linear search, use carefully.
    */
    void selectItemByUID( const TQString& uid );
    bool removeAgendaItem( KOAgendaItem *item );
    void showAgendaItem( KOAgendaItem *item );

  signals:
    void newEventSignal( ResourceCalendar *res, const TQString &subResource );
    void newTimeSpanSignal( const TQPoint &, const TQPoint & );
    void newStartSelectSignal();

    void showIncidenceSignal( Incidence *, const TQDate & );
    void editIncidenceSignal( Incidence *, const TQDate & );
    void deleteIncidenceSignal( Incidence * );
    void showIncidencePopupSignal( Calendar *, Incidence *, const TQDate &);
    void showNewEventPopupSignal();

    void itemModified( KOAgendaItem *item );
    void incidenceSelected( Incidence *, const TQDate & );
    void startMultiModify( const TQString & );
    void endMultiModify();

    void lowerYChanged( int );
    void upperYChanged( int );

    void startDragSignal(Incidence *);
    void droppedToDo( Todo*todo, const TQPoint &gpos, bool allDay );

    void enableAgendaUpdate( bool enable );
    void zoomView( const int delta, const TQPoint &pos, const Qt::Orientation );

    void mousePosSignal(const TQPoint &pos);
    void enterAgenda();
    void leaveAgenda();

    void gridSpacingYChanged( double );

  private:
    enum MouseActionType { NOP, MOVE, SELECT,
                           RESIZETOP, RESIZEBOTTOM, RESIZELEFT, RESIZERIGHT };

  protected:
    void drawContents( TQPainter *p, int cx, int cy, int cw, int ch );
    int columnWidth( int column );
    virtual void resizeEvent ( TQResizeEvent * );

    /** Handles mouse events. Called from eventFilter */
    virtual bool eventFilter_mouse ( TQObject *, TQMouseEvent * );
#ifndef QT_NO_WHEELEVENT
    /** Handles mousewheel events. Called from eventFilter */
    virtual bool eventFilter_wheel ( TQObject *, TQWheelEvent * );
#endif
    /** Handles key events. Called from eventFilter */
    virtual bool eventFilter_key ( TQObject *, TQKeyEvent * );

    /** Handles drag and drop events. Called from eventFilter */
    virtual bool eventFilter_drag( TQObject *, TQDropEvent * );

    /** returns RESIZELEFT if pos is near the lower edge of the action item,
      RESIZERIGHT if pos is near the higher edge, and MOVE otherwise.
      If --reverse is used, RESIZELEFT still means resizing the beginning of
      the event, although that means moving to the right!
      horizontal is the same as mAllDayAgenda.
    */
    MouseActionType isInResizeArea( bool horizontal, const TQPoint &pos, KOAgendaItem *item );
    /** Return whether the cell specified by the grid point belongs to the current select
    */
    bool ptInSelection( TQPoint gpos ) const;


    /** Start selecting time span. */
    void startSelectAction( const TQPoint &viewportPos );

    /** Select time span. */
    void performSelectAction( const TQPoint &viewportPos );

    /** Emd selecting time span. */
    void endSelectAction( const TQPoint &viewportPos );

    /** Start moving/resizing agenda item */
    void startItemAction(const TQPoint& viewportPos);

    /** Move/resize agenda item */
    void performItemAction(const TQPoint& viewportPos);

    /** End moving/resizing agenda item */
    void endItemAction();

    /** Set cursor, when no item action is in progress */
    void setNoActionCursor( KOAgendaItem *moveItem, const TQPoint &viewportPos );
    /** Sets the cursor according to the given action type. If acting==true,
      the corresponding action is running (i.e. the item is really moved). If
      acting==false the cursor should just indicate that the corresponding action
      is possible */
    void setActionCursor( int actionType, bool acting=false );

    /** calculate the width of the column subcells of the given item */
    double calcSubCellWidth( KOAgendaItem *item );
    /** Move and resize the given item to the correct position */
    void placeAgendaItem( KOAgendaItem *item, double subCellWidth );
    /** Place agenda item in agenda and adjust other cells if necessary */
    void placeSubCells( KOAgendaItem *placeItem );
    /** Place the agenda item at the correct position (ignoring conflicting items) */
    void adjustItemPosition( KOAgendaItem *item );

    /** Process the keyevent, including the ignored keyevents of eventwidgets.
     * Implements pgup/pgdn and cursor key navigation in the view.
     */
    void keyPressEvent( TQKeyEvent * );

    void calculateWorkingHours();

    virtual void contentsMousePressEvent ( TQMouseEvent * );

    void emitNewEventForSelection();

  protected slots:
    /** delete the items that are queued for deletion */
    void deleteItemsToDelete();
    /** Resizes all the child elements after the size of the agenda
        changed. This is needed because Qt seems to have a bug when
        the resizeEvent of one of the widgets in a splitter takes a
        lot of time / does a lot of resizes.... see bug 80114 */
    void resizeAllContents();

  private:
    void init();
    void marcus_bains();
    bool mAllDayMode;

    // We need the calendar for drag'n'drop and for paint the ResourceColor
    Calendar *mCalendar;

    // Width and height of agenda cells. mDesiredGridSpacingY is the height
    // set in the config. The actual height might be larger since otherwise
    // more than 24 hours might be displayed.
    double mGridSpacingX;
    double mGridSpacingY;
    double mDesiredGridSpacingY;

    // size of border, where mouse action will resize the KOAgendaItem
    int mResizeBorderWidth;

    // size of border, where mouse mve will cause a scroll of the agenda
    int mScrollBorderWidth;
    int mScrollDelay;
    int mScrollOffset;

    TQTimer mScrollUpTimer;
    TQTimer mScrollDownTimer;

    // Number of Columns/Rows of agenda grid
    int mColumns;
    int mRows;

    // Cells to store Move and Resize coordiantes while performing the action
    TQPoint mStartCell;
    TQPoint mEndCell;

    // Working Hour coordiantes
    bool mWorkingHoursEnable;
    TQMemArray<bool> *mHolidayMask;
    int mWorkingHoursYTop;
    int mWorkingHoursYBottom;

    // Selection
    bool mHasSelection;
    TQPoint mSelectionStartPoint;
    TQPoint mSelectionStartCell;
    TQPoint mSelectionEndCell;

    // List of dates to be displayed
    DateList mSelectedDates;

    // The KOAgendaItem, which has been right-clicked last
    TQGuardedPtr<KOAgendaItem> mClickedItem;

    // The KOAgendaItem, which is being moved/resized
    TQGuardedPtr<KOAgendaItem> mActionItem;
    QPair<ResourceCalendar *, TQString> mResPair;

    // Currently selected item
    TQGuardedPtr<KOAgendaItem> mSelectedItem;
    // Uid of the last selected item. Used for reselecting in situations
    // where the selected item points to a no longer valid incidence, for
    // example during resource reload.
    TQString mSelectedUid;

    // The Marcus Bains Line widget.
    MarcusBains *mMarcusBains;

    MouseActionType mActionType;

    bool mItemMoved;

    // List of all Items contained in agenda
    TQPtrList<KOAgendaItem> mItems;
    TQPtrList<KOAgendaItem> mItemsToDelete;

    TQPopupMenu *mItemPopup; // Right mouse button popup menu for KOAgendaItems

    int mOldLowerScrollValue;
    int mOldUpperScrollValue;

    bool mTypeAhead;
    TQObject *mTypeAheadReceiver;
    TQPtrList<TQEvent> mTypeAheadEvents;

    bool mReturnPressed;
    KOrg::IncidenceChangerBase *mChanger;

    CalendarView *mCalendarView;
};

#endif // KOAGENDA_H

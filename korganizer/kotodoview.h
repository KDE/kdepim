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
#ifndef KOTODOVIEW_H
#define KOTODOVIEW_H

#include <tqmap.h>
#include <tqtooltip.h>

#include <klistview.h>

#include <libkcal/todo.h>
#include <korganizer/baseview.h>

class TQDragEnterEvent;
class TQDragMoveEvent;
class TQDragLeaveEvent;
class TQDropEvent;
class TQPopupMenu;

class KOTodoListView;
class KOTodoViewItem;
class KDatePickerPopup;

class DocPrefs;

namespace KPIM {
  class ClickLineEdit;
}
namespace KCal {
class Incidence;
class Calendar;
}
using namespace KCal;
using namespace KOrg;

class KOTodoListViewToolTip : public QToolTip
{
  public:
    KOTodoListViewToolTip( TQWidget *parent, Calendar *calendar, KOTodoListView *lv );

  protected:
    void maybeTip( const TQPoint &pos );

  private:
    Calendar *mCalendar;
    KOTodoListView *todolist;
};


class KOTodoListView : public KListView
{
    Q_OBJECT
  public:
    KOTodoListView( TQWidget *parent = 0, const char *name = 0 );
    ~KOTodoListView();

    void setCalendar( Calendar * );

    void setIncidenceChanger( IncidenceChangerBase *changer ) { mChanger = changer; }

  protected:
    virtual bool event( TQEvent * );

    void contentsDragEnterEvent( TQDragEnterEvent * );
    void contentsDragMoveEvent( TQDragMoveEvent * );
    void contentsDragLeaveEvent( TQDragLeaveEvent * );
    void contentsDropEvent( TQDropEvent * );

    void contentsMousePressEvent( TQMouseEvent * );
    void contentsMouseMoveEvent( TQMouseEvent * );
    void contentsMouseReleaseEvent( TQMouseEvent * );
    void contentsMouseDoubleClickEvent( TQMouseEvent * );

  private:
    Calendar *mCalendar;
    KOrg::IncidenceChangerBase *mChanger;

    TQPoint mPressPos;
    bool mMousePressed;
    TQListViewItem *mOldCurrent;
};


/**
  This class provides a multi-column list view of todo events.

  @short multi-column list view of todo events.
  @author Cornelius Schumacher <schumacher@kde.org>
*/
class KOTodoView : public KOrg::BaseView
{
    Q_OBJECT
  public:
    KOTodoView( Calendar *cal, TQWidget *parent = 0, const char *name = 0 );
    ~KOTodoView();

    void setCalendar( Calendar * );

    Incidence::List selectedIncidences();
    Todo::List selectedTodos();

    DateList selectedIncidenceDates() { return DateList(); }

    /** Return number of shown dates. TodoView does not show dates, */
    int currentDateCount() { return 0; }

    CalPrinterBase::PrintType printType();

    void setDocumentId( const TQString & );

    void saveLayout( KConfig *config, const TQString &group ) const;
    void restoreLayout( KConfig *config, const TQString &group );
    /** Create a popup menu to set categories */
    TQPopupMenu *getCategoryPopupMenu( KOTodoViewItem *todoItem );
    void setIncidenceChanger( IncidenceChangerBase *changer );

  public slots:
    void updateView();
    void updateConfig();

    void changeIncidenceDisplay( Incidence *, int );

    void showDates( const TQDate &start, const TQDate &end );
    void showIncidences( const Incidence::List &incidenceList, const TQDate &date );

    void clearSelection();

    void editItem( TQListViewItem *item, const TQPoint &, int );
    void editItem( TQListViewItem *item );
    void showItem( TQListViewItem *item, const TQPoint &, int );
    void showItem( TQListViewItem *item );
    void popupMenu( TQListViewItem *item, const TQPoint &, int );
    void newTodo();
    void newSubTodo();
    void showTodo();
    void editTodo();
    void printTodo();
    void deleteTodo();

    void setNewPercentage( KOTodoViewItem *item, int percentage );

    void setNewPriority( int );
    void setNewPercentage( int );
    void setNewDate( TQDate );
    void copyTodoToDate( TQDate );
    void changedCategories( int );

    void purgeCompleted();

    void itemStateChanged( TQListViewItem * );

    void setNewPercentageDelayed( KOTodoViewItem *item, int percentage );
    void processDelayedNewPercentage();

  signals:
    void unSubTodoSignal();
    void unAllSubTodoSignal();
    void purgeCompletedSignal();
    void configChanged();

  protected slots:
    void processSelectionChange();
    void addQuickTodo();
    void removeTodoItems();

  private:
    /*
     * the TodoEditor approach is rather unscaling in the long
     * run.
     * Korganizer keeps it in memory and we need to update
     * 1. make KOTodoViewItem a TQObject again?
     * 2. add a public method for setting one todo modified?
     * 3. add a private method for setting a todo modified + friend here?
     *  -- zecke 2002-07-08
     */
    friend class KOTodoViewItem;

    TQMap<Todo *,KOTodoViewItem *>::ConstIterator insertTodoItem( Todo *todo );
    bool scheduleRemoveTodoItem( KOTodoViewItem *todoItem );
    void restoreItemState( TQListViewItem * );

    KOTodoListView *mTodoListView;
    TQPopupMenu *mItemPopupMenu;
    TQPopupMenu *mPopupMenu;
    TQPopupMenu *mPriorityPopupMenu;
    TQPopupMenu *mPercentageCompletedPopupMenu;
    TQPopupMenu *mCategoryPopupMenu;
    KDatePickerPopup *mMovePopupMenu;
    KDatePickerPopup *mCopyPopupMenu;

    TQMap<int, int> mPercentage;
    TQMap<int, int> mPriority;
    TQMap<int, TQString> mCategory;

    KOTodoViewItem *mActiveItem;

    TQMap<Todo *,KOTodoViewItem *> mTodoMap;
    TQPtrList<KOTodoViewItem> mItemsToDelete;
    TQValueList< QPair<KOTodoViewItem *, int> > mPercentChangedMap;

    DocPrefs *mDocPrefs;
    TQString mCurrentDoc;
    KPIM::ClickLineEdit *mQuickAdd;

  public:
    enum {
      eSummaryColumn = 0,
      eRecurColumn = 1,
      ePriorityColumn = 2,
      ePercentColumn = 3,
      eDueDateColumn = 4,
      eCategoriesColumn = 5,
      eFolderColumn = 6
    };
    enum {
      ePopupEdit = 1300,
      ePopupDelete = 1301,
      ePopupMoveTo = 1302,
      ePopupCopyTo = 1303,
      ePopupUnSubTodo = 1304,
      ePopupUnAllSubTodo = 1305
    };

};

#endif

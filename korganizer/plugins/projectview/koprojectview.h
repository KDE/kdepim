/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KOPROJECTVIEW_H
#define KOPROJECTVIEW_H
/* $Id$ */

#include <tqptrlist.h>
#include <tqfontmetrics.h>

#include <tqmap.h>

#include <libkcal/calendar.h>
#include <libkcal/event.h>

#include "korganizer/baseview.h"
#include "KGanttItem.h"

class KGantt;
class TQLineEdit;
class TQFont;
class TQLabel;
class TQPopupMenu;
class TQListBox;
class TQStrList;
class TQListView;

/**
  This class provides an item of the project view. It is a xQTask with
  an additional Event attribute.
*/
class KOProjectViewItem : public KGanttItem {
  public:
    KOProjectViewItem(Todo *,KGanttItem* parentTask, const TQString& text,
                      const TQDateTime& start, const TQDateTime& end);
    ~KOProjectViewItem();

    Todo *event();

  private:
    Todo *mEvent;
};


/**
 * This class provides a Gantt-like project view on todo items
 *
 * @short project view on todo items.
 * @author Cornelius Schumacher <schumacher@kde.org>
 */
class KOProjectView : public KOrg::BaseView
{
    Q_OBJECT
  public:
    KOProjectView(Calendar *, TQWidget* parent=0, const char* name=0 );
    ~KOProjectView() {}

    Incidence::List selectedIncidences();
    DateList selectedDates();

    /** Return number of shown dates. */
    int currentDateCount() { return 0; }

    void readSettings();
    void writeSettings(KConfig *);

  public slots:
    void updateView();
    void updateConfig();

    void changeIncidenceDisplay(Incidence *, int);

    void showDates(const TQDate &start, const TQDate &end);
    void showIncidences( const Incidence::List &incidenceList );

/*
    void editItem(TQListViewItem *item);
    void showItem(TQListViewItem *item);
    void popupMenu(TQListViewItem *item,const TQPoint &,int);
    void newTodo();
    void newSubTodo();
    void itemClicked(TQListViewItem *);
*/

  protected slots:
    void showModeMenu();
    void zoomIn();
    void zoomOut();
    void taskChanged(KGanttItem *task,KGanttItem::Change change);

  private:
    void createMainTask();
    KGanttItem *createTask(KGanttItem *,Todo *);

    KGantt *mGantt;
    KGanttItem *mMainTask;

    TQMap<Todo *,KGanttItem *>::ConstIterator insertTodoItem(Todo *todo);

    TQMap<Todo *,KGanttItem *> mTodoMap;
};

#endif

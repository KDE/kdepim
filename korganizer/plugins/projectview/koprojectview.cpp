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

#include <tqlayout.h>
#include <tqheader.h>
#include <tqpushbutton.h>
#include <tqfont.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqlistbox.h>
#include <tqpopupmenu.h>
#include <tqstrlist.h>
#include <tqlistview.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <libkcal/vcaldrag.h>

#include "KGantt.h"

#include "koprojectview.h"

using namespace KOrg;

KOProjectViewItem::KOProjectViewItem(Todo *event,KGanttItem* parentTask,
                                     const TQString& text,
                                     const TQDateTime& start,
                                     const TQDateTime& end) :
  KGanttItem(parentTask,text,start,end)
{
  mEvent = event;
}

KOProjectViewItem::~KOProjectViewItem()
{
}

Todo *KOProjectViewItem::event()
{
  return mEvent;
}


KOProjectView::KOProjectView(Calendar *calendar,TQWidget* parent,
                             const char* name) :
  KOrg::BaseView(calendar,parent,name)
{
  TQBoxLayout *topLayout = new TQVBoxLayout(this);

  TQBoxLayout *topBar = new TQHBoxLayout;
  topLayout->addLayout(topBar);

  TQLabel *title = new TQLabel(i18n("Project View"),this);
  title->setFrameStyle(TQFrame::Panel|TQFrame::Raised);
  topBar->addWidget(title,1);

  TQPushButton *zoomIn = new TQPushButton(i18n("Zoom In"),this);
  topBar->addWidget(zoomIn,0);
  connect(zoomIn,TQT_SIGNAL(clicked()),TQT_SLOT(zoomIn()));

  TQPushButton *zoomOut = new TQPushButton(i18n("Zoom Out"),this);
  topBar->addWidget(zoomOut,0);
  connect(zoomOut,TQT_SIGNAL(clicked()),TQT_SLOT(zoomOut()));

  TQPushButton *menuButton = new TQPushButton(i18n("Select Mode"),this);
  topBar->addWidget(menuButton,0);
  connect(menuButton,TQT_SIGNAL(clicked()),TQT_SLOT(showModeMenu()));

  createMainTask();

  mGantt = new KGantt(mMainTask,this);
  topLayout->addWidget(mGantt,1);

#if 0
  mGantt->addHoliday(2000, 10, 3);
  mGantt->addHoliday(2001, 10, 3);
  mGantt->addHoliday(2000, 12, 24);

  for(int i=1; i<7; ++i)
    mGantt->addHoliday(2001, 1, i);
#endif
}

void KOProjectView::createMainTask()
{
  mMainTask = new KGanttItem(0,i18n("main task"),
                         TQDateTime::currentDateTime(),
                         TQDateTime::currentDateTime());
  mMainTask->setMode(KGanttItem::Rubberband);
  mMainTask->setStyle(KGanttItem::DrawBorder | KGanttItem::DrawText |
                      KGanttItem::DrawHandle);
}

void KOProjectView::readSettings()
{
  kdDebug(5850) << "KOProjectView::readSettings()" << endl;

  //KConfig *config = kapp->config();
  KConfig config( "korganizerrc", true, false); // Open read-only, no kdeglobals
  config.setGroup("Views");

  TQValueList<int> sizes = config.readIntListEntry("Separator ProjectView");
  if (sizes.count() == 2) {
    mGantt->splitter()->setSizes(sizes);
  }
}

void KOProjectView::writeSettings(KConfig *config)
{
  kdDebug(5850) << "KOProjectView::writeSettings()" << endl;

  config->setGroup("Views");

  TQValueList<int> list = mGantt->splitter()->sizes();
  config->writeEntry("Separator ProjectView",list);
}


void KOProjectView::updateView()
{
  kdDebug(5850) << "KOProjectView::updateView()" << endl;

  // Clear Gantt view
  TQPtrList<KGanttItem> subs = mMainTask->getSubItems();
  KGanttItem *t=subs.first();
  while(t) {
    KGanttItem *nt=subs.next();
    delete t;
    t = nt;
  }

#if 0
  KGanttItem* t1 = new KGanttItem(mGantt->getMainTask(), "task 1, no subtasks",
                             TQDateTime::currentDateTime().addDays(10),
                             TQDateTime::currentDateTime().addDays(20) );

  KGanttItem* t2 = new KGanttItem(mGantt->getMainTask(), "task 2, subtasks, no rubberband",
                             TQDateTime(TQDate(2000,10,1)),
                             TQDateTime(TQDate(2000,10,31)) );
#endif

  Todo::List todoList = calendar()->todos();

/*
  kdDebug(5850) << "KOProjectView::updateView(): Todo List:" << endl;
  Event *t;
  for(t = todoList.first(); t; t = todoList.next()) {
    kdDebug(5850) << "  " << t->getSummary() << endl;

    if (t->getRelatedTo()) {
      kdDebug(5850) << "      (related to " << t->getRelatedTo()->getSummary() << ")" << endl;
    }

    TQPtrList<Event> l = t->getRelations();
    Event *c;
    for(c=l.first();c;c=l.next()) {
      kdDebug(5850) << "    - relation: " << c->getSummary() << endl;
    }
  }
*/

  // Put for each Event a KOProjectViewItem in the list view. Don't rely on a
  // specific order of events. That means that we have to generate parent items
  // recursively for proper hierarchical display of Todos.
  mTodoMap.clear();
  Todo::List::ConstIterator it;
  for( it = todoList.begin(); it != todoList.end(); ++it ) {
    if ( !mTodoMap.contains( *it ) ) {
      insertTodoItem( *it );
    }
  }
}

TQMap<Todo *,KGanttItem *>::ConstIterator
    KOProjectView::insertTodoItem(Todo *todo)
{
//  kdDebug(5850) << "KOProjectView::insertTodoItem(): " << todo->getSummary() << endl;
  Todo *relatedTodo = dynamic_cast<Todo *>(todo->relatedTo());
  if (relatedTodo) {
//    kdDebug(5850) << "  has Related" << endl;
    TQMap<Todo *,KGanttItem *>::ConstIterator itemIterator;
    itemIterator = mTodoMap.find(relatedTodo);
    if (itemIterator == mTodoMap.end()) {
//      kdDebug(5850) << "    related not yet in list" << endl;
      itemIterator = insertTodoItem (relatedTodo);
    }
    KGanttItem *task = createTask(*itemIterator,todo);
    return mTodoMap.insert(todo,task);
  } else {
//    kdDebug(5850) << "  no Related" << endl;
    KGanttItem *task = createTask(mMainTask,todo);
    return mTodoMap.insert(todo,task);
  }
}

KGanttItem *KOProjectView::createTask(KGanttItem *parent,Todo *todo)
{
  TQDateTime startDt;
  TQDateTime endDt;

  if (todo->hasStartDate() && !todo->hasDueDate()) {
    // start date but no due date
    startDt = todo->dtStart();
    endDt = TQDateTime::currentDateTime();
  } else if (!todo->hasStartDate() && todo->hasDueDate()) {
    // due date but no start date
    startDt = todo->dtDue();
    endDt = todo->dtDue();
  } else if (!todo->hasStartDate() || !todo->hasDueDate()) {
    startDt = TQDateTime::currentDateTime();
    endDt = TQDateTime::currentDateTime();
  } else {
    startDt = todo->dtStart();
    endDt = todo->dtDue();
  }

  KGanttItem *task = new KOProjectViewItem(todo,parent,todo->summary(),startDt,
                                       endDt);
  connect(task,TQT_SIGNAL(changed(KGanttItem*, KGanttItem::Change)),
          TQT_SLOT(taskChanged(KGanttItem*,KGanttItem::Change)));
  if (todo->relations().count() > 0) {
    task->setBrush(TQBrush(TQColor(240,240,240), TQBrush::Dense4Pattern));
  }

  return task;
}

void KOProjectView::updateConfig()
{
  // FIXME: to be implemented.
}

Incidence::List KOProjectView::selectedIncidences()
{
  Incidence::List selected;

/*
  KOProjectViewItem *item = (KOProjectViewItem *)(mTodoListView->selectedItem());
  if (item) selected.append(item->event());
*/

  return selected;
}

DateList KOProjectView::selectedDates()
{
  DateList selected;
  return selected;
}

void KOProjectView::changeIncidenceDisplay(Incidence *, int)
{
  updateView();
}

void KOProjectView::showDates(const TQDate &, const TQDate &)
{
  updateView();
}

void KOProjectView::showIncidences( const Incidence::List & )
{
  kdDebug(5850) << "KOProjectView::showIncidences( const Incidence::List & ): not yet implemented" << endl;
}

#if 0
void KOProjectView::editItem(TQListViewItem *item)
{
  emit editIncidenceSignal(((KOProjectViewItem *)item)->event());
}

void KOProjectView::showItem(TQListViewItem *item)
{
  emit showIncidenceSignal(((KOProjectViewItem *)item)->event());
}

void KOProjectView::popupMenu(TQListViewItem *item,const TQPoint &,int)
{
  mActiveItem = (KOProjectViewItem *)item;
  if (item) mItemPopupMenu->popup(TQCursor::pos());
  else mPopupMenu->popup(TQCursor::pos());
}

void KOProjectView::newTodo()
{
  emit newTodoSignal();
}

void KOProjectView::newSubTodo()
{
  if (mActiveItem) {
    emit newSubTodoSignal(mActiveItem->event());
  }
}

void KOProjectView::itemClicked(TQListViewItem *item)
{
  if (!item) return;

  KOProjectViewItem *todoItem = (KOProjectViewItem *)item;
  int completed = todoItem->event()->isCompleted();  // Completed or not?

  if (todoItem->isOn()) {
    if (!completed) {
      todoItem->event()->setCompleted(true);
    }
  } else {
    if (completed) {
      todoItem->event()->setCompleted(false);
    }
  }
}
#endif

void KOProjectView::showModeMenu()
{
  mGantt->menu()->popup(TQCursor::pos());
}

void KOProjectView::taskChanged(KGanttItem *task,KGanttItem::Change change)
{
  if (task == mMainTask) return;

  KOProjectViewItem *item = (KOProjectViewItem *)task;

  if (change == KGanttItem::StartChanged) {
    item->event()->setDtStart(task->getStart());
  } else if (change == KGanttItem::EndChanged) {
    item->event()->setDtDue(task->getEnd());
  }
}

void KOProjectView::zoomIn()
{
  mGantt->zoom(2);
}

void KOProjectView::zoomOut()
{
  mGantt->zoom(0.5);
}

#include "koprojectview.moc"

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

#include <tqlayout.h>
#include <tqheader.h>
#include <tqcursor.h>
#include <tqlabel.h>
#include <tqtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <libkcal/calhelper.h>
#include <libkcal/icaldrag.h>
#include <libkcal/vcaldrag.h>
#include <libkcal/dndfactory.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/calfilter.h>
#include <libkcal/incidenceformatter.h>

#include <libkdepim/clicklineedit.h>
#include <libkdepim/kdatepickerpopup.h>

#include <libemailfunctions/email.h>

#include "docprefs.h"

#include "koincidencetooltip.h"
#include "kodialogmanager.h"
#include "kotodoview.h"
#include "koprefs.h"
#include "koglobals.h"
using namespace KOrg;
#include "kotodoviewitem.h"
#include "kotodoview.moc"
#ifndef KORG_NOPRINTER
#include "kocorehelper.h"
#include "calprinter.h"
#endif

KOTodoListViewToolTip::KOTodoListViewToolTip (TQWidget *parent,
                                              Calendar *calendar,
                                              KOTodoListView *lv )
  :TQToolTip(parent), mCalendar( calendar )
{
  todolist=lv;
}

void KOTodoListViewToolTip::maybeTip( const TQPoint & pos)
{
  TQRect r;
  int headerPos;
  int col=todolist->header()->sectionAt(todolist->contentsX() + pos.x());
  KOTodoViewItem *i=(KOTodoViewItem *)todolist->itemAt(pos);

  /* Check wether a tooltip is necessary. */
  if( i && KOPrefs::instance()->mEnableToolTips )
  {

    /* Calculate the rectangle. */
    r=todolist->itemRect(i);
    headerPos = todolist->header()->sectionPos(col)-todolist->contentsX();
    r.setLeft( (headerPos < 0 ? 0 : headerPos) );
    r.setRight(headerPos + todolist->header()->sectionSize(col));

    /* Show the tip */
    TQString tipText( IncidenceFormatter::toolTipStr( mCalendar, i->todo(), TQDate(), true ) );;
    if ( !tipText.isEmpty() ) {
      tip(r, tipText);
    }
  }

}



KOTodoListView::KOTodoListView( TQWidget *parent, const char *name )
  : KListView( parent, name ), mCalendar( 0 ), mChanger( 0 )
{
  mOldCurrent = 0;
  mMousePressed = false;
}

KOTodoListView::~KOTodoListView()
{
}

void KOTodoListView::setCalendar( Calendar *cal )
{
  mCalendar = cal;
  setAcceptDrops( mCalendar );
  viewport()->setAcceptDrops( mCalendar );
}

bool KOTodoListView::event(TQEvent *e)
{
  int tmp=0;
  KOTodoViewItem *i;

  /* Checks for an ApplicationPaletteChange event and updates
   * the small Progress bars to make therm have the right colors. */
  if(e->type()==TQEvent::ApplicationPaletteChange)
  {

    KListView::event(e);
    i=(KOTodoViewItem *)itemAtIndex(tmp);

    while(i!=0)
    {
      i->construct();
      tmp++;
      i=(KOTodoViewItem *)itemAtIndex(tmp);
    }

  }

  return (KListView::event(e) || e->type()==TQEvent::ApplicationPaletteChange);
}

void KOTodoListView::contentsDragEnterEvent(TQDragEnterEvent *e)
{
#ifndef KORG_NODND
//  kdDebug(5850) << "KOTodoListView::contentsDragEnterEvent" << endl;
  if ( !ICalDrag::canDecode( e ) && !VCalDrag::canDecode( e ) &&
       !TQTextDrag::canDecode( e ) ) {
    e->ignore();
    return;
  }

  mOldCurrent = currentItem();
#endif
}

void KOTodoListView::contentsDragMoveEvent(TQDragMoveEvent *e)
{
#ifndef KORG_NODND
//  kdDebug(5850) << "KOTodoListView::contentsDragMoveEvent" << endl;

  if ( !ICalDrag::canDecode( e ) && !VCalDrag::canDecode( e ) &&
       !TQTextDrag::canDecode( e ) ) {
    e->ignore();
    return;
  }

  e->accept();
#endif
}

void KOTodoListView::contentsDragLeaveEvent( TQDragLeaveEvent * )
{
#ifndef KORG_NODND
//  kdDebug(5850) << "KOTodoListView::contentsDragLeaveEvent" << endl;

  setCurrentItem(mOldCurrent);
  setSelected(mOldCurrent,true);
#endif
}

void KOTodoListView::contentsDropEvent( TQDropEvent *e )
{
#ifndef KORG_NODND
  kdDebug(5850) << "KOTodoListView::contentsDropEvent" << endl;

  if ( !mCalendar || !mChanger ||
       ( !ICalDrag::canDecode( e ) && !VCalDrag::canDecode( e ) &&
         !TQTextDrag::canDecode( e ) ) ) {
    e->ignore();
    return;
  }

  DndFactory factory( mCalendar );
  Todo *todo = factory.createDropTodo(e);

  if ( todo ) {
    e->acceptAction();

    KOTodoViewItem *destination =
        (KOTodoViewItem *)itemAt(contentsToViewport(e->pos()));
    Todo *destinationEvent = 0;
    if (destination) destinationEvent = destination->todo();

    Todo *existingTodo = mCalendar->todo(todo->uid());

    if( existingTodo ) {
       kdDebug(5850) << "Drop existing Todo " << existingTodo << " onto " << destinationEvent << endl;
      Incidence *to = destinationEvent;
      while(to) {
        if (to->uid() == todo->uid()) {
          KMessageBox::information(this,
              i18n("Cannot move to-do to itself or a child of itself."),
              i18n("Drop To-do"), "NoDropTodoOntoItself" );
          delete todo;
          return;
        }
        to = to->relatedTo();
      }

      Todo*oldTodo = existingTodo->clone();
      if ( mChanger->beginChange( existingTodo, 0, TQString() ) ) {
        existingTodo->setRelatedTo( destinationEvent );
        mChanger->changeIncidence( oldTodo, existingTodo, KOGlobals::RELATION_MODIFIED, this );
        mChanger->endChange( existingTodo, 0, TQString() );
      } else {
        KMessageBox::sorry( this, i18n("Unable to change to-do's parent, "
                            "because the to-do cannot be locked.") );
      }
      delete oldTodo;
      delete todo;
    } else {
//      kdDebug(5850) << "Drop new Todo" << endl;
      todo->setRelatedTo(destinationEvent);
      if ( !mChanger->addIncidence( todo, 0, TQString(), this ) ) {
        KODialogManager::errorSaveIncidence( this, todo );
        delete todo;
        return;
      }
    }
  } else {
    TQString text;
    KOTodoViewItem *todoi = dynamic_cast<KOTodoViewItem *>(itemAt( contentsToViewport(e->pos()) ));
    if ( ! todoi ) {
      // Not dropped on a todo item:
      e->ignore();
      kdDebug( 5850 ) << "KOTodoListView::contentsDropEvent(): Not dropped on a todo item" << endl;
      kdDebug( 5850 ) << "TODO: Create a new todo with the given data" << endl;
      // FIXME: Create a new todo with the given text/contact/whatever
    } else if ( TQTextDrag::decode(e, text) ) {
      //TQListViewItem *qlvi = itemAt( contentsToViewport(e->pos()) );
      kdDebug(5850) << "Dropped : " << text << endl;
      Todo*todo = todoi->todo();
      if( mChanger->beginChange( todo, 0, TQString() ) ) {
        Todo*oldtodo = todo->clone();

        if( text.startsWith( "file:" ) ) {
          todo->addAttachment( new Attachment( text ) );
        } else {
          TQStringList emails = KPIM::splitEmailAddrList( text );
          for(TQStringList::ConstIterator it = emails.begin();it!=emails.end();++it) {
            kdDebug(5850) << " Email: " << (*it) << endl;
            int pos = (*it).find("<");
            TQString name = (*it).left(pos);
            TQString email = (*it).mid(pos);
            if (!email.isEmpty() && todoi) {
              todo->addAttendee( new Attendee( name, email ) );
            }
          }
        }
        //FIXME: attendees or attachment added, so there is something modified
        mChanger->changeIncidence( oldtodo, todo, KOGlobals::NOTHING_MODIFIED, this );
        mChanger->endChange( todo, 0, TQString() );
      } else {
        KMessageBox::sorry( this, i18n("Unable to add attendees to the to-do, "
            "because the to-do cannot be locked.") );
      }
    }
    else {
      kdDebug(5850) << "KOTodoListView::contentsDropEvent(): Todo from drop not decodable" << endl;
      e->ignore();
    }
  }
#endif
}

void KOTodoListView::contentsMousePressEvent(TQMouseEvent* e)
{
  TQListView::contentsMousePressEvent(e);
  TQPoint p(contentsToViewport(e->pos()));
  TQListViewItem *i = itemAt(p);
  if (i) {
    // if the user clicked into the root decoration of the item, don't
    // try to start a drag!
    if (p.x() > header()->sectionPos(header()->mapToIndex(0)) +
        treeStepSize() * (i->depth() + (rootIsDecorated() ? 1 : 0)) +
        itemMargin() ||
        p.x() < header()->sectionPos(header()->mapToIndex(0))) {
      if (e->button()==Qt::LeftButton) {
        mPressPos = e->pos();
        mMousePressed = true;
      }
    }
  }
}

void KOTodoListView::contentsMouseMoveEvent(TQMouseEvent* e)
{
#ifndef KORG_NODND
//  kdDebug(5850) << "KOTodoListView::contentsMouseMoveEvent()" << endl;
  TQListView::contentsMouseMoveEvent(e);
  if (mMousePressed && (mPressPos - e->pos()).manhattanLength() >
      TQApplication::startDragDistance()) {
    mMousePressed = false;
    TQListViewItem *item = itemAt(contentsToViewport(mPressPos));
    if ( item && mCalendar ) {
//      kdDebug(5850) << "Start Drag for item " << item->text(0) << endl;
      DndFactory factory( mCalendar );
      ICalDrag *vd = factory.createDrag(
                          ((KOTodoViewItem *)item)->todo(),viewport());
      if (vd->drag()) {
        kdDebug(5850) << "KOTodoListView::contentsMouseMoveEvent(): Delete drag source" << endl;
      }
/*
      TQString source = fullPath(item);
      if ( TQFile::exists(source) ) {
        KURL url;
        url.setPath(source);
        KURLDrag* ud = KURLDrag::newDrag(KURL::List(url), viewport());
        if ( ud->drag() )
          TQMessageBox::information( this, "Drag source",
                                    TQString("Delete ")+source, "Not implemented" );
*/
    }
  }
#endif
}

void KOTodoListView::contentsMouseReleaseEvent(TQMouseEvent *e)
{
  TQListView::contentsMouseReleaseEvent(e);
  mMousePressed = false;
}

void KOTodoListView::contentsMouseDoubleClickEvent(TQMouseEvent *e)
{
  if (!e) return;

  TQPoint vp = contentsToViewport(e->pos());

  TQListViewItem *item = itemAt(vp);

  if (!item) return;

  emit doubleClicked(item,vp,0);
}

/////////////////////////////////////////////////////////////////////////////

KOTodoView::KOTodoView( Calendar *calendar, TQWidget *parent, const char* name)
  : KOrg::BaseView( calendar, parent, name )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );

  TQLabel *title = new TQLabel( i18n("To-dos:"), this );
  title->setFrameStyle( TQFrame::Panel | TQFrame::Raised );
  topLayout->addWidget( title );

  mQuickAdd = new KPIM::ClickLineEdit( this, i18n( "Click to add a new to-do" ) );
  mQuickAdd->setAcceptDrops( false );
  topLayout->addWidget( mQuickAdd );

  if ( !KOPrefs::instance()->mEnableQuickTodo ) mQuickAdd->hide();

  mTodoListView = new KOTodoListView( this );
  topLayout->addWidget( mTodoListView );

  mTodoListView->setRootIsDecorated( true );
  mTodoListView->setAllColumnsShowFocus( true );

  mTodoListView->setShowSortIndicator( true );

  mTodoListView->addColumn( i18n("Summary") );
  mTodoListView->addColumn( i18n("Recurs") );
  mTodoListView->addColumn( i18n("Priority") );
  mTodoListView->setColumnAlignment( ePriorityColumn, AlignHCenter );
  mTodoListView->addColumn( i18n("Complete") );
  mTodoListView->setColumnAlignment( ePercentColumn, AlignRight );
  mTodoListView->addColumn( i18n("Due Date/Time") );
  mTodoListView->setColumnAlignment( eDueDateColumn, AlignLeft );
  mTodoListView->addColumn( i18n("Categories") );
  mTodoListView->addColumn( i18n( "Calendar" ) );
#if 0
  mTodoListView->addColumn( i18n("Sort Id") );
  mTodoListView->setColumnAlignment( 4, AlignHCenter );
#endif

  mTodoListView->setMinimumHeight( 60 );
  mTodoListView->setItemsRenameable( true );
  mTodoListView->setRenameable( 0 );

  mTodoListView->setColumnWidthMode( eSummaryColumn, TQListView::Manual );
  mTodoListView->setColumnWidthMode( eRecurColumn, TQListView::Manual );
  mTodoListView->setColumnWidthMode( ePriorityColumn, TQListView::Manual );
  mTodoListView->setColumnWidthMode( ePercentColumn, TQListView::Manual );
  mTodoListView->setColumnWidthMode( eDueDateColumn, TQListView::Manual );
  mTodoListView->setColumnWidthMode( eCategoriesColumn, TQListView::Manual );
  mTodoListView->setColumnWidthMode( eFolderColumn, TQListView::Manual );
#if 0
  mTodoListView->setColumnWidthMode( eDescriptionColumn, TQListView::Manual );
#endif

  mPriorityPopupMenu = new TQPopupMenu( this );
  mPriority[ mPriorityPopupMenu->insertItem( i18n("Unspecified priority", "unspecified") ) ] = 0;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "1 (highest)") ) ] = 1;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "2" ) ) ] = 2;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "3" ) ) ] = 3;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "4" ) ) ] = 4;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "5 (medium)" ) ) ] = 5;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "6" ) ) ] = 6;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "7" ) ) ] = 7;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "8" ) ) ] = 8;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "9 (lowest)" ) ) ] = 9;
  connect( mPriorityPopupMenu, TQT_SIGNAL( activated( int ) ),
           TQT_SLOT( setNewPriority( int ) ));

  mPercentageCompletedPopupMenu = new TQPopupMenu(this);
  for (int i = 0; i <= 100; i+=10) {
    TQString label = TQString ("%1 %").arg (i);
    mPercentage[mPercentageCompletedPopupMenu->insertItem (label)] = i;
  }
  connect( mPercentageCompletedPopupMenu, TQT_SIGNAL( activated( int ) ),
           TQT_SLOT( setNewPercentage( int ) ) );

  mMovePopupMenu = new KDatePickerPopup(
                             KDatePickerPopup::NoDate |
                             KDatePickerPopup::DatePicker |
                             KDatePickerPopup::Words );
  mCopyPopupMenu = new KDatePickerPopup(
                             KDatePickerPopup::NoDate |
                             KDatePickerPopup::DatePicker |
                             KDatePickerPopup::Words );


  connect( mMovePopupMenu, TQT_SIGNAL( dateChanged( TQDate )),
           TQT_SLOT( setNewDate( TQDate ) ) );
  connect( mCopyPopupMenu, TQT_SIGNAL( dateChanged( TQDate )),
           TQT_SLOT( copyTodoToDate( TQDate ) ) );

  mItemPopupMenu = new TQPopupMenu(this);
  mItemPopupMenu->insertItem(i18n("&Show"), this,
                             TQT_SLOT (showTodo()));
  mItemPopupMenu->insertItem(i18n("&Edit..."), this,
                             TQT_SLOT (editTodo()), 0, ePopupEdit );
#ifndef KORG_NOPRINTER
  mItemPopupMenu->insertItem(KOGlobals::self()->smallIcon("printer1"), i18n("&Print..."), this, TQT_SLOT( printTodo() ) );
#endif
  mItemPopupMenu->insertItem(KOGlobals::self()->smallIconSet("editdelete"), i18n("&Delete"), this,
                             TQT_SLOT (deleteTodo()), 0, ePopupDelete );
  mItemPopupMenu->insertSeparator();
  mItemPopupMenu->insertItem(KOGlobals::self()->smallIconSet("todo"), i18n("New &To-do..."), this,
                             TQT_SLOT (newTodo()) );
  mItemPopupMenu->insertItem(i18n("New Su&b-to-do..."), this,
                             TQT_SLOT (newSubTodo()));
  mItemPopupMenu->insertItem( i18n("&Make this To-do Independent"), this,
      TQT_SIGNAL( unSubTodoSignal() ), 0, ePopupUnSubTodo );
  mItemPopupMenu->insertItem( i18n("Make all Sub-to-dos &Independent"), this,
      TQT_SIGNAL( unAllSubTodoSignal() ), 0, ePopupUnAllSubTodo );
  mItemPopupMenu->insertSeparator();
  mItemPopupMenu->insertItem( i18n("&Copy To"), mCopyPopupMenu, ePopupCopyTo );
  mItemPopupMenu->insertItem(i18n("&Move To"), mMovePopupMenu, ePopupMoveTo );
  mItemPopupMenu->insertSeparator();
  mItemPopupMenu->insertItem(i18n("delete completed to-dos","Pur&ge Completed"),
                             this, TQT_SLOT( purgeCompleted() ) );

  connect( mMovePopupMenu, TQT_SIGNAL( dateChanged( TQDate ) ),
           mItemPopupMenu, TQT_SLOT( hide() ) );
  connect( mCopyPopupMenu, TQT_SIGNAL( dateChanged( TQDate ) ),
           mItemPopupMenu, TQT_SLOT( hide() ) );

  mPopupMenu = new TQPopupMenu(this);
  mPopupMenu->insertItem(KOGlobals::self()->smallIconSet("todo"), i18n("&New To-do..."), this,
                         TQT_SLOT(newTodo()) );
  mPopupMenu->insertItem(i18n("delete completed to-dos","&Purge Completed"),
                         this, TQT_SLOT(purgeCompleted()));

  mDocPrefs = new DocPrefs( name );

  // Double clicking conflicts with opening/closing the subtree
  connect( mTodoListView, TQT_SIGNAL( doubleClicked( TQListViewItem *,
                                                 const TQPoint &, int ) ),
           TQT_SLOT( editItem( TQListViewItem *, const TQPoint &, int ) ) );
  connect( mTodoListView, TQT_SIGNAL( returnPressed( TQListViewItem * ) ),
           TQT_SLOT( editItem( TQListViewItem * ) ) );
  connect( mTodoListView, TQT_SIGNAL( contextMenuRequested( TQListViewItem *,
                                                        const TQPoint &, int ) ),
           TQT_SLOT( popupMenu( TQListViewItem *, const TQPoint &, int ) ) );
  connect( mTodoListView, TQT_SIGNAL( expanded( TQListViewItem * ) ),
           TQT_SLOT( itemStateChanged( TQListViewItem * ) ) );
  connect( mTodoListView, TQT_SIGNAL( collapsed( TQListViewItem * ) ),
           TQT_SLOT( itemStateChanged( TQListViewItem * ) ) );

#if 0
  connect(mTodoListView,TQT_SIGNAL(selectionChanged(TQListViewItem *)),
          TQT_SLOT(selectionChanged(TQListViewItem *)));
  connect(mTodoListView,TQT_SIGNAL(clicked(TQListViewItem *)),
          TQT_SLOT(selectionChanged(TQListViewItem *)));
  connect(mTodoListView,TQT_SIGNAL(pressed(TQListViewItem *)),
          TQT_SLOT(selectionChanged(TQListViewItem *)));
#endif
  connect( mTodoListView, TQT_SIGNAL(selectionChanged() ),
           TQT_SLOT( processSelectionChange() ) );
  connect( mQuickAdd, TQT_SIGNAL( returnPressed () ),
           TQT_SLOT( addQuickTodo() ) );

  new KOTodoListViewToolTip( mTodoListView->viewport(), calendar, mTodoListView );
}

KOTodoView::~KOTodoView()
{
  delete mDocPrefs;
}

void KOTodoView::setCalendar( Calendar *cal )
{
  BaseView::setCalendar( cal );
  mTodoListView->setCalendar( cal );
}

void KOTodoView::updateView()
{
//  kdDebug(5850) << "KOTodoView::updateView()" << endl;
  int oldPos = mTodoListView->contentsY();
  mItemsToDelete.clear();
  mTodoListView->clear();

  Todo::List todoList = calendar()->todos();

/*
  kdDebug(5850) << "KOTodoView::updateView(): Todo List:" << endl;
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

  // Put for each Event a KOTodoViewItem in the list view. Don't rely on a
  // specific order of events. That means that we have to generate parent items
  // recursively for proper hierarchical display of Todos.
  mTodoMap.clear();
  Todo::List::ConstIterator it;
  for( it = todoList.begin(); it != todoList.end(); ++it ) {
    if ( !mTodoMap.contains( *it ) ) {
      insertTodoItem( *it );
    }
  }

  // Restore opened/closed state
  mTodoListView->blockSignals( true );
  if( mDocPrefs ) restoreItemState( mTodoListView->firstChild() );
  mTodoListView->blockSignals( false );

  mTodoListView->setContentsPos( 0, oldPos );

  processSelectionChange();
}

void KOTodoView::restoreItemState( TQListViewItem *item )
{
  while( item ) {
    KOTodoViewItem *todoItem = (KOTodoViewItem *)item;
    todoItem->setOpen( mDocPrefs->readBoolEntry( todoItem->todo()->uid() ) );
    if( item->childCount() > 0 ) restoreItemState( item->firstChild() );
    item = item->nextSibling();
  }
}


TQMap<Todo *,KOTodoViewItem *>::ConstIterator
  KOTodoView::insertTodoItem(Todo *todo)
{
//  kdDebug(5850) << "KOTodoView::insertTodoItem(): " << todo->getSummary() << endl;
  Incidence *incidence = todo->relatedTo();
  if (incidence && incidence->type() == "Todo") {
    // Use dynamic_cast, because in the future the related item might also be an event
    Todo *relatedTodo = dynamic_cast<Todo *>(incidence);

    // just make sure we know we have this item already to avoid endless recursion (Bug 101696)
    mTodoMap.insert(todo,0);

//    kdDebug(5850) << "  has Related" << endl;
    TQMap<Todo *,KOTodoViewItem *>::ConstIterator itemIterator;
    itemIterator = mTodoMap.find(relatedTodo);
    if (itemIterator == mTodoMap.end()) {
//      kdDebug(5850) << "    related not yet in list" << endl;
      itemIterator = insertTodoItem (relatedTodo);
    }
    // isn't this pretty stupid? We give one Todo  to the KOTodoViewItem
    // and one into the map. Sure finding is more easy but why? -zecke
    KOTodoViewItem *todoItem;

    // in case we found a related parent, which has no KOTodoViewItem yet, this must
    // be the case where 2 items refer to each other, therefore simply create item as root item
    if ( *itemIterator == 0 ) {
      todo->setRelatedTo(0);  // break the recursion, else we will have troubles later
      todoItem = new KOTodoViewItem(mTodoListView,todo,this);
    }
    else
      todoItem = new KOTodoViewItem(*itemIterator,todo,this);

    return mTodoMap.insert(todo,todoItem);
  } else {
//    kdDebug(5850) << "  no Related" << endl;
      // see above -zecke
    KOTodoViewItem *todoItem = new KOTodoViewItem(mTodoListView,todo,this);
    return mTodoMap.insert(todo,todoItem);
  }
}

void KOTodoView::removeTodoItems()
{
  KOTodoViewItem *item;
  for ( item = mItemsToDelete.first(); item; item = mItemsToDelete.next() ) {
    Todo *todo = item->todo();
    if ( todo && mTodoMap.contains( todo ) ) {
      mTodoMap.remove( todo );
    }
    delete item;
  }
  mItemsToDelete.clear();
}


bool KOTodoView::scheduleRemoveTodoItem( KOTodoViewItem *todoItem )
{
  if ( todoItem ) {
    mItemsToDelete.append( todoItem );
    TQTimer::singleShot( 0, this, TQT_SLOT( removeTodoItems() ) );
    return true;
  } else
    return false;
}

void KOTodoView::updateConfig()
{
  mTodoListView->repaintContents();
}

Incidence::List KOTodoView::selectedIncidences()
{
  Incidence::List selected;

  KOTodoViewItem *item = (KOTodoViewItem *)(mTodoListView->selectedItem());
//  if (!item) item = mActiveItem;
  if (item) selected.append(item->todo());

  return selected;
}

Todo::List KOTodoView::selectedTodos()
{
  Todo::List selected;

  KOTodoViewItem *item = (KOTodoViewItem *)(mTodoListView->selectedItem());
//  if (!item) item = mActiveItem;
  if (item) selected.append(item->todo());

  return selected;
}

void KOTodoView::changeIncidenceDisplay(Incidence *incidence, int action)
{
  // The todo view only displays todos, so exit on all other incidences
  if ( incidence->type() != "Todo" )
    return;
  CalFilter *filter = calendar()->filter();
  bool isFiltered = filter && !filter->filterIncidence( incidence );
  Todo *todo = static_cast<Todo *>(incidence);
  if ( todo ) {
    KOTodoViewItem *todoItem = 0;
    if ( mTodoMap.contains( todo ) ) {
      todoItem = mTodoMap[todo];
    }
    switch ( action ) {
      case KOGlobals::INCIDENCEADDED:
      case KOGlobals::INCIDENCEEDITED:
        // If it's already there, edit it, otherwise just add
        if ( todoItem ) {
          if ( isFiltered ) {
            scheduleRemoveTodoItem( todoItem );
          } else {
            // correctly update changes in relations
            Todo*parent = dynamic_cast<Todo*>( todo->relatedTo() );
            KOTodoViewItem*parentItem = 0;
            if ( parent && mTodoMap.contains(parent) ) {
              parentItem = mTodoMap[ parent ];
            }
            if ( todoItem->parent() != parentItem ) {
              // The relations changed
              if ( parentItem ) {
                parentItem->insertItem( todoItem );
              } else {
                mTodoListView->insertItem( todoItem );
              }
            }
            todoItem->construct();
          }
        } else {
          if ( !isFiltered ) {
            insertTodoItem( todo );
          }
        }
        mTodoListView->sort();
        break;
      case KOGlobals::INCIDENCEDELETED:
        if ( todoItem ) {
          scheduleRemoveTodoItem( todoItem );
        }
        break;
      default:
        TQTimer::singleShot( 0, this, TQT_SLOT( updateView() ) );
    }
  } else {
    // use a TQTimer here, because when marking todos finished using
    // the checkbox, this slot gets called, but we cannot update the views
    // because we're still inside KOTodoViewItem::stateChange
    TQTimer::singleShot(0,this,TQT_SLOT(updateView()));
  }
}

void KOTodoView::showDates(const TQDate &, const TQDate &)
{
}

void KOTodoView::showIncidences( const Incidence::List &, const TQDate & )
{
  kdDebug(5850) << "KOTodoView::showIncidences( const Incidence::List & ): not yet implemented" << endl;
}

CalPrinterBase::PrintType KOTodoView::printType()
{
  return CalPrinterBase::Todolist;
}

void KOTodoView::editItem( TQListViewItem *item )
{
  if ( item ) {
    emit editIncidenceSignal( static_cast<KOTodoViewItem *>( item )->todo(), TQDate () );
  }
}

void KOTodoView::editItem( TQListViewItem *item, const TQPoint &, int )
{
  editItem( item );
}

void KOTodoView::showItem( TQListViewItem *item )
{
  if ( item ) {
    emit showIncidenceSignal( static_cast<KOTodoViewItem *>( item )->todo(), TQDate() );
  }
}

void KOTodoView::showItem( TQListViewItem *item, const TQPoint &, int )
{
  showItem( item );
}

void KOTodoView::popupMenu( TQListViewItem *item, const TQPoint &, int column )
{
  mActiveItem = static_cast<KOTodoViewItem *>( item );
  if ( mActiveItem && mActiveItem->todo() &&
       !mActiveItem->todo()->isReadOnly() ) {
    bool editable = !mActiveItem->todo()->isReadOnly();
    mItemPopupMenu->setItemEnabled( ePopupEdit, editable );
    mItemPopupMenu->setItemEnabled( ePopupDelete, editable );
    mItemPopupMenu->setItemEnabled( ePopupMoveTo, editable );
    mItemPopupMenu->setItemEnabled( ePopupCopyTo, editable );
    mItemPopupMenu->setItemEnabled( ePopupUnSubTodo, editable );
    mItemPopupMenu->setItemEnabled( ePopupUnAllSubTodo, editable );

    if ( editable ) {
      TQDate date = mActiveItem->todo()->dtDue().date();
      if ( mActiveItem->todo()->hasDueDate () ) {
        mMovePopupMenu->datePicker()->setDate( date );
      } else {
        mMovePopupMenu->datePicker()->setDate( TQDate::currentDate() );
      }
      switch ( column ) {
        case ePriorityColumn:
          mPriorityPopupMenu->popup( TQCursor::pos() );
          break;
        case ePercentColumn: {
          mPercentageCompletedPopupMenu->popup( TQCursor::pos() );
          break;
        }
        case eDueDateColumn:
          mMovePopupMenu->popup( TQCursor::pos() );
          break;
        case eCategoriesColumn:
          getCategoryPopupMenu( mActiveItem )->popup( TQCursor::pos() );
          break;
        default:
          mCopyPopupMenu->datePicker()->setDate( date );
          mCopyPopupMenu->datePicker()->setDate( TQDate::currentDate() );
          mItemPopupMenu->setItemEnabled( ePopupUnSubTodo,
                                          mActiveItem->todo()->relatedTo() );
          mItemPopupMenu->setItemEnabled( ePopupUnAllSubTodo,
                                          !mActiveItem->todo()->relations().isEmpty() );
          mItemPopupMenu->popup( TQCursor::pos() );
      }
    } else {
      mItemPopupMenu->popup( TQCursor::pos() );
    }
  } else mPopupMenu->popup( TQCursor::pos() );
}

void KOTodoView::newTodo()
{
  kdDebug() << k_funcinfo << endl;
  emit newTodoSignal( 0/*ResourceCalendar*/, TQString()/*subResource*/,
                      TQDate::currentDate().addDays(7) );
}

void KOTodoView::newSubTodo()
{
  if (mActiveItem) {
    emit newSubTodoSignal(mActiveItem->todo());
  }
}

void KOTodoView::editTodo()
{
  editItem( mActiveItem );
}

void KOTodoView::showTodo()
{
  showItem( mActiveItem );
}

void KOTodoView::printTodo()
{
#ifndef KORG_NOPRINTER
  KOCoreHelper helper;
  CalPrinter printer( this, BaseView::calendar(), &helper );
  connect( this, TQT_SIGNAL(configChanged()), &printer, TQT_SLOT(updateConfig()) );

  Incidence::List selectedIncidences;
  selectedIncidences.append( mActiveItem->todo() );

  TQDateTime todoDate;
  if ( mActiveItem->todo() && mActiveItem->todo()->hasStartDate() ) {
    todoDate = mActiveItem->todo()->dtStart();
  } else {
    todoDate = mActiveItem->todo()->dtDue();
  }

  printer.print( KOrg::CalPrinterBase::Incidence,
                 todoDate.date(), todoDate.date(), selectedIncidences );
#endif
}

void KOTodoView::deleteTodo()
{
  if (mActiveItem) {
    emit deleteIncidenceSignal( mActiveItem->todo() );
  }
}

void KOTodoView::setNewPriority(int index)
{
  if ( !mActiveItem || !mChanger ) return;
  Todo *todo = mActiveItem->todo();
  if ( !todo->isReadOnly () &&
       mChanger->beginChange( todo, 0, TQString() ) ) {
    Todo *oldTodo = todo->clone();
    todo->setPriority(mPriority[index]);
    mActiveItem->construct();

    mChanger->changeIncidence( oldTodo, todo, KOGlobals::PRIORITY_MODIFIED, this );
    mChanger->endChange( todo, 0, TQString() );
    delete oldTodo;
  }
}

void KOTodoView::setNewPercentage( KOTodoViewItem *item, int percentage )
{
  kdDebug(5850) << "KOTodoView::setNewPercentage( " << percentage << "), item = " << item << endl;
  if ( !item || !mChanger  ) return;
  Todo *todo = item->todo();
  if ( !todo ) return;

  if ( !todo->isReadOnly () &&
       mChanger->beginChange( todo, 0, TQString() ) ) {
    Todo *oldTodo = todo->clone();

/*  Old code to make sub-items's percentage related to this one's:
    TQListViewItem *myChild = firstChild();
    KOTodoViewItem *item;
    while( myChild ) {
      item = static_cast<KOTodoViewItem*>(myChild);
      item->stateChange(state);
      myChild = myChild->nextSibling();
    }*/
    if ( percentage == 100 ) {
      todo->setCompleted( TQDateTime::currentDateTime() );
      // If the todo does recur, it doesn't get set as completed. However, the
      // item is still checked. Uncheck it again.
      if ( !todo->isCompleted() ) {
        item->setState( TQCheckListItem::Off );
      }
    } else {
      todo->setPercentComplete( percentage );
    }
    item->construct();
    if ( todo->doesRecur() && percentage == 100 )
      mChanger->changeIncidence( oldTodo, todo,
                                 KOGlobals::COMPLETION_MODIFIED_WITH_RECURRENCE, this );
    else
      mChanger->changeIncidence( oldTodo, todo,
                                 KOGlobals::COMPLETION_MODIFIED, this );
    mChanger->endChange( todo, 0, TQString() );
    delete oldTodo;
  } else {
    item->construct();
    kdDebug(5850) << "No active item, active item is read-only, or locking failed" << endl;
  }
}

void KOTodoView::setNewPercentage( int index )
{
  setNewPercentage( mActiveItem, mPercentage[index] );
}

void KOTodoView::setNewDate( TQDate date )
{
  if ( !mActiveItem || !mChanger ) return;
  Todo *todo = mActiveItem->todo();
  if ( !todo ) return;

  if ( !todo->isReadOnly() && mChanger->beginChange( todo, 0, TQString() ) ) {
    Todo *oldTodo = todo->clone();

    TQDateTime dt;
    dt.setDate( date );

    if ( !todo->doesFloat() ) {
      dt.setTime( todo->dtDue().time() );
    }

    todo->setHasDueDate( !date.isNull() );
    todo->setDtDue( dt );

    mActiveItem->construct();
    mChanger->changeIncidence( oldTodo, todo, KOGlobals::COMPLETION_MODIFIED, this );
    mChanger->endChange( todo, 0, TQString() );
    delete oldTodo;
  } else {
    kdDebug(5850) << "No active item, active item is read-only, or locking failed" << endl;
  }
}

void KOTodoView::copyTodoToDate( TQDate date )
{
  TQDateTime dt( date );

  if ( mActiveItem && mChanger ) {
    Todo *oldTodo = mActiveItem->todo();
    Todo *newTodo = oldTodo->clone();
    newTodo->recreate();

    newTodo->setHasDueDate( !date.isNull() );

    if ( oldTodo->hasDueDate() && !oldTodo->doesFloat() ) {
      dt.setTime( oldTodo->dtDue().time() );
    }

    newTodo->setDtDue( dt );
    newTodo->setPercentComplete( 0 );

    QPair<ResourceCalendar *, TQString>p =
      CalHelper::incSubResourceCalendar( calendar(), mActiveItem->todo() );

    mChanger->addIncidence( newTodo, p.first, p.second, this );
  }
}

TQPopupMenu *KOTodoView::getCategoryPopupMenu( KOTodoViewItem *todoItem )
{
  TQPopupMenu *tempMenu = new TQPopupMenu( this );
  TQStringList checkedCategories = todoItem->todo()->categories();

  tempMenu->setCheckable( true );
  TQStringList::Iterator it;
  for ( it = KOPrefs::instance()->mCustomCategories.begin();
        it != KOPrefs::instance()->mCustomCategories.end();
        ++it ) {
    int index = tempMenu->insertItem( *it );
    mCategory[ index ] = *it;
    if ( checkedCategories.find( *it ) != checkedCategories.end() )
      tempMenu->setItemChecked( index, true );
  }

  connect ( tempMenu, TQT_SIGNAL( activated( int ) ),
            TQT_SLOT( changedCategories( int ) ) );
  return tempMenu;
}

void KOTodoView::changedCategories(int index)
{
  if ( !mActiveItem || !mChanger ) return;
  Todo *todo = mActiveItem->todo();
  if ( !todo ) return;

  if ( !todo->isReadOnly() && mChanger->beginChange( todo, 0, TQString() ) ) {
    Todo *oldTodo = todo->clone();

    TQStringList categories = todo->categories ();
    if ( categories.find( mCategory[index] ) != categories.end() )
      categories.remove( mCategory[index] );
    else
      categories.insert( categories.end(), mCategory[index] );
    categories.sort();
    todo->setCategories( categories );
    mActiveItem->construct();
    mChanger->changeIncidence( oldTodo, todo, KOGlobals::CATEGORY_MODIFIED, this );
    mChanger->endChange( todo, 0, TQString() );
    delete oldTodo;
  } else {
    kdDebug(5850) << "No active item, active item is read-only, or locking failed" << endl;
  }
}

void KOTodoView::setDocumentId( const TQString &id )
{
  kdDebug(5850) << "KOTodoView::setDocumentId()" << endl;

  mDocPrefs->setDoc( id );
}

void KOTodoView::itemStateChanged( TQListViewItem *item )
{
  if (!item) return;

  KOTodoViewItem *todoItem = (KOTodoViewItem *)item;

//  kdDebug(5850) << "KOTodoView::itemStateChanged(): " << todoItem->todo()->summary() << endl;

  if( mDocPrefs ) mDocPrefs->writeEntry( todoItem->todo()->uid(), todoItem->isOpen() );
}

void KOTodoView::setNewPercentageDelayed( KOTodoViewItem *item, int percentage )
{
  mPercentChangedMap.append( qMakePair( item, percentage ) );

  TQTimer::singleShot( 0, this, TQT_SLOT( processDelayedNewPercentage() ) );
}

void KOTodoView::processDelayedNewPercentage()
{
  TQValueList< QPair< KOTodoViewItem *, int> >::Iterator it;
  for ( it = mPercentChangedMap.begin(); it != mPercentChangedMap.end(); ++it )
    setNewPercentage( (*it).first, (*it).second );

  mPercentChangedMap.clear();
}

void KOTodoView::saveLayout(KConfig *config, const TQString &group) const
{
  mTodoListView->saveLayout(config,group);
}

void KOTodoView::restoreLayout(KConfig *config, const TQString &group)
{
  mTodoListView->restoreLayout(config,group);
}

void KOTodoView::processSelectionChange()
{
//  kdDebug(5850) << "KOTodoView::processSelectionChange()" << endl;

  KOTodoViewItem *item =
    static_cast<KOTodoViewItem *>( mTodoListView->selectedItem() );

  if ( !item ) {
    emit incidenceSelected( 0, TQDate() );
  } else {
    if ( selectedIncidenceDates().isEmpty() ) {
      emit incidenceSelected( item->todo(), TQDate() );
    } else {
      emit incidenceSelected( item->todo(), selectedIncidenceDates().first() );
    }
  }
}

void KOTodoView::clearSelection()
{
  mTodoListView->selectAll( false );
}

void KOTodoView::purgeCompleted()
{
  emit purgeCompletedSignal();
}

void KOTodoView::addQuickTodo()
{
  if ( ! mQuickAdd->text().stripWhiteSpace().isEmpty() ) {
    Todo *todo = new Todo();
    todo->setSummary( mQuickAdd->text() );
    todo->setOrganizer( Person( KOPrefs::instance()->fullName(),
                        KOPrefs::instance()->email() ) );
    if ( !mChanger->addIncidence( todo, 0, TQString(), this ) ) {
      delete todo;
      return;
    }
    mQuickAdd->setText( TQString::null );
  }
}

void KOTodoView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
  mTodoListView->setIncidenceChanger( changer );
}

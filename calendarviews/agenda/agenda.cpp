/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Kevin Krammer, krake@kdab.com

  Marcus Bains line.
  Copyright (c) 2001 Ali Rahimi <ali@mit.edu>

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
#include "agenda.h"

#include "agendaitem.h"
#include "eventview.h"
#include "helper.h"
#include "prefs.h"

#include <libkdepim/pimmessagebox.h>

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/utils.h>
#include <akonadi/kcal/incidencechanger.h>

#include <KCal/DndFactory>
#include <KCal/ICalDrag>
#include <KCal/Todo>
#include <KCal/VCalDrag>

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <KUrl>

#include <QScrollBar>
#include <QScrollArea>
#include <QDateTime>
#include <QApplication>
#include <QCursor>
#include <QPainter>
#include <QLabel>
#include <QWheelEvent>
#include <QPixmap>
#include <QVector>
#include <QList>
#include <QEvent>
#include <QKeyEvent>
#include <QFrame>
#include <QDropEvent>
#include <QPointer>
#include <QResizeEvent>
#include <QTimer>
#include <QMouseEvent>

#include <cmath>

using namespace Akonadi;
using namespace EventViews;

///////////////////////////////////////////////////////////////////////////////
class MarcusBains::Private
{
  MarcusBains *const q;

  public:
    Private( MarcusBains *parent, EventView *eventView, Agenda *agenda )
      : q( parent ), mEventView( eventView ), mAgenda( agenda ),
        mTimer( 0 ), mTimeBox( 0 ), mOldTime( 0, 0 ), mOldTodayCol( -1 )
    {
    }

    int todayColumn() const;

  public:
    EventView *mEventView;
    Agenda *mAgenda;
    QTimer *mTimer;
    QLabel *mTimeBox;  // Label showing the current time
    QTime mOldTime;
    int mOldTodayCol;
};

int MarcusBains::Private::todayColumn() const
{
  const QDate currentDate = QDate::currentDate();

  int col = 0;
  const DateList dateList = mAgenda->dateList();
  Q_FOREACH( const QDate &date, dateList ) {
    if ( date == currentDate ) {
      return QApplication::isRightToLeft() ? mAgenda->columns() - 1 - col : col;
    }
    ++col;
  }

  return -1;
}

MarcusBains::MarcusBains( EventView *eventView, Agenda *agenda )
  : QFrame( agenda ), d( new Private( this, eventView, agenda ) )
{
  d->mTimeBox = new QLabel( d->mAgenda );
  d->mTimeBox->setAlignment( Qt::AlignRight | Qt::AlignBottom );

  d->mTimer = new QTimer( this );
  d->mTimer->setSingleShot( true );
  connect( d->mTimer, SIGNAL(timeout()), this, SLOT(updateLocation()) );
  d->mTimer->start( 0 );
}

MarcusBains::~MarcusBains()
{
  delete d;
}

void MarcusBains::updateLocation()
{
  updateLocationRecalc();
}

void MarcusBains::updateLocationRecalc( bool recalculate )
{
  bool showSeconds = d->mEventView->preferences()->marcusBainsShowSeconds();
  QColor color = d->mEventView->preferences()->agendaMarcusBainsLineLineColor();

  QTime tim = QTime::currentTime();
  if ( ( tim.hour() == 0 ) && ( d->mOldTime.hour() == 23 ) ) {
    // We are on a new day
    recalculate = true;
  }
  int todayCol = recalculate ? d->todayColumn() : d->mOldTodayCol;

  // Number of minutes since beginning of the day
  int minutes = tim.hour() * 60 + tim.minute();
  int minutesPerCell = 24 * 60 / d->mAgenda->rows();

  d->mOldTime = tim;
  d->mOldTodayCol = todayCol;

  int y = int( minutes  *  d->mAgenda->gridSpacingY() / minutesPerCell );
  int x = int( d->mAgenda->gridSpacingX() * todayCol );

  bool hideIt = !( d->mEventView->preferences()->marcusBainsEnabled() );
  if ( !isHidden() && ( hideIt || ( todayCol < 0 ) ) ) {
     hide();
     d->mTimeBox->hide();
     return;
  }

  if ( isHidden() && !hideIt ) {
    show();
    d->mTimeBox->show();
  }

  /* Line */
  // It seems logical to adjust the line width with the label's font weight
  int fw = d->mEventView->preferences()->agendaMarcusBainsLineFont().weight();
  setLineWidth( 1 + abs( fw - QFont::Normal ) / QFont::Light );
  setFrameStyle( QFrame::HLine | QFrame::Plain );
  QPalette pal = palette();
  pal.setColor( QPalette::Window, color ); // for Oxygen
  pal.setColor( QPalette::WindowText, color ); // for Plastique
  setPalette( pal );
  if ( recalculate ) {
    setFixedSize( int( d->mAgenda->gridSpacingX() ), 1 );
  }
  move( x, y );
  raise();

  /* Label */
  d->mTimeBox->setFont( d->mEventView->preferences()->agendaMarcusBainsLineFont() );
  QPalette pal1 = d->mTimeBox->palette();
  pal1.setColor( QPalette::WindowText, color );
  d->mTimeBox->setPalette( pal1 );
  d->mTimeBox->setText( KGlobal::locale()->formatTime( tim, showSeconds ) );
  d->mTimeBox->adjustSize();
  if ( y - d->mTimeBox->height() >= 0 ) {
    y -= d->mTimeBox->height();
  } else {
    y++;
  }
  if ( x - d->mTimeBox->width() + d->mAgenda->gridSpacingX() > 0 ) {
    x += int( d->mAgenda->gridSpacingX() - d->mTimeBox->width() - 1 );
  } else {
    x++;
  }
  d->mTimeBox->move( x, y );
  d->mTimeBox->raise();

  if ( showSeconds || recalculate ) {
    d->mTimer->start( 1000 );
  } else {
    d->mTimer->start( 1000 * ( 60 - tim.second() ) );
  }
}

////////////////////////////////////////////////////////////////////////////

class Agenda::Private
{
  Agenda *const q;

  public:
    Private( Agenda *parent, EventView *eventView, QScrollArea *scrollArea, int columns, int rows, int rowSize )
      : q( parent ), mEventView( eventView ), mScrollArea( scrollArea ), mAllDayMode( false ),
        mColumns( columns ), mRows( rows ), mGridSpacingX( 0.0 ), mGridSpacingY( rowSize ),
        mDesiredGridSpacingY( rowSize ), mCalendar( 0 ), mChanger( 0 ),
        mResizeBorderWidth( 0 ), mScrollBorderWidth( 0 ), mScrollDelay( 0 ), mScrollOffset( 0 ),
        mWorkingHoursEnable( false ), mHolidayMask( 0 ), mWorkingHoursYTop( 0 ),
        mWorkingHoursYBottom( 0 ), mHasSelection( 0 ), mSelectedId( -1 ), mMarcusBains( 0 ),
        mActionType( Agenda::NOP ), mItemMoved( false ), mOldLowerScrollValue( 0 ),
        mOldUpperScrollValue( 0 ), mReturnPressed( false )
    {
      if ( mGridSpacingY < 4 || mGridSpacingY > 30 ) {
        mGridSpacingY = 10;
      }
    }

  public:
    EventView *mEventView;
    QScrollArea *mScrollArea;

    bool mAllDayMode;

    // Number of Columns/Rows of agenda grid
    int mColumns;
    int mRows;

    // Width and height of agenda cells. mDesiredGridSpacingY is the height
    // set in the config. The actual height might be larger since otherwise
    // more than 24 hours might be displayed.
    double mGridSpacingX;
    double mGridSpacingY;
    double mDesiredGridSpacingY;

    // We need the calendar for drag'n'drop and for paint the ResourceColor
    Akonadi::Calendar *mCalendar;
    IncidenceChanger *mChanger;

    // size of border, where mouse action will resize the AgendaItem
    int mResizeBorderWidth;

    // size of border, where mouse mve will cause a scroll of the agenda
    int mScrollBorderWidth;
    int mScrollDelay;
    int mScrollOffset;

    QTimer mScrollUpTimer;
    QTimer mScrollDownTimer;

    // Cells to store Move and Resize coordiantes while performing the action
    QPoint mStartCell;
    QPoint mEndCell;

    // Working Hour coordiantes
    bool mWorkingHoursEnable;
    QVector<bool> *mHolidayMask;
    int mWorkingHoursYTop;
    int mWorkingHoursYBottom;

    // Selection
    bool mHasSelection;
    QPoint mSelectionStartPoint;
    QPoint mSelectionStartCell;
    QPoint mSelectionEndCell;

    // List of dates to be displayed
    DateList mSelectedDates;

    // The AgendaItem, which has been right-clicked last
    QPointer<AgendaItem> mClickedItem;

    // The AgendaItem, which is being moved/resized
    QPointer<AgendaItem> mActionItem;

    // Currently selected item
    QPointer<AgendaItem> mSelectedItem;
    // Id of the last selected item. Used for reselecting in situations
    // where the selected item points to a no longer valid incidence, for
    // example during resource reload.
    Akonadi::Item::Id mSelectedId;

    // The Marcus Bains Line widget.
    MarcusBains *mMarcusBains;

    MouseActionType mActionType;

    bool mItemMoved;

    // List of all Items contained in agenda
    QList<AgendaItem*> mItems;
    QList<AgendaItem*> mItemsToDelete;

    int mOldLowerScrollValue;
    int mOldUpperScrollValue;

    bool mReturnPressed;
};

/*
  Create an agenda widget with rows rows and columns columns.
*/
Agenda::Agenda( EventView *eventView, QScrollArea *scrollArea,
                int columns, int rows, int rowSize )
  : QWidget( scrollArea ), d( new Private( this, eventView, scrollArea, columns, rows, rowSize ) )
{
  setMouseTracking( true );

  init();
}

/*
  Create an agenda widget with columns columns and one row. This is used for
  all-day events.
*/
Agenda::Agenda( EventView *eventView, QScrollArea *scrollArea,
                int columns )
  : QWidget( scrollArea ), d( new Private( this, eventView, scrollArea, columns, 1, 24 ) )
{
  d->mAllDayMode = true;

  init();
}

Agenda::~Agenda()
{
  delete d->mMarcusBains;
  delete d;
}

Akonadi::Item Agenda::selectedIncidence() const
{
  return ( d->mSelectedItem ? d->mSelectedItem->incidence() : Item() );
}

QDate Agenda::selectedIncidenceDate() const
{
  return ( d->mSelectedItem ? d->mSelectedItem->itemDate() : QDate() );
}

Item::Id Agenda::lastSelectedItemId() const
{
  return d->mSelectedId;
}

void Agenda::init()
{
  d->mGridSpacingX = static_cast<double>( d->mScrollArea->width() ) / d->mColumns;
  d->mDesiredGridSpacingY = d->mEventView->preferences()->hourSize();
  if ( d->mDesiredGridSpacingY < 4 || d->mDesiredGridSpacingY > 30 ) {
    d->mDesiredGridSpacingY = 10;
  }

 // make sure that there are not more than 24 per day
  d->mGridSpacingY = static_cast<double>( height() ) / d->mRows;
  if ( d->mGridSpacingY < d->mDesiredGridSpacingY ) {
    d->mGridSpacingY = d->mDesiredGridSpacingY;
  }

  d->mResizeBorderWidth = 8;
  d->mScrollBorderWidth = 8;
  d->mScrollDelay = 30;
  d->mScrollOffset = 10;

  // Grab key strokes for keyboard navigation of agenda. Seems to have no
  // effect. Has to be fixed.
  setFocusPolicy( Qt::WheelFocus );

  connect( &d->mScrollUpTimer, SIGNAL(timeout()), SLOT(scrollUp()) );
  connect( &d->mScrollDownTimer, SIGNAL(timeout()), SLOT(scrollDown()) );

  d->mStartCell = QPoint( 0, 0 );
  d->mEndCell = QPoint( 0, 0 );

  d->mHasSelection = false;
  d->mSelectionStartPoint = QPoint( 0, 0 );
  d->mSelectionStartCell = QPoint( 0, 0 );
  d->mSelectionEndCell = QPoint( 0, 0 );

  d->mOldLowerScrollValue = -1;
  d->mOldUpperScrollValue = -1;

  d->mClickedItem = 0;

  d->mActionItem = 0;
  d->mActionType = NOP;
  d->mItemMoved = false;

  d->mSelectedItem = 0;
  d->mSelectedId = -1;

  setAcceptDrops( true );
  installEventFilter( this );

/*  resizeContents( int( mGridSpacingX * mColumns ), int( mGridSpacingY * mRows ) ); */

  d->mScrollArea->viewport()->update();
//  mScrollArea->viewport()->setAttribute( Qt::WA_NoSystemBackground, true );
  d->mScrollArea->viewport()->setFocusPolicy( Qt::WheelFocus );

  calculateWorkingHours();

  connect( verticalScrollBar(), SIGNAL(valueChanged(int)),
           SLOT(checkScrollBoundaries(int)) );

  // Create the Marcus Bains line.
  if( d->mAllDayMode ) {
    d->mMarcusBains = 0;
  } else {
    d->mMarcusBains = new MarcusBains( d->mEventView, this );
  }
}

void Agenda::clear()
{
  qDeleteAll( d->mItems );
  qDeleteAll( d->mItemsToDelete );
  d->mItems.clear();
  d->mItemsToDelete.clear();

  d->mSelectedItem = 0;

  clearSelection();
}

void Agenda::clearSelection()
{
  d->mHasSelection = false;
  d->mActionType = NOP;
  update();
}

void Agenda::marcus_bains()
{
  if ( d->mMarcusBains ) {
    d->mMarcusBains->updateLocationRecalc( true );
  }
}

void Agenda::changeColumns( int columns )
{
  if ( columns == 0 ) {
    kDebug() << "called with argument 0";
    return;
  }

  clear();
  d->mColumns = columns;
//  setMinimumSize( mColumns * 10, mGridSpacingY + 1 );
//  init();
//  update();

  QResizeEvent event( size(), size() );

  QApplication::sendEvent( this, &event );
}

int Agenda::columns() const
{
  return d->mColumns;
}

int Agenda::rows() const
{
  return d->mRows;
}

double Agenda::gridSpacingX() const
{
  return d->mGridSpacingX;
}

double Agenda::gridSpacingY() const
{
  return d->mGridSpacingY;
}

/*
  This is the eventFilter function, which gets all events from the AgendaItems
  contained in the agenda. It has to handle moving and resizing for all items.
*/
bool Agenda::eventFilter ( QObject *object, QEvent *event )
{
  switch( event->type() ) {
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonDblClick:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseMove:
    return eventFilter_mouse( object, static_cast<QMouseEvent *>( event ) );
#ifndef QT_NO_WHEELEVENT
  case QEvent::Wheel:
    return eventFilter_wheel( object, static_cast<QWheelEvent *>( event ) );
#endif
  case QEvent::KeyPress:
  case QEvent::KeyRelease:
    return eventFilter_key( object, static_cast<QKeyEvent *>( event ) );

  case ( QEvent::Leave ):
    if ( !d->mActionItem ) {
      setCursor( Qt::ArrowCursor );
    }

    if ( object == this ) {
      emit leaveAgenda();
    }
    return true;

  case QEvent::Enter:
    emit enterAgenda();
    return QWidget::eventFilter( object, event );

#ifndef KORG_NODND
  case QEvent::DragEnter:
  case QEvent::DragMove:
  case QEvent::DragLeave:
  case QEvent::Drop:
//  case QEvent::DragResponse:
    return eventFilter_drag( object, static_cast<QDropEvent*>( event ) );
#endif

  default:
    return QWidget::eventFilter( object, event );
  }
}

bool Agenda::eventFilter_drag( QObject *object, QDropEvent *de )
{
  // FIXME: Implement dropping of events!
  QPoint viewportPos;
  if ( object != this ) {
    viewportPos = static_cast<QWidget *>( object )->mapToParent( de->pos() );
  } else {
    viewportPos = de->pos();
  }
  const QMimeData *md = de->mimeData();

  switch ( de->type() ) {
  case QEvent::DragEnter:
  case QEvent::DragMove:
    if ( !Akonadi::canDecode( md ) ) {
      return false;
    }

    if ( Akonadi::mimeDataHasTodo( md ) ) {
      de->accept();
    } else {
      de->ignore();
    }
    return true;
    break;
  case QEvent::DragLeave:
    return false;
    break;
  case QEvent::Drop:
  {
    if ( !Akonadi::canDecode( md ) ) {
      return false;
    }

    const QList<KUrl> todoUrls = Akonadi::todoItemUrls( md );
    const QList<Todo::Ptr> todos = Akonadi::todos( md, d->mCalendar->timeSpec() );

    Q_ASSERT( !todoUrls.isEmpty() || !todos.isEmpty() );

    de->setDropAction( Qt::MoveAction );
    QPoint pos;
    // FIXME: This is a bad hack, as the viewportToContents seems to be off by
    // 2000 (which is the left upper corner of the viewport). It works correctly
    // for agendaItems.
    if ( object == this ) {
      pos = viewportPos + QPoint( contentsX(), contentsY() );
    } else {
      pos = viewportPos;// viewportToContents( viewportPos );
    }
    QPoint gpos = contentsToGrid( pos );
    if ( !todoUrls.isEmpty() ) {
      emit droppedToDos( todoUrls, gpos, d->mAllDayMode );
    } else {
      emit droppedToDos( todos, gpos, d->mAllDayMode );
    }
    return true;
  }
  break;

  case QEvent::DragResponse:
  default:
    break;
  }
  return false;
}

#ifndef QT_NO_WHEELEVENT
bool Agenda::eventFilter_wheel ( QObject *object, QWheelEvent *e )
{
  QPoint viewportPos;
  bool accepted=false;
  if  ( ( e->modifiers() & Qt::ShiftModifier ) == Qt::ShiftModifier ) {
    if ( object != this ) {
      viewportPos = ( (QWidget *) object )->mapToParent( e->pos() );
    } else {
      viewportPos = e->pos();
    }
    //kDebug() << type:" << e->type() << "delta:" << e->delta();
    emit zoomView( -e->delta(),
                   contentsToGrid(  viewportPos  ), Qt::Horizontal );
    accepted = true;
  }

  if  ( ( e->modifiers() & Qt::ControlModifier ) == Qt::ControlModifier ){
    if ( object != this ) {
      viewportPos = ( (QWidget *)object )->mapToParent( e->pos() );
    } else {
      viewportPos = e->pos();
    }
    emit zoomView( -e->delta(), contentsToGrid(  viewportPos  ), Qt::Vertical );
    emit mousePosSignal( gridToContents( contentsToGrid( viewportPos  ) ) );
    accepted = true;
  }
  if ( accepted ) {
    e->accept();
  }
  return accepted;
}
#endif

bool Agenda::eventFilter_key( QObject *, QKeyEvent *ke )
{
  return d->mEventView->processKeyEvent( ke );
}

bool Agenda::eventFilter_mouse( QObject *object, QMouseEvent *me )
{
  QPoint viewportPos;
  if ( object != this ) {
    viewportPos = static_cast<QWidget *>( object )->mapToParent( me->pos() );
  } else {
    viewportPos = me->pos();
  }

  switch ( me->type() )  {
  case QEvent::MouseButtonPress:
    if ( object != this ) {
      if ( me->button() == Qt::RightButton ) {
        d->mClickedItem = dynamic_cast<AgendaItem *>( object );
        if ( d->mClickedItem ) {
          selectItem( d->mClickedItem );
          emit showIncidencePopupSignal( d->mClickedItem->incidence(),
                                         d->mClickedItem->itemDate() );
        }
      } else {
        AgendaItem *item = dynamic_cast<AgendaItem *>(object);
        if (item) {
          const Item aitem = item->incidence();
          Incidence::Ptr incidence = Akonadi::incidence( aitem );
          if ( incidence->isReadOnly() ) {
            d->mActionItem = 0;
          } else {
            d->mActionItem = item;
            startItemAction( viewportPos );
          }
          // Warning: do selectItem() as late as possible, since all
          // sorts of things happen during this call. Some can lead to
          // this filter being run again and mActionItem being set to
          // null.
          selectItem( item );
        }
      }
    } else {
      if ( me->button() == Qt::RightButton ) {
        // if mouse pointer is not in selection, select the cell below the cursor
        QPoint gpos = contentsToGrid( viewportPos  );
        if ( !ptInSelection( gpos ) ) {
          d->mSelectionStartCell = gpos;
          d->mSelectionEndCell = gpos;
          d->mHasSelection = true;
          emit newStartSelectSignal();
          emit newTimeSpanSignal( d->mSelectionStartCell, d->mSelectionEndCell );
//          updateContents();
        }
        showNewEventPopupSignal();
      } else {
        selectItem( 0 );
        d->mActionItem = 0;
        setCursor( Qt::ArrowCursor );
        startSelectAction( viewportPos );
      }
    }
    break;

  case QEvent::MouseButtonRelease:
    if ( d->mActionItem ) {
      endItemAction();
    } else if ( d->mActionType == SELECT ) {
      endSelectAction( viewportPos );
    }
    // This nasty gridToContents(contentsToGrid(..)) is needed to
    // avoid an offset of a few pixels. Don't ask me why...
    emit mousePosSignal( gridToContents( contentsToGrid( viewportPos )  ) );
    break;

  case QEvent::MouseMove:
  {
    // This nasty gridToContents(contentsToGrid(..)) is needed to
    // avoid an offset of a few pixels. Don't ask me why...
    QPoint indicatorPos = gridToContents( contentsToGrid(  viewportPos  ) );
    if ( object != this ) {
      AgendaItem *moveItem = dynamic_cast<AgendaItem *>( object );
      const Item aitem = moveItem ? moveItem->incidence() : Item();
      Incidence::Ptr incidence = Akonadi::incidence( aitem );
      if ( incidence && !incidence->isReadOnly() ) {
        if ( !d->mActionItem ) {
          setNoActionCursor( moveItem, viewportPos );
        } else {
          performItemAction( viewportPos );

          if ( d->mActionType == MOVE ) {
            // show cursor at the current begin of the item
            AgendaItem *firstItem = d->mActionItem->firstMultiItem();
            if ( !firstItem ) {
              firstItem = d->mActionItem;
            }
            indicatorPos = gridToContents(
              QPoint( firstItem->cellXLeft(), firstItem->cellYTop() ) );

          } else if ( d->mActionType == RESIZEBOTTOM ) {
            // RESIZETOP is handled correctly, only resizebottom works differently
            indicatorPos = gridToContents(
              QPoint( d->mActionItem->cellXLeft(), d->mActionItem->cellYBottom() + 1 ) );
          }

        } // If we have an action item
      } // If move item && !read only
    } else {
      if ( d->mActionType == SELECT ) {
        performSelectAction( viewportPos );

        // show cursor at end of timespan
        if ( ( ( d->mStartCell.y() < d->mEndCell.y() ) && ( d->mEndCell.x() >= d->mStartCell.x() ) ) ||
             ( d->mEndCell.x() > d->mStartCell.x() ) ) {
          indicatorPos = gridToContents( QPoint( d->mEndCell.x(), d->mEndCell.y() + 1 ) );
        } else {
          indicatorPos = gridToContents( d->mEndCell );
        }
      }
    }
    emit mousePosSignal( indicatorPos );
    break;
  }

  case QEvent::MouseButtonDblClick:
    if ( object == this ) {
      selectItem( 0 );
      emit newEventSignal();
    } else {
      AgendaItem *doubleClickedItem = dynamic_cast<AgendaItem *>( object );
      if ( doubleClickedItem ) {
        selectItem( doubleClickedItem );
        emit editIncidenceSignal( doubleClickedItem->incidence() );
      }
    }
    break;

  default:
    break;
  }

  return true;
}

bool Agenda::ptInSelection( const QPoint &gpos ) const
{
  if ( !d->mHasSelection ) {
    return false;
  } else if ( gpos.x() < d->mSelectionStartCell.x() || gpos.x() > d->mSelectionEndCell.x() ) {
    return false;
  } else if ( ( gpos.x() == d->mSelectionStartCell.x() ) && ( gpos.y() < d->mSelectionStartCell.y() ) ) {
    return false;
  } else if ( ( gpos.x() == d->mSelectionEndCell.x() ) && ( gpos.y() > d->mSelectionEndCell.y() ) ) {
    return false;
  }
  return true;
}

void Agenda::startSelectAction( const QPoint &viewportPos )
{
  emit newStartSelectSignal();

  d->mActionType = SELECT;
  d->mSelectionStartPoint = viewportPos;
  d->mHasSelection = true;

  QPoint pos =  viewportPos ;
  QPoint gpos = contentsToGrid( pos );

  // Store new selection
  d->mStartCell = gpos;
  d->mEndCell = gpos;
  d->mSelectionStartCell = gpos;
  d->mSelectionEndCell = gpos;

//  updateContents();
}

void Agenda::performSelectAction( const QPoint &viewportPos )
{
  QPoint pos =  viewportPos ;
  QPoint gpos = contentsToGrid( pos );

  QPoint clipperPos = QPoint();

  // Scroll if cursor was moved to upper or lower end of agenda.
  if ( clipperPos.y() < d->mScrollBorderWidth ) {
    d->mScrollUpTimer.start( d->mScrollDelay );
  } else if ( d->mScrollArea->height() < d->mScrollBorderWidth  ) {
    d->mScrollDownTimer.start( d->mScrollDelay );
  } else {
    d->mScrollUpTimer.stop();
    d->mScrollDownTimer.stop();
  }

  if ( gpos != d->mEndCell ) {
    d->mEndCell = gpos;
    if ( d->mStartCell.x() > d->mEndCell.x() ||
         ( d->mStartCell.x() == d->mEndCell.x() && d->mStartCell.y() > d->mEndCell.y() ) ) {
      // backward selection
      d->mSelectionStartCell = d->mEndCell;
      d->mSelectionEndCell = d->mStartCell;
    } else {
      d->mSelectionStartCell = d->mStartCell;
      d->mSelectionEndCell = d->mEndCell;
    }

    update();
  }
}

void Agenda::endSelectAction( const QPoint &currentPos )
{
  d->mScrollUpTimer.stop();
  d->mScrollDownTimer.stop();

  d->mActionType = NOP;

  emit newTimeSpanSignal( d->mSelectionStartCell, d->mSelectionEndCell );

  if ( d->mEventView->preferences()->selectionStartsEditor() ) {
    if ( ( d->mSelectionStartPoint - currentPos ).manhattanLength() >
         QApplication::startDragDistance() ) {
       emit newEventSignal();
    }
  }
}

Agenda::MouseActionType Agenda::isInResizeArea( bool horizontal,
                                                const QPoint &pos,
                                                AgendaItem *item )
{
  if ( !item ) {
    return NOP;
  }
  QPoint gridpos = contentsToGrid( pos );
  QPoint contpos = gridToContents(
    gridpos + QPoint( ( QApplication::isRightToLeft() ) ? 1 : 0, 0 ) );

//kDebug() << "contpos=" << contpos << ", pos=" << pos << ", gpos=" << gpos;
//kDebug() << "clXLeft=" << clXLeft << ", clXRight=" << clXRight;

  if ( horizontal ) {
    int clXLeft = item->cellXLeft();
    int clXRight = item->cellXRight();
    if ( QApplication::isRightToLeft() ) {
      int tmp = clXLeft;
      clXLeft = clXRight;
      clXRight = tmp;
    }
    int gridDistanceX = int( pos.x() - contpos.x() );
    if ( gridDistanceX < d->mResizeBorderWidth && clXLeft == gridpos.x() ) {
      if ( QApplication::isRightToLeft() ) {
        return RESIZERIGHT;
      } else {
        return RESIZELEFT;
      }
    } else if ( ( d->mGridSpacingX - gridDistanceX ) < d->mResizeBorderWidth &&
                clXRight == gridpos.x() ) {
      if ( QApplication::isRightToLeft() ) {
        return RESIZELEFT;
      } else {
        return RESIZERIGHT;
      }
    } else {
      return MOVE;
    }
  } else {
    int gridDistanceY = int( pos.y() - contpos.y() );
    if ( gridDistanceY < d->mResizeBorderWidth &&
         item->cellYTop() == gridpos.y() && !item->firstMultiItem() ) {
      return RESIZETOP;
    } else if ( ( d->mGridSpacingY - gridDistanceY ) < d->mResizeBorderWidth &&
                item->cellYBottom() == gridpos.y() && !item->lastMultiItem() )  {
      return RESIZEBOTTOM;
    } else {
      return MOVE;
    }
  }
}

void Agenda::startItemAction( const QPoint &viewportPos )
{
  Q_ASSERT( d->mActionItem );

  QPoint pos =  viewportPos ;
  d->mStartCell = contentsToGrid( pos );
  d->mEndCell = d->mStartCell;

  bool noResize = Akonadi::hasTodo( d->mActionItem->incidence() );

  d->mActionType = MOVE;
  if ( !noResize ) {
    d->mActionType = isInResizeArea( d->mAllDayMode, pos, d->mActionItem );
  }

  d->mActionItem->startMove();
  setActionCursor( d->mActionType, true );
}

void Agenda::performItemAction( const QPoint &viewportPos )
{
//  kDebug() << "viewportPos:" << viewportPos.x() << "," << viewportPos.y();
//  QPoint point = viewport()->mapToGlobal(viewportPos);
//  kDebug() << "Global:" << point.x() << "," << point.y();
//  point = clipper()->mapFromGlobal(point);
//  kDebug() << "clipper:" << point.x() << "," << point.y();
//  kDebug() << "visible height:" << visibleHeight();
  QPoint pos =  viewportPos ;
//  kDebug() << "contents:" << x << "," << y;
  QPoint gpos = contentsToGrid( pos );
  QPoint clipperPos = QPoint();//clipper()->mapFromGlobal( viewport()->mapToGlobal( viewportPos ) );

  // Cursor left active agenda area.
  // This starts a drag.
//  if ( clipperPos.y() < 0 || clipperPos.y() >= visibleHeight() ||
//       clipperPos.x() < 0 || clipperPos.x() >= visibleWidth() ) {
  if (  false ) { //
    if ( d->mActionType == MOVE ) {
      d->mScrollUpTimer.stop();
      d->mScrollDownTimer.stop();
      d->mActionItem->resetMove();
      placeSubCells( d->mActionItem );
      emit startDragSignal( d->mActionItem->incidence() );
      setCursor( Qt::ArrowCursor );
      if ( d->mChanger ) {
        d->mChanger->cancelChange( d->mActionItem->incidence() );
      }
      d->mActionItem = 0;
      d->mActionType = NOP;
      d->mItemMoved = false;
      return;
    }
  } else {
    setActionCursor( d->mActionType, true );
  }

  // Scroll if item was moved to upper or lower end of agenda.
  if ( clipperPos.y() < d->mScrollBorderWidth ) {
    d->mScrollUpTimer.start( d->mScrollDelay );
  } else if ( d->mScrollArea->height() < d->mScrollBorderWidth ) {
    d->mScrollDownTimer.start( d->mScrollDelay );
  } else {
    d->mScrollUpTimer.stop();
    d->mScrollDownTimer.stop();
  }

  // Move or resize item if necessary
  if ( d->mEndCell != gpos ) {
    if ( !d->mItemMoved ) {
      if ( !d->mChanger || !d->mChanger->beginChange( d->mActionItem->incidence() ) ) {
        KMessageBox::information( this,
                                  i18n( "Unable to lock item for modification. "
                                        "You cannot make any changes." ),
                                  i18n( "Locking Failed" ), "AgendaLockingFailed" );
        d->mScrollUpTimer.stop();
        d->mScrollDownTimer.stop();
        d->mActionItem->resetMove();
        placeSubCells( d->mActionItem );
        setCursor( Qt::ArrowCursor );
        d->mActionItem = 0;
        d->mActionType = NOP;
        d->mItemMoved = false;
        return;
      }
      d->mItemMoved = true;
    }
    d->mActionItem->raise();
    if ( d->mActionType == MOVE ) {
      // Move all items belonging to a multi item
      AgendaItem *firstItem = d->mActionItem->firstMultiItem();
      if ( !firstItem ) {
        firstItem = d->mActionItem;
      }
      AgendaItem *lastItem = d->mActionItem->lastMultiItem();
      if ( !lastItem ) {
        lastItem = d->mActionItem;
      }
      QPoint deltapos = gpos - d->mEndCell;
      AgendaItem *moveItem = firstItem;
      while ( moveItem ) {
        bool changed = false;
        if ( deltapos.x() != 0 ) {
          moveItem->moveRelative( deltapos.x(), 0 );
          changed = true;
        }
        // in all day view don't try to move multi items, since there are none
        if ( moveItem == firstItem && !d->mAllDayMode ) { // is the first item
          int newY = deltapos.y() + moveItem->cellYTop();
          // If event start moved earlier than 0:00, it starts the previous day
          if ( newY < 0 ) {
            moveItem->expandTop( -moveItem->cellYTop() );
            // prepend a new item at ( x-1, rows()+newY to rows() )
            AgendaItem *newFirst = firstItem->prevMoveItem();
            // cell's y values are first and last cell of the bar,
            // so if newY=-1, they need to be the same
            if ( newFirst ) {
              newFirst->setCellXY( moveItem->cellXLeft() - 1, rows() + newY, rows() - 1 );
              d->mItems.append( newFirst );
              moveItem->resize( int( d->mGridSpacingX * newFirst->cellWidth() ),
                                int( d->mGridSpacingY * newFirst->cellHeight() ) );
              QPoint cpos = gridToContents(
                QPoint( newFirst->cellXLeft(), newFirst->cellYTop() ) );
              newFirst->setParent( this );
              newFirst->move( cpos.x(), cpos.y() );
            } else {
              newFirst = insertItem( moveItem->incidence(), moveItem->itemDate(),
                                     moveItem->cellXLeft() - 1, rows() + newY, rows() - 1 ) ;
            }
            if ( newFirst ) {
              newFirst->show();
            }
            moveItem->prependMoveItem( newFirst );
            firstItem = newFirst;
          } else if ( newY >= rows() ) {
            // If event start is moved past 24:00, it starts the next day
            // erase current item (i.e. remove it from the multiItem list)
            firstItem = moveItem->nextMultiItem();
            moveItem->hide();
            d->mItems.removeAll( moveItem );
//            removeChild( moveItem );
            d->mActionItem->removeMoveItem( moveItem );
            moveItem=firstItem;
            // adjust next day's item
            if ( moveItem ) {
              moveItem->expandTop( rows() - newY );
            }
          } else {
            moveItem->expandTop( deltapos.y(), true );
          }
          changed=true;
        }
        if ( moveItem && !moveItem->lastMultiItem() && !d->mAllDayMode ) { // is the last item
          int newY = deltapos.y() + moveItem->cellYBottom();
          if ( newY < 0 ) {
            // erase current item
            lastItem = moveItem->prevMultiItem();
            moveItem->hide();
            d->mItems.removeAll( moveItem );
//            removeChild( moveItem );
            moveItem->removeMoveItem( moveItem );
            moveItem = lastItem;
            moveItem->expandBottom( newY + 1 );
          } else if ( newY >= rows() ) {
            moveItem->expandBottom( rows()-moveItem->cellYBottom() - 1 );
            // append item at ( x+1, 0 to newY-rows() )
            AgendaItem *newLast = lastItem->nextMoveItem();
            if ( newLast ) {
              newLast->setCellXY( moveItem->cellXLeft() + 1, 0, newY-rows() - 1 );
              d->mItems.append( newLast );
              moveItem->resize( int( d->mGridSpacingX * newLast->cellWidth() ),
                                int( d->mGridSpacingY * newLast->cellHeight() ) );
              QPoint cpos = gridToContents( QPoint( newLast->cellXLeft(), newLast->cellYTop() ) ) ;
              newLast->setParent( this );
              newLast->move( cpos.x(), cpos.y() );
            } else {
              newLast = insertItem( moveItem->incidence(), moveItem->itemDate(),
                                    moveItem->cellXLeft() + 1, 0, newY - rows() - 1 ) ;
            }
            moveItem->appendMoveItem( newLast );
            newLast->show();
            lastItem = newLast;
          } else {
            moveItem->expandBottom( deltapos.y() );
          }
          changed = true;
        }
        if ( changed ) {
          adjustItemPosition( moveItem );
        }
        if ( moveItem ) {
          moveItem = moveItem->nextMultiItem();
        }
      }
    } else if ( d->mActionType == RESIZETOP ) {
      if ( d->mEndCell.y() <= d->mActionItem->cellYBottom() ) {
        d->mActionItem->expandTop( gpos.y() - d->mEndCell.y() );
        adjustItemPosition( d->mActionItem );
      }
    } else if ( d->mActionType == RESIZEBOTTOM ) {
      if ( d->mEndCell.y() >= d->mActionItem->cellYTop() ) {
        d->mActionItem->expandBottom( gpos.y() - d->mEndCell.y() );
        adjustItemPosition( d->mActionItem );
      }
    } else if ( d->mActionType == RESIZELEFT ) {
      if ( d->mEndCell.x() <= d->mActionItem->cellXRight() ) {
        d->mActionItem->expandLeft( gpos.x() - d->mEndCell.x() );
        adjustItemPosition( d->mActionItem );
      }
    } else if ( d->mActionType == RESIZERIGHT ) {
      if ( d->mEndCell.x() >= d->mActionItem->cellXLeft() ) {
        d->mActionItem->expandRight( gpos.x() - d->mEndCell.x() );
        adjustItemPosition( d->mActionItem );
      }
    }
    d->mEndCell = gpos;
  }
}

void Agenda::endItemAction()
{
  //PENDING(AKONADI_PORT) review all this cloning and changer calls
  // kDebug();
  d->mActionType = NOP;
  d->mScrollUpTimer.stop();
  d->mScrollDownTimer.stop();
  setCursor( Qt::ArrowCursor );
  bool multiModify = false;
  // FIXME: do the cloning here...
  Akonadi::Item inc = d->mActionItem->incidence();
  const Incidence::Ptr incidence = Akonadi::incidence( inc );
 d->mItemMoved = d->mItemMoved && !( d->mStartCell.x() == d->mEndCell.x() &&
                                     d->mStartCell.y() == d->mEndCell.y() );

  if ( d->mItemMoved ) {
    bool modify = false;
    if ( incidence->recurs() ) {
      int res = d->mEventView->showMoveRecurDialog( d->mActionItem->incidence(),
                                                    d->mActionItem->itemDate() );
      switch ( res ) {
      case KMessageBox::Ok: // All occurrences
        // Moving the whole sequene of events is handled by the itemModified below.
        modify = true;
        break;
      case KMessageBox::Yes:
      { // Just this occurrence
        // Dissociate this occurrence:
        // create clone of event, set relation to old event, set cloned event
        // for mActionItem, add exception date to old event, changeIncidence
        // for the old event, remove the recurrence from the new copy and then
        // just go on with the newly adjusted mActionItem and let the usual
        // code take care of the new time!
        modify = true;
        multiModify = true;
        emit startMultiModify( i18n( "Dissociate event from recurrence" ) );
        Incidence::Ptr oldIncSaved( incidence->clone() );
        Incidence::Ptr newInc( d->mCalendar->dissociateOccurrence(
          inc, d->mActionItem->itemDate(), d->mEventView->preferences()->timeSpec() ) );
        if ( newInc ) {
          // don't recreate items, they already have the correct position
          emit enableAgendaUpdate( false );
          if ( d->mChanger ) {
            d->mChanger->changeIncidence( oldIncSaved, inc,
                                          IncidenceChanger::RECURRENCE_MODIFIED_ONE_ONLY, this );
          } else {
            kError() << "No IncidenceChanger set";
          }
#ifdef AKONADI_PORT_DISABLED // this needs to be done when the async item adding is done and we have the real akonadi item
          Akonadi::Item item;
          item.setPayload( newInc );
          d->mActionItem->setIncidence( item );
          d->mActionItem->dissociateFromMultiItem();
#endif
          if ( d->mChanger ) {
            d->mChanger->addIncidence( newInc, inc.parentCollection(), this );
          } else {
            kError() << "No IncidenceChanger set";
          }
          emit enableAgendaUpdate( true );
        } else {
          KMessageBox::sorry(
            this,
            i18n( "Unable to add the exception item to the calendar. "
                  "No change will be done." ),
            i18n( "Error Occurred" ) );
        }
        break;
      }
      case KMessageBox::No/*Future*/:
      { // All future occurrences
        // Dissociate this occurrence:
        // create clone of event, set relation to old event, set cloned event
        // for mActionItem, add recurrence end date to old event, changeIncidence
        // for the old event, adjust the recurrence for the new copy and then just
        // go on with the newly adjusted mActionItem and let the usual code take
        // care of the new time!
        modify = true;
        multiModify = true;
        emit startMultiModify( i18n( "Split future recurrences" ) );
        Incidence::Ptr oldIncSaved( incidence->clone() );
        Incidence::Ptr newInc( d->mCalendar->dissociateOccurrence(
          inc, d->mActionItem->itemDate(), d->mEventView->preferences()->timeSpec(), false ) );
        if ( newInc ) {
          emit enableAgendaUpdate( false );
#ifdef AKONADI_PORT_DISABLED // this needs to be done when the async item adding is done and we have the real akonadi item
          d->mActionItem->dissociateFromMultiItem();
          Item item;
          item.setPayload( newInc );
          d->mActionItem->setIncidence( item );
#endif
          if ( d->mChanger ) {
            d->mChanger->addIncidence( newInc, inc.parentCollection(), this );
          } else {
            kError() << "No IncidenceChanger set";
          }
          emit enableAgendaUpdate( true );
          if ( d->mChanger ) {
            d->mChanger->changeIncidence( oldIncSaved, inc,
                                          IncidenceChanger::RECURRENCE_MODIFIED_ALL_FUTURE, this );
          } else {
            kError() << "No IncidenceChanger set";
          }
        } else {
          KMessageBox::sorry(
            this,
            i18n( "Unable to add the future items to the calendar. "
                  "No change will be done." ),
            i18n( "Error Occurred" ) );
        }
        break;
      }
      default:
        modify = false;
        d->mActionItem->resetMove();
        placeSubCells( d->mActionItem ); //PENDING(AKONADI_PORT) should this be done after the new item was asynchronously added?
      }
    }

    if ( modify ) {
      d->mActionItem->endMove();
      AgendaItem *placeItem = d->mActionItem->firstMultiItem();
      if  ( !placeItem ) {
        placeItem = d->mActionItem;
      }

      AgendaItem *modif = placeItem;

      QList<AgendaItem*> oldconflictItems = placeItem->conflictItems();
      QList<AgendaItem*>::iterator it;
      for ( it = oldconflictItems.begin(); it != oldconflictItems.end(); ++it ) {
        placeSubCells( *it );
      }
      while ( placeItem ) {
        placeSubCells( placeItem );
        placeItem = placeItem->nextMultiItem();
      }

      // Notify about change
      // The agenda view will apply the changes to the actual Incidence*!
      // Bug #228696 don't call endChanged now it's async in Akonadi so it can
      // be called before that modified item was done.  And endChange is
      // calling when we move item.
      // Not perfect need to improve it!
      //mChanger->endChange( inc );
      emit itemModified( modif );
    } else {
      // the item was moved, but not further modified, since it's not recurring
      // make sure the view updates anyhow, with the right item
      if ( d->mChanger ) {
        d->mChanger->endChange( inc );
      } else {
        kError() << "No IncidenceChanger set";
      }
      emit itemModified( d->mActionItem );
    }
  } else {
    if ( d->mChanger ) {
      d->mChanger->endChange( inc );
    } else {
      kError() << "No IncidenceChanger set";
    }
  }

  d->mActionItem = 0;
  d->mItemMoved = false;

  if ( multiModify ) {
    emit endMultiModify();
  }

  kDebug() << "done";
}

void Agenda::setActionCursor( int actionType, bool acting )
{
  switch ( actionType ) {
    case MOVE:
      if ( acting ) {
        setCursor( Qt::SizeAllCursor );
      } else {
        setCursor( Qt::ArrowCursor );
      }
      break;
    case RESIZETOP:
    case RESIZEBOTTOM:
      setCursor( Qt::SizeVerCursor );
      break;
    case RESIZELEFT:
    case RESIZERIGHT:
      setCursor( Qt::SizeHorCursor );
      break;
    default:
      setCursor( Qt::ArrowCursor );
  }
}

void Agenda::setNoActionCursor( AgendaItem *moveItem, const QPoint &viewportPos )
{
//  kDebug() << "viewportPos:" << viewportPos.x() << "," << viewportPos.y();
//  QPoint point = viewport()->mapToGlobal( viewportPos );
//  kDebug() << "Global:" << point.x() << "," << point.y();
//  point = clipper()->mapFromGlobal( point );
//  kDebug() << "clipper:" << point.x() << "," << point.y();

  QPoint pos =  viewportPos ;
  const Item item = moveItem ? moveItem->incidence() : Item();

  const bool noResize = Akonadi::hasTodo( item );

  Agenda::MouseActionType resizeType = MOVE;
  if ( !noResize ) {
    resizeType = isInResizeArea( d->mAllDayMode, pos, moveItem );
  }
  setActionCursor( resizeType );
}

/** calculate the width of the column subcells of the given item
*/
double Agenda::calcSubCellWidth( AgendaItem *item )
{
  QPoint pt, pt1;
  pt = gridToContents( QPoint( item->cellXLeft(), item->cellYTop() ) );
  pt1 = gridToContents( QPoint( item->cellXLeft(), item->cellYTop() ) + QPoint( 1, 1 ) );
  pt1 -= pt;
  int maxSubCells = item->subCells();
  double newSubCellWidth;
  if ( d->mAllDayMode ) {
    newSubCellWidth = double( pt1.y() ) / maxSubCells;
  } else {
    newSubCellWidth = double( pt1.x() ) / maxSubCells;
  }
  return newSubCellWidth;
}

void Agenda::adjustItemPosition( AgendaItem *item )
{
  if ( !item ) {
    return;
  }
  item->resize( int( d->mGridSpacingX * item->cellWidth() ),
                int( d->mGridSpacingY * item->cellHeight() ) );
  int clXLeft = item->cellXLeft();
  if ( QApplication::isRightToLeft() ) {
    clXLeft = item->cellXRight() + 1;
  }
  QPoint cpos = gridToContents( QPoint( clXLeft, item->cellYTop() ) );
  item->move( cpos.x(), cpos.y() );
}

void Agenda::placeAgendaItem( AgendaItem *item, double subCellWidth )
{
  // "left" upper corner, no subcells yet, RTL layouts have right/left
  // switched, widths are negative then
  QPoint pt = gridToContents( QPoint( item->cellXLeft(), item->cellYTop() ) );
  // right lower corner
  QPoint pt1 = gridToContents(
    QPoint( item->cellXLeft() + item->cellWidth(), item->cellYBottom() + 1 ) );

  double subCellPos = item->subCell() * subCellWidth;

  // we need to add 0.01 to make sure we don't loose one pixed due to numerics
  // (i.e. if it would be x.9998, we want the integer, not rounded down.
  double delta = 0.01;
  if ( subCellWidth < 0 ) {
    delta = -delta;
  }
  int height, width, xpos, ypos;
  if ( d->mAllDayMode ) {
    width = pt1.x() - pt.x();
    height = int( subCellPos + subCellWidth + delta ) - int( subCellPos );
    xpos = pt.x();
    ypos = pt.y() + int( subCellPos );
  } else {
    width = int( subCellPos + subCellWidth + delta ) - int( subCellPos );
    height = pt1.y() - pt.y();
    xpos = pt.x() + int( subCellPos );
    ypos = pt.y();
  }
  if ( QApplication::isRightToLeft() ) { // RTL language/layout
    xpos += width;
    width = -width;
  }
  if ( height < 0 ) { // BTT (bottom-to-top) layout ?!?
    ypos += height;
    height = -height;
  }
  item->resize( width, height );
  item->move( xpos, ypos );
}

/*
  Place item in cell and take care that multiple items using the same cell do
  not overlap. This method is not yet optimal. It doesn't use the maximum space
  it can get in all cases.
  At the moment the method has a bug: When an item is placed only the sub cell
  widths of the items are changed, which are within the Y region the item to
  place spans. When the sub cell width change of one of this items affects a
  cell, where other items are, which do not overlap in Y with the item to
  place, the display gets corrupted, although the corruption looks quite nice.
*/
void Agenda::placeSubCells( AgendaItem *placeItem )
{
#if 0
  kDebug();
  if ( placeItem ) {
    Incidence *event = placeItem->incidence();
    if ( !event ) {
      kDebug() << "  event is 0";
    } else {
      kDebug() << "  event:" << event->summary();
    }
  } else {
    kDebug() << "  placeItem is 0";
  }
  kDebug() << "Agenda::placeSubCells()...";
#endif

  QList<CellItem*> cells;
  foreach ( CellItem *item, d->mItems ) {
    cells.append( item );
  }

  QList<CellItem*> items = CellItem::placeItem( cells, placeItem );

  placeItem->setConflictItems( QList<AgendaItem*>() );
  double newSubCellWidth = calcSubCellWidth( placeItem );
  QList<CellItem*>::iterator it;
  for ( it = items.begin(); it != items.end(); ++it ) {
    AgendaItem *item = static_cast<AgendaItem *>( *it );
    placeAgendaItem( item, newSubCellWidth );
    item->addConflictItem( placeItem );
    placeItem->addConflictItem( item );
  }
  if ( items.isEmpty() ) {
    placeAgendaItem( placeItem, newSubCellWidth );
  }
  placeItem->update();
}

int Agenda::columnWidth( int column ) const
{
  int start = gridToContents( QPoint( column, 0 ) ).x();
  if ( QApplication::isRightToLeft() ) {
    column--;
  } else {
    column++;
  }
  int end = gridToContents( QPoint( column, 0 ) ).x();
  return end - start;
}

void Agenda::paintEvent( QPaintEvent * )
{
  QPainter p( this );
  drawContents( &p, 0, 0, d->mGridSpacingX * d->mColumns, d->mGridSpacingY * d->mRows );
}

/*
  Draw grid in the background of the agenda.
*/
void Agenda::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
  QPixmap db( cw, ch );
  db.fill(); // We don't want to see leftovers from previous paints
  QPainter dbp( &db );
  // TODO: CHECK THIS
//  if ( ! mEventView->preferences()->agendaGridBackgroundImage().isEmpty() ) {
//    QPixmap bgImage( mEventView->preferences()->agendaGridBackgroundImage() );
//    dbp.drawPixmap( 0, 0, cw, ch, bgImage ); FIXME
//  }

  dbp.translate( -cx, -cy );

  double lGridSpacingY = d->mGridSpacingY * 2;

  // Highlight working hours
  if ( d->mWorkingHoursEnable && d->mHolidayMask ) {
    QPoint pt1( cx, d->mWorkingHoursYTop );
    QPoint pt2( cx + cw, d->mWorkingHoursYBottom );
    if ( pt2.x() >= pt1.x() /*&& pt2.y() >= pt1.y()*/) {
      int gxStart = contentsToGrid( pt1 ).x();
      int gxEnd = contentsToGrid( pt2 ).x();
      // correct start/end for rtl layouts
      if ( gxStart > gxEnd ) {
        int tmp = gxStart;
        gxStart = gxEnd;
        gxEnd = tmp;
      }
      int xoffset = ( QApplication::isRightToLeft() ? 1 : 0 );
      while ( gxStart <= gxEnd ) {
        int xStart = gridToContents( QPoint( gxStart + xoffset, 0 ) ).x();
        int xWidth = columnWidth( gxStart ) + 1;
        if ( pt2.y() < pt1.y() ) {
          // overnight working hours
          if ( ( ( gxStart == 0 ) && !d->mHolidayMask->at( d->mHolidayMask->count() - 1 ) ) ||
               ( ( gxStart > 0 ) && ( gxStart < int( d->mHolidayMask->count() ) ) &&
                 ( !d->mHolidayMask->at( gxStart - 1 ) ) ) ) {
            if ( pt2.y() > cy ) {
              dbp.fillRect( xStart, cy, xWidth, pt2.y() - cy + 1,
                            d->mEventView->preferences()->agendaGridWorkHoursBackgroundColor() );
            }
          }
          if ( ( gxStart < int( d->mHolidayMask->count() - 1 ) ) &&
               ( !d->mHolidayMask->at( gxStart ) ) ) {
            if ( pt1.y() < cy + ch - 1 ) {
              dbp.fillRect( xStart, pt1.y(), xWidth, cy + ch - pt1.y() + 1,
                            d->mEventView->preferences()->agendaGridWorkHoursBackgroundColor() );
            }
          }
        } else {
          // last entry in holiday mask denotes the previous day not visible
          // (needed for overnight shifts)
          if ( gxStart < int( d->mHolidayMask->count() - 1 ) && !d->mHolidayMask->at( gxStart ) ) {
            dbp.fillRect( xStart, pt1.y(), xWidth, pt2.y() - pt1.y() + 1,
                          d->mEventView->preferences()->agendaGridWorkHoursBackgroundColor() );
          }
        }
        ++gxStart;
      }
    }
  }

  // draw selection
  if ( d->mHasSelection ) {
    QPoint pt, pt1;

    if ( d->mSelectionEndCell.x() > d->mSelectionStartCell.x() ) { // multi day selection
      // draw start day
      pt = gridToContents( d->mSelectionStartCell );
      pt1 = gridToContents( QPoint( d->mSelectionStartCell.x() + 1, d->mRows + 1 ) );
      dbp.fillRect( QRect( pt, pt1 ), d->mEventView->preferences()->agendaGridHighlightColor() );
      // draw all other days between the start day and the day of the selection end
      for ( int c = d->mSelectionStartCell.x() + 1; c < d->mSelectionEndCell.x(); ++c ) {
        pt = gridToContents( QPoint( c, 0 ) );
        pt1 = gridToContents( QPoint( c + 1, d->mRows + 1 ) );
        dbp.fillRect( QRect( pt, pt1 ), d->mEventView->preferences()->agendaGridHighlightColor() );
      }
      // draw end day
      pt = gridToContents( QPoint( d->mSelectionEndCell.x(), 0 ) );
      pt1 = gridToContents( d->mSelectionEndCell + QPoint( 1, 1 ) );
      dbp.fillRect( QRect( pt, pt1 ), d->mEventView->preferences()->agendaGridHighlightColor() );
    } else { // single day selection
      pt = gridToContents( d->mSelectionStartCell );
      pt1 = gridToContents( d->mSelectionEndCell + QPoint( 1, 1 ) );
      dbp.fillRect( QRect( pt, pt1 ), d->mEventView->preferences()->agendaGridHighlightColor() );
    }
  }

  QPen hourPen( d->mEventView->preferences()->agendaGridBackgroundColor().dark( 150 ) );
  QPen halfHourPen( d->mEventView->preferences()->agendaGridBackgroundColor().dark( 125 ) );
  dbp.setPen( hourPen );

  // Draw vertical lines of grid, start with the last line not yet visible
  double x = ( int( cx / d->mGridSpacingX ) ) * d->mGridSpacingX;
  while ( x < cx + cw ) {
    dbp.drawLine( int( x ), cy, int( x ), cy + ch );
    x += d->mGridSpacingX;
  }

  // Draw horizontal lines of grid
  double y = ( int( cy / ( 2 * lGridSpacingY ) ) ) * 2 * lGridSpacingY;
  while ( y < cy + ch ) {
    dbp.drawLine( cx, int( y ), cx + cw, int( y ) );
    y += 2 * lGridSpacingY;
  }
  y = ( 2 * int( cy / ( 2 * lGridSpacingY ) ) + 1 ) * lGridSpacingY;
  dbp.setPen( halfHourPen );
  while ( y < cy + ch ) {
    dbp.drawLine( cx, int( y ), cx + cw, int( y ) );
    y += 2 * lGridSpacingY;
  }
  p->drawPixmap( cx, cy, db );
}

/*
  Convert srcollview contents coordinates to agenda grid coordinates.
*/
QPoint Agenda::contentsToGrid ( const QPoint &pos ) const
{
  int gx = int( QApplication::isRightToLeft() ?
                d->mColumns - pos.x() / d->mGridSpacingX : pos.x() / d->mGridSpacingX );
  int gy = int( pos.y() / d->mGridSpacingY );
  return QPoint( gx, gy );
}

/*
  Convert agenda grid coordinates to scrollview contents coordinates.
*/
QPoint Agenda::gridToContents( const QPoint &gpos ) const
{
  int x = int( QApplication::isRightToLeft() ?
               ( d->mColumns - gpos.x() ) * d->mGridSpacingX : gpos.x() * d->mGridSpacingX );
  int y = int( gpos.y() * d->mGridSpacingY );
  return QPoint( x, y );
}

/*
  Return Y coordinate corresponding to time. Coordinates are rounded to
  fit into the grid.
*/
int Agenda::timeToY( const QTime &time ) const
{
//  kDebug() << "Time:" << time.toString();
  int minutesPerCell = 24 * 60 / d->mRows;
//  kDebug() << "minutesPerCell:" << minutesPerCell;
  int timeMinutes = time.hour() * 60 + time.minute();
//  kDebug() << "timeMinutes:" << timeMinutes;
  int Y = ( timeMinutes + ( minutesPerCell / 2 ) ) / minutesPerCell;
//  kDebug() << "y:" << Y;
  return Y;
}

/*
  Return time corresponding to cell y coordinate. Coordinates are rounded to
  fit into the grid.
*/
QTime Agenda::gyToTime( int gy ) const
{
//  kDebug() << gy;
  int secondsPerCell = 24 * 60 * 60 / d->mRows;
  int timeSeconds = secondsPerCell * gy;

  QTime time( 0, 0, 0 );
  if ( timeSeconds < 24 * 60 * 60 ) {
    time = time.addSecs(timeSeconds);
  } else {
    time.setHMS( 23, 59, 59 );
  }
  return time;
}

QVector<int> Agenda::minContentsY() const
{
  QVector<int> minArray;
  minArray.fill( timeToY( QTime( 23, 59 ) ), d->mSelectedDates.count() );
  foreach ( AgendaItem *item, d->mItems ) {
    int ymin = item->cellYTop();
    int index = item->cellXLeft();
    if ( index >= 0 && index < (int)( d->mSelectedDates.count() ) ) {
      if ( ymin < minArray[index] && !d->mItemsToDelete.contains( item ) ) {
        minArray[index] = ymin;
      }
    }
  }

  return minArray;
}

QVector<int> Agenda::maxContentsY() const
{
  QVector<int> maxArray;
  maxArray.fill( timeToY( QTime( 0, 0 ) ), d->mSelectedDates.count() );
  foreach ( AgendaItem *item, d->mItems ) {
    int ymax = item->cellYBottom();
    int index = item->cellXLeft();
    if ( index >= 0 && index < (int)( d->mSelectedDates.count() ) ) {
      if ( ymax > maxArray[index] && !d->mItemsToDelete.contains( item ) ) {
        maxArray[index] = ymax;
      }
    }
  }

  return maxArray;
}

void Agenda::setStartTime( const QTime &startHour )
{
  double startPos =
    ( startHour.hour() / 24. + startHour.minute() / 1440. + startHour.second() / 86400. ) *
    d->mRows * gridSpacingY();
  setContentsPos( 0, int( startPos ) );
}

/*
  Insert AgendaItem into agenda.
*/
AgendaItem *Agenda::insertItem( const Item &incidence, const QDate &qd,
                                int X, int YTop, int YBottom )
{
  if ( d->mAllDayMode ) {
    kDebug() << "using this in all-day mode is illegal.";
    return 0;
  }

  d->mActionType = NOP;

  AgendaItem *agendaItem = new AgendaItem( d->mEventView, d->mCalendar, incidence, qd, this );
  connect( agendaItem, SIGNAL(removeAgendaItem(AgendaItem *)),
           SLOT(removeAgendaItem(AgendaItem *)) );
  connect( agendaItem, SIGNAL(showAgendaItem(AgendaItem *)),
           SLOT(showAgendaItem(AgendaItem *)) );

  if ( YBottom <= YTop ) {
    kDebug() << "Text:" << agendaItem->text() << " YSize<0";
    YBottom = YTop;
  }

  agendaItem->resize( int( ( X + 1 ) * d->mGridSpacingX ) -
                      int( X * d->mGridSpacingX ),
                      int( YTop * d->mGridSpacingY ) -
                      int( ( YBottom + 1 ) * d->mGridSpacingY ) );
  agendaItem->setCellXY( X, YTop, YBottom );
  agendaItem->setCellXRight( X );
  agendaItem->setResourceColor( EventViews::resourceColor( incidence, d->mEventView->preferences() ) );
  agendaItem->installEventFilter( this );

  agendaItem->move( int( X * d->mGridSpacingX ), int( YTop * d->mGridSpacingY ) );

  d->mItems.append( agendaItem );

  placeSubCells( agendaItem );

  agendaItem->show();

  marcus_bains();

  return agendaItem;
}

/*
  Insert all-day AgendaItem into agenda.
*/
AgendaItem *Agenda::insertAllDayItem( const Item &incidence, const QDate &qd,
                                      int XBegin, int XEnd )
{
  if ( !d->mAllDayMode ) {
    kDebug() << "using this in non all-day mode is illegal.";
    return 0;
  }

  d->mActionType = NOP;

  AgendaItem *agendaItem = new AgendaItem( d->mEventView, d->mCalendar, incidence, qd, this );
  connect( agendaItem, SIGNAL(removeAgendaItem(AgendaItem *)),
           SLOT(removeAgendaItem(AgendaItem *)) );
  connect( agendaItem, SIGNAL(showAgendaItem(AgendaItem *)),
           SLOT(showAgendaItem(AgendaItem *)) );

  agendaItem->setCellXY( XBegin, 0, 0 );
  agendaItem->setCellXRight( XEnd );

  double startIt = d->mGridSpacingX * ( agendaItem->cellXLeft() );
  double endIt = d->mGridSpacingX * ( agendaItem->cellWidth() +
                                      agendaItem->cellXLeft() );

  agendaItem->resize( int( endIt ) - int( startIt ), int( d->mGridSpacingY ) );

  agendaItem->installEventFilter( this );
  agendaItem->setResourceColor( EventViews::resourceColor( incidence, d->mEventView->preferences() ) );
  agendaItem->move( int( XBegin * d->mGridSpacingX ), 0 ) ;
  d->mItems.append( agendaItem );

  placeSubCells( agendaItem );

  agendaItem->show();

  return agendaItem;
}

void Agenda::insertMultiItem( const Item &event, const QDate &qd, int XBegin,
                              int XEnd, int YTop, int YBottom )
{
  Event::Ptr ev = Akonadi::event( event );
  Q_ASSERT( ev );
  if ( d->mAllDayMode ) {
    kDebug() << "using this in all-day mode is illegal.";
    return;
  }

  d->mActionType = NOP;
  int cellX, cellYTop, cellYBottom;
  QString newtext;
  int width = XEnd - XBegin + 1;
  int count = 0;
  AgendaItem *current = 0;
  QList<AgendaItem*> multiItems;
  int visibleCount = d->mSelectedDates.first().daysTo( d->mSelectedDates.last() );
  for ( cellX = XBegin; cellX <= XEnd; ++cellX ) {
    ++count;
    //Only add the items that are visible.
    if( cellX >=0 && cellX <= visibleCount ) {
      if ( cellX == XBegin ) {
        cellYTop = YTop;
      } else {
        cellYTop = 0;
      }
      if ( cellX == XEnd ) {
        cellYBottom = YBottom;
      } else {
        cellYBottom = rows() - 1;
      }
      newtext = QString( "(%1/%2): " ).arg( count ).arg( width );
      newtext.append( ev->summary() );

      current = insertItem( event, qd, cellX, cellYTop, cellYBottom );
      current->setText( newtext );
      multiItems.append( current );
    }
  }

  QList<AgendaItem*>::iterator it = multiItems.begin();
  QList<AgendaItem*>::iterator e = multiItems.end();

  if ( it != e ) { // .first asserts if the list is empty
    AgendaItem *first = multiItems.first();
    AgendaItem *last = multiItems.last();
    AgendaItem *prev = 0, *next = 0;

    while ( it != e ) {
      AgendaItem *item = *it;
      ++it;
      next = ( it == e ) ? 0 : (*it);
      if ( item ) {
        item->setMultiItem( ( item == first ) ? 0 : first,
                            prev, next,
                            ( item == last ) ? 0 : last );
      }
      prev = item;
    }
  }

  marcus_bains();
}

QList<AgendaItem*> Agenda::agendaItems( const Akonadi::Item &aitem ) const
{
  QList<AgendaItem*> items;
  Q_FOREACH ( AgendaItem * const item, d->mItems ) {
    if ( item && item->incidence() == aitem ) {
      items.push_back( item );
    }
  }
  return items;
}

void Agenda::removeIncidence( const Item &incidence )
{
  // First find all items to be deleted and store them
  // in its own list. Otherwise removeAgendaItem will reset
  // the current position in the iterator-loop and mess the logic up.
  const QList<AgendaItem*> itemsToRemove = agendaItems( incidence );

  foreach ( AgendaItem * const item, itemsToRemove ) {
    removeAgendaItem( item );
  }
}

void Agenda::showAgendaItem( AgendaItem *agendaItem )
{
  if ( !agendaItem ) {
    return;
  }

  agendaItem->hide();

  agendaItem->setParent( this );

  if ( !d->mItems.contains( agendaItem ) ) {
    d->mItems.append( agendaItem );
  }
  placeSubCells( agendaItem );

  agendaItem->show();
}

bool Agenda::removeAgendaItem( AgendaItem *item )
{
  // we found the item. Let's remove it and update the conflicts
  bool taken = false;
  AgendaItem *thisItem = item;
  QList<AgendaItem*> conflictItems = thisItem->conflictItems();
//  removeChild( thisItem );

  taken = ( d->mItems.removeAll( thisItem ) > 0 );

  QList<AgendaItem*>::iterator it;
  for ( it = conflictItems.begin(); it != conflictItems.end(); ++it ) {
      (*it)->setSubCells( ( *it )->subCells()-1 );
  }

  for ( it = conflictItems.begin(); it != conflictItems.end(); ++it ) {
    // the item itself is also in its own conflictItems list!
    if ( *it != thisItem ) {
      placeSubCells( *it );
    }
  }
  d->mItemsToDelete.append( thisItem );
  QTimer::singleShot( 0, this, SLOT(deleteItemsToDelete()) );
  return taken;
}

void Agenda::deleteItemsToDelete()
{
  qDeleteAll( d->mItemsToDelete );
  d->mItemsToDelete.clear();
}

/*QSizePolicy Agenda::sizePolicy() const
{
  // Thought this would make the all-day event agenda minimum size and the
  // normal agenda take the remaining space. But it doesn't work. The QSplitter
  // don't seem to think that an Expanding widget needs more space than a
  // Preferred one.
  // But it doesn't hurt, so it stays.
  if (mAllDayMode) {
    return QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
  } else {
  return QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  }
}*/


/*
  Overridden from QScrollView to provide proper resizing of AgendaItems.
*/
void Agenda::resizeEvent ( QResizeEvent *ev )
{

  QSize newSize( ev->size() );

  if ( d->mAllDayMode ) {
    d->mGridSpacingX = static_cast<double>( newSize.width() ) / d->mColumns;
    d->mGridSpacingY = newSize.height();
  } else {
    d->mGridSpacingX = static_cast<double>( newSize.width() ) / d->mColumns;
    // make sure that there are not more than 24 per day
    d->mGridSpacingY = static_cast<double>( newSize.height() ) / d->mRows;
    if ( d->mGridSpacingY < d->mDesiredGridSpacingY ) {
      d->mGridSpacingY = d->mDesiredGridSpacingY;
    }
  }
  calculateWorkingHours();

  QTimer::singleShot( 0, this, SLOT(resizeAllContents()) );
  emit gridSpacingYChanged( d->mGridSpacingY * 4 );

  QWidget::resizeEvent( ev );
  updateGeometry();
}

void Agenda::resizeAllContents()
{
  double subCellWidth;
  AgendaItem *item;
  if ( d->mAllDayMode ) {
    foreach ( item, d->mItems ) {
      subCellWidth = calcSubCellWidth( item );
      placeAgendaItem( item, subCellWidth );
    }
  } else {
    foreach ( item, d->mItems ) {
      subCellWidth = calcSubCellWidth( item );
      placeAgendaItem( item, subCellWidth );
    }
  }
  checkScrollBoundaries();
  marcus_bains();
  repaint();
}

void Agenda::scrollUp()
{
  d->mScrollArea->verticalScrollBar()->scroll( 0, -d->mScrollOffset );
}

void Agenda::scrollDown()
{
  d->mScrollArea->verticalScrollBar()->scroll( 0, d->mScrollOffset );
}

QSize Agenda::minimumSize() const
{
  return sizeHint();
}

QSize Agenda::minimumSizeHint() const
{
  return sizeHint();
}

int Agenda::minimumHeight() const
{
  // all day agenda never has scrollbars and the scrollarea will
  // resize it to fit exactly on the viewport.

  if ( d->mAllDayMode ) {
    return 0;
  } else {
    return d->mGridSpacingY * d->mRows;
  }
}

void Agenda::updateConfig()
{
  double oldGridSpacingY = d->mGridSpacingY;

  d->mDesiredGridSpacingY = d->mEventView->preferences()->hourSize();
  if ( d->mDesiredGridSpacingY < 4 || d->mDesiredGridSpacingY > 30 ) {
    d->mDesiredGridSpacingY = 10;
  }

  // make sure that there are not more than 24 per day
  d->mGridSpacingY = static_cast<double>( height() ) / d->mRows;
  if ( d->mGridSpacingY < d->mDesiredGridSpacingY ) {
    d->mGridSpacingY = d->mDesiredGridSpacingY;
  }

  //can be two doubles equal?, it's better to compare them with an epsilon
  if ( fabs( oldGridSpacingY - d->mGridSpacingY ) > 0.1 ) {
//    resizeContents( int( mGridSpacingX * mColumns ), int( mGridSpacingY * mRows ) );
  }

  calculateWorkingHours();

  marcus_bains();
}

void Agenda::checkScrollBoundaries()
{
  // Invalidate old values to force update
  d->mOldLowerScrollValue = -1;
  d->mOldUpperScrollValue = -1;

  checkScrollBoundaries( verticalScrollBar()->value() );
}

void Agenda::checkScrollBoundaries( int v )
{
  int yMin = int( (v) / d->mGridSpacingY );
  int yMax = int( ( v + d->mScrollArea->height() ) / d->mGridSpacingY );

  if ( yMin != d->mOldLowerScrollValue ) {
    d->mOldLowerScrollValue = yMin;
    emit lowerYChanged( yMin );
  }
  if ( yMax != d->mOldUpperScrollValue ) {
    d->mOldUpperScrollValue = yMax;
    emit upperYChanged( yMax );
  }
}

int Agenda::visibleContentsYMin()
{
  int v = verticalScrollBar()->value();
  return int( v / d->mGridSpacingY );
}

int Agenda::visibleContentsYMax()
{
  int v = verticalScrollBar()->value();
  return int( ( v + d->mScrollArea->height() ) / d->mGridSpacingY );
}

void Agenda::deselectItem()
{
  if ( d->mSelectedItem.isNull() ) {
    return;
  }

  const Item selectedItem = d->mSelectedItem->incidence();

  foreach ( AgendaItem *item, d->mItems ) {
    const Item itemInc = item->incidence();
    if( itemInc.isValid() && selectedItem.isValid() && itemInc.id() == selectedItem.id() ) {
      item->select( false );
    }
  }

  d->mSelectedItem = 0;
}

void Agenda::selectItem( AgendaItem *item )
{
  if ( (AgendaItem *)d->mSelectedItem == item ) {
    return;
  }
  deselectItem();
  if ( item == 0 ) {
    emit incidenceSelected( Item(), QDate() );
    return;
  }
  d->mSelectedItem = item;
  d->mSelectedItem->select();
  Q_ASSERT( Akonadi::hasIncidence( d->mSelectedItem->incidence() ) );
  d->mSelectedId = d->mSelectedItem->incidence().id();

  foreach ( AgendaItem *item, d->mItems ) {
    if( item->incidence().id() == d->mSelectedId ) {
      item->select();
    }
  }
  emit incidenceSelected( d->mSelectedItem->incidence(), d->mSelectedItem->itemDate() );
}

void Agenda::selectItemByItemId( const Item::Id &id )
{
  foreach ( AgendaItem *item, d->mItems ) {
    if( item->incidence().id() == id ) {
      selectItem( item );
      break;
    }
  }
}

void Agenda::selectItem( const Item &item )
{
  selectItemByItemId( item.id() );
}

// This function seems never be called.
void Agenda::keyPressEvent( QKeyEvent *kev )
{
  switch( kev->key() ) {
  case Qt::Key_PageDown:
    verticalScrollBar()->triggerAction( QAbstractSlider::SliderPageStepAdd );
    break;
  case Qt::Key_PageUp:
    verticalScrollBar()->triggerAction( QAbstractSlider::SliderPageStepSub );
    break;
  case Qt::Key_Down:
    verticalScrollBar()->triggerAction( QAbstractSlider::SliderSingleStepAdd );
    break;
  case Qt::Key_Up:
    verticalScrollBar()->triggerAction( QAbstractSlider::SliderSingleStepSub );
    break;
  default:
    ;
  }
}

void Agenda::calculateWorkingHours()
{
  d->mWorkingHoursEnable = !d->mAllDayMode;

  QTime tmp = d->mEventView->preferences()->workingHoursStart().time();
  d->mWorkingHoursYTop = int( 4 * d->mGridSpacingY *
                              ( tmp.hour() + tmp.minute() / 60. +
                                tmp.second() / 3600. ) );
  tmp = d->mEventView->preferences()->workingHoursEnd().time();
  d->mWorkingHoursYBottom = int( 4 * d->mGridSpacingY *
                                 ( tmp.hour() + tmp.minute() / 60. +
                                   tmp.second() / 3600. ) - 1 );
}

void Agenda::setDateList( const DateList &selectedDates )
{
  d->mSelectedDates = selectedDates;
  marcus_bains();
}

DateList Agenda::dateList() const
{
  return d->mSelectedDates;
}

void Agenda::setCalendar( Akonadi::Calendar *cal )
{
  d->mCalendar = cal;
}

void Agenda::setIncidenceChanger( Akonadi::IncidenceChanger *changer )
{
  d->mChanger = changer;
}

void Agenda::setHolidayMask( QVector<bool> *mask )
{
  d->mHolidayMask = mask;
}

void Agenda::contentsMousePressEvent ( QMouseEvent *event )
{
  Q_UNUSED( event );
}

QSize Agenda::sizeHint() const
{
  if ( d->mAllDayMode ) {
    return QWidget::sizeHint();
  } else {
    return QSize( parentWidget()->width(), d->mGridSpacingY * d->mRows );
  }
}

QScrollBar * Agenda::verticalScrollBar()
{
  return d->mScrollArea->verticalScrollBar();
}

void Agenda::setContentsPos( int x, int y )
{
  d->mScrollArea->ensureVisible( x, y, 0, 0 );
}

AgendaScrollArea::AgendaScrollArea( bool isAllDay, EventView *eventView, QWidget *parent )
  : QScrollArea( parent )
{
  if ( isAllDay ) {
    mAgenda = new Agenda( eventView, this, 1 );
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  } else {
    mAgenda = new Agenda( eventView, this, 1, 96, eventView->preferences()->hourSize() );
  }

  setWidgetResizable( true );
  setWidget( mAgenda );
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  mAgenda->setStartTime( eventView->preferences()->dayBegins().time() );
}

AgendaScrollArea::~AgendaScrollArea()
{
}

Agenda *AgendaScrollArea::agenda()
{
  return mAgenda;
}

#include "agenda.moc"
// kate: space-indent on; indent-width 2; replace-tabs on;

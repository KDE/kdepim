/*
  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>

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

#include "monthscene.h"
#include "monthgraphicsitems.h"
#include "monthitem.h"
#include "monthview.h"
#include "prefs.h"

#include <calendarsupport/utils.h>

#include <KCalendarSystem>
#include <KIconLoader>

#include <QGraphicsSceneMouseEvent>
#include <QResizeEvent>

static const int AUTO_REPEAT_DELAY = 600;

using namespace EventViews;

MonthScene::MonthScene( MonthView *parent )
  : QGraphicsScene( parent ),
    mMonthView( parent ),
    mInitialized( false ),
    mClickedItem( 0 ),
    mActionItem( 0 ),
    mActionInitiated( false ),
    mSelectedItem( 0 ),
    mStartCell( 0 ),
    mPreviousCell( 0 ),
    mActionType( None ),
    mStartHeight( 0 ),
    mCurrentIndicator( 0 )
{
  mBirthdayPixmap  = SmallIcon( QLatin1String("view-calendar-birthday") );
  mAnniversaryPixmap = SmallIcon( QLatin1String("view-calendar-wedding-anniversary") );
  mAlarmPixmap     = SmallIcon( QLatin1String("appointment-reminder" ));
  mRecurPixmap     = SmallIcon( QLatin1String("appointment-recurring") );
  mReadonlyPixmap  = SmallIcon( QLatin1String("object-locked") );
  mReplyPixmap     = SmallIcon( QLatin1String("mail-reply-sender") );
  mHolidayPixmap   = SmallIcon( QLatin1String("view-calendar-holiday") );

  setSceneRect( 0, 0, parent->width(), parent->height() );
}

MonthScene::~MonthScene()
{
  qDeleteAll( mMonthCellMap );
  qDeleteAll( mManagerList );
}

MonthCell *MonthScene::selectedCell() const
{
  return mMonthCellMap.value( mSelectedCellDate );
}

MonthCell *MonthScene::previousCell() const
{
  return mPreviousCell;
}

int MonthScene::getRightSpan( const QDate &date ) const
{
  MonthCell *cell = mMonthCellMap.value( date );
  if ( !cell ) {
    return 0;
  }

  return 7 - cell->x() - 1;
}

int MonthScene::getLeftSpan( const QDate &date ) const
{
  MonthCell *cell = mMonthCellMap.value( date );
  if ( !cell ) {
    return 0;
  }

  return cell->x();
}

int MonthScene::maxRowCount()
{
  return ( rowHeight() - MonthCell::topMargin() ) / itemHeightIncludingSpacing();
}

int MonthScene::itemHeightIncludingSpacing()
{
  return MonthCell::topMargin() + 2;
}

int MonthScene::itemHeight()
{
  return MonthCell::topMargin();
}

MonthCell *MonthScene::firstCellForMonthItem( MonthItem *manager )
{
  for ( QDate d = manager->startDate(); d <= manager->endDate(); d = d.addDays( 1 ) ) {
    MonthCell *monthCell = mMonthCellMap.value( d );
    if ( monthCell ) {
      return monthCell;
    }
  }

  return 0;
}

void MonthScene::updateGeometry()
{
  foreach ( MonthItem *manager, mManagerList ) {
    manager->updateGeometry();
  }
}

int MonthScene::availableWidth() const
{
  return static_cast<int> ( sceneRect().width() );
}

int MonthScene::availableHeight() const
{
  return static_cast<int> ( sceneRect().height() - headerHeight() );
}

int MonthScene::columnWidth() const
{
  return static_cast<int> ( ( availableWidth() - 1 ) / 7. );
}

int MonthScene::rowHeight() const
{
  return static_cast<int> ( ( availableHeight() - 1 ) / 6. );
}

int MonthScene::headerHeight() const
{
  return 50;
}

int MonthScene::cellVerticalPos( const MonthCell *cell ) const
{
  return headerHeight() + cell->y() * rowHeight();
}

int MonthScene::cellHorizontalPos( const MonthCell *cell ) const
{
  return cell->x() * columnWidth();
}

int MonthScene::sceneYToMonthGridY( int yScene )
{
  return yScene - headerHeight();
}

int MonthScene::sceneXToMonthGridX( int xScene )
{
  return xScene;
}

void MonthGraphicsView::drawBackground( QPainter *p, const QRectF & rect )
{
  Q_ASSERT( mScene );
  PrefsPtr prefs = mScene->monthView()->preferences();
  p->setFont( prefs->monthViewFont() );
  p->fillRect( rect, palette().color( QPalette::Base ) );

  /*
    Headers
  */
  QFont font = prefs->monthViewFont();
  font.setBold( true );
  font.setPointSize( 15 );
  p->setFont( font );
  const int dayLabelsHeight = 20;
  const KCalendarSystem *calSys = KGlobal::locale()->calendar();
  p->drawText( QRect( 0,  0, // top right
                      static_cast<int> ( mScene->sceneRect().width() ),
                      static_cast<int> ( mScene->headerHeight() - dayLabelsHeight ) ),
               Qt::AlignCenter,
               i18nc( "monthname year", "%1 %2",
                      calSys->monthName( mMonthView->averageDate() ),
                      calSys->formatDate( mMonthView->averageDate(),
                                          KLocale::Year, KLocale::LongNumber ) ) );

  font.setPixelSize( dayLabelsHeight - 10 );
  p->setFont( font );

  const QDate start = mMonthView->actualStartDateTime().date();
  const QDate end = mMonthView->actualEndDateTime().date();

  for ( QDate d = start;
        d <= start.addDays( 6 ); d = d.addDays( 1 ) ) {
    MonthCell *cell = mScene->mMonthCellMap.value( d );

    if ( !cell ) {
      // This means drawBackground() is being called before reloadIncidences(). Can happen with some
      // themes. Bug  #190191
      return;
    }

    p->drawText( QRect( mScene->cellHorizontalPos( cell ),
                        mScene->cellVerticalPos( cell ) - 15,
                        mScene->columnWidth(),
                        15 ),
                 Qt::AlignCenter,
                 calSys->weekDayName( d, KCalendarSystem::LongDayName ) );
  }

  /*
    Month grid
  */
  int columnWidth = mScene->columnWidth();
  int rowHeight = mScene->rowHeight();

  const QList<QDate> workDays = CalendarSupport::workDays( mMonthView->actualStartDateTime().date(),
                                                           mMonthView->actualEndDateTime().date() );

  for ( QDate d = start; d <= end; d = d.addDays( 1 ) ) {
    if ( !mScene->mMonthCellMap.contains( d ) ) {
      // This means drawBackground() is being called before reloadIncidences(). Can happen with some
      // themes. Bug  #190191
      return;
    }

    MonthCell *cell = mScene->mMonthCellMap[ d ];

    QColor color;
    if ( workDays.contains( d ) ) {
      color = mMonthView->preferences()->monthGridWorkHoursBackgroundColor();
    } else {
      color = mMonthView->preferences()->monthGridBackgroundColor();
    }
    if ( cell == mScene->selectedCell() ) {
      color = color.dark( 115 );
    }
    if ( cell->date() == QDate::currentDate() ) {
      color = color.dark( 140 );
    }

    // Draw cell
    p->setPen( mMonthView->preferences()->monthGridBackgroundColor().dark( 150 ) );
    p->setBrush( color );
    p->drawRect( QRect( mScene->cellHorizontalPos( cell ), mScene->cellVerticalPos( cell ),
                        columnWidth, rowHeight ) );

    if ( mMonthView->isBusyDay( d ) ) {
      QColor busyColor = mMonthView->preferences()->viewBgBusyColor();
      busyColor.setAlpha( EventViews::BUSY_BACKGROUND_ALPHA );
      p->setBrush( busyColor );
      p->drawRect( QRect( mScene->cellHorizontalPos( cell ), mScene->cellVerticalPos( cell ),
                          columnWidth, rowHeight ) );
    }

    // Draw cell header
    int cellHeaderX = mScene->cellHorizontalPos( cell ) + 1;
    int cellHeaderY = mScene->cellVerticalPos( cell ) + 1;
    int cellHeaderWidth = columnWidth - 2;
    int cellHeaderHeight = cell->topMargin() - 2;
    QLinearGradient bgGradient( QPointF( cellHeaderX, cellHeaderY ),
                                QPointF( cellHeaderX + cellHeaderWidth,
                                         cellHeaderY + cellHeaderHeight ) );
    bgGradient.setColorAt( 0, color.dark( 105 ) );
    bgGradient.setColorAt( 0.7, color.dark( 105 ) );
    bgGradient.setColorAt( 1, color );
    p->setBrush( bgGradient );

    p->setPen( Qt::NoPen );
    p->drawRect( QRect( cellHeaderX, cellHeaderY,
                        cellHeaderWidth, cellHeaderHeight ) );
  }

  font = mMonthView->preferences()->monthViewFont();
  font.setPixelSize( MonthCell::topMargin() - 4 );

  p->setFont( font );

  QPen oldPen =  mMonthView->preferences()->monthGridBackgroundColor().dark( 150 );

  // Draw dates
  for ( QDate d = mMonthView->actualStartDateTime().date();
        d <= mMonthView->actualEndDateTime().date(); d = d.addDays( 1 ) ) {
    MonthCell *cell = mScene->mMonthCellMap.value( d );

    QFont font = p->font();
    if ( cell->date() == QDate::currentDate() ) {
      font.setBold( true );
    } else {
      font.setBold( false );
    }
    p->setFont( font );

    if ( d.month() == mMonthView->currentMonth() ) {
      p->setPen( QPalette::Text );
    } else {
      p->setPen( oldPen );
    }

    /*
      Draw arrows if all items won't fit
    */

    // Up arrow if first item is above cell top
    if ( mScene->startHeight() != 0 && cell->hasEventBelow( mScene->startHeight() ) ) {
      cell->upArrow()->setPos(
        mScene->cellHorizontalPos( cell ) + columnWidth / 2,
        mScene->cellVerticalPos( cell ) + cell->upArrow()->boundingRect().height() / 2 + 2 );
      cell->upArrow()->show();
    } else {
      cell->upArrow()->hide();
    }

    // Down arrow if last item is below cell bottom
    if ( !mScene->lastItemFit( cell ) ) {
      cell->downArrow()->setPos(
        mScene->cellHorizontalPos( cell ) + columnWidth / 2,
        mScene->cellVerticalPos( cell ) + rowHeight -
        cell->downArrow()->boundingRect().height() / 2 - 2 );
      cell->downArrow()->show();
    } else {
      cell->downArrow()->hide();
    }

    const KCalendarSystem *calSys = KGlobal::locale()->calendar();

    QString dayText;
    // Prepend month name if d is the first or last day of month
    if ( calSys->day( d ) == 1 ||                // d is the first day of month
         calSys->day( d.addDays( 1 ) ) == 1 ) {  // d is the last day of month
      dayText = i18nc( "'Month day' for month view cells", "%1 %2",
                  calSys->monthName( d, KCalendarSystem::ShortName ),
                  calSys->day( d ) );
    } else {
      dayText = QString::number( calSys->day( d ) );
    }

    p->drawText( QRect( mScene->cellHorizontalPos( cell ), // top right
                        mScene->cellVerticalPos( cell ),     // of the cell
                        mScene->columnWidth() - 2,
                        cell->topMargin() ),
                 Qt::AlignRight,
                 dayText );
  }

  // ...
}

void MonthScene::resetAll()
{
  qDeleteAll( mMonthCellMap );
  mMonthCellMap.clear();

  qDeleteAll( mManagerList );
  mManagerList.clear();

  mSelectedItem = 0;
  mActionItem = 0;
  mClickedItem = 0;
}

Akonadi::IncidenceChanger *MonthScene::incidenceChanger() const
{
  return mMonthView->changer();
}

QDate MonthScene::firstDateOnRow( int row ) const
{
  return mMonthView->actualStartDateTime().date().addDays( 7 * row );
}

bool MonthScene::lastItemFit( MonthCell *cell )
{
  if ( cell->firstFreeSpace() > maxRowCount() + startHeight() ) {
    return false;
  } else {
    return true;
  }
}

int MonthScene::totalHeight()
{
  int max = 0;
  for ( QDate d = mMonthView->actualStartDateTime().date();
        d <= mMonthView->actualEndDateTime().date(); d = d.addDays( 1 ) ) {
    int c = mMonthCellMap[ d ]->firstFreeSpace();
    if ( c > max ) {
      max = c;
    }
  }

  return max;
}

void MonthScene::wheelEvent( QGraphicsSceneWheelEvent *event )
{
  Q_UNUSED( event ); // until we figure out what to do in here

/*  int numDegrees = -event->delta() / 8;
  int numSteps = numDegrees / 15;

  if ( startHeight() + numSteps < 0 ) {
    numSteps = -startHeight();
  }

  int cellHeight = 0;

  MonthCell *currentCell = getCellFromPos( event->scenePos() );
  if ( currentCell ) {
    cellHeight = currentCell->firstFreeSpace();
  }
  if ( cellHeight == 0 ) {
    // no items in this cell, there's no point to scroll
    return;
  }

  int newHeight;
  int maxStartHeight = qMax( 0, cellHeight - maxRowCount() );
  if ( numSteps > 0  && startHeight() + numSteps >= maxStartHeight ) {
    newHeight = maxStartHeight;
  } else {
    newHeight = startHeight() + numSteps;
  }

  if ( newHeight == startHeight() ) {
    return;
  }

  setStartHeight( newHeight );

  foreach ( MonthItem *manager, mManagerList ) {
    manager->updateGeometry();
  }

  invalidate( QRectF(), BackgroundLayer );

  event->accept();
*/
}

void MonthScene::scrollCellsDown()
{
  int newHeight = startHeight() + 1;
  setStartHeight( newHeight );

  foreach ( MonthItem *manager, mManagerList ) {
    manager->updateGeometry();
  }

  invalidate( QRectF(), BackgroundLayer );
}

void MonthScene::scrollCellsUp()
{
  int newHeight = startHeight() - 1;
  setStartHeight( newHeight );

  foreach ( MonthItem *manager, mManagerList ) {
    manager->updateGeometry();
  }

  invalidate( QRectF(), BackgroundLayer );
}

void MonthScene::clickOnScrollIndicator( ScrollIndicator *scrollItem )
{
  if ( scrollItem->direction() == ScrollIndicator::UpArrow ) {
    scrollCellsUp();
  } else if ( scrollItem->direction() == ScrollIndicator::DownArrow ) {
    scrollCellsDown();
  }
}

void MonthScene::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent *mouseEvent )
{
  QPointF pos = mouseEvent->scenePos();
  repeatTimer.stop();
  MonthGraphicsItem *iItem = dynamic_cast<MonthGraphicsItem*>( itemAt( pos ) );
  if ( iItem ) {
    if ( iItem->monthItem() ) {
      IncidenceMonthItem *tmp = qobject_cast<IncidenceMonthItem *>( iItem->monthItem() );
      if ( tmp ) {
        selectItem( iItem->monthItem() );
        mMonthView->defaultAction( tmp->akonadiItem() );

        mouseEvent->accept();
      }
    }
  } else {
    emit newEventSignal();
  }
}

void MonthScene::mouseMoveEvent ( QGraphicsSceneMouseEvent *mouseEvent )
{
  QPointF pos = mouseEvent->scenePos();

  MonthGraphicsView *view = static_cast<MonthGraphicsView*>( views().first() );

  // Change cursor depending on the part of the item it hovers to inform
  // the user that he can resize the item.
  if ( mActionType == None ) {
    MonthGraphicsItem *iItem = dynamic_cast<MonthGraphicsItem*>( itemAt( pos ) );
    if ( iItem ) {
      if ( iItem->monthItem()->isResizable() &&
            iItem->isBeginItem() && iItem->mapFromScene( pos ).x() <= 10 ) {
        view->setActionCursor( Resize );
      } else if ( iItem->monthItem()->isResizable() &&
                    iItem->isEndItem() &&
                    iItem->mapFromScene( pos ).x() >= iItem->boundingRect().width() - 10 ) {
        view->setActionCursor( Resize );
      } else {
        view->setActionCursor( None );
      }
    } else {
      view->setActionCursor( None );
    }
    mouseEvent->accept();
    return;
  }

  // If an item was selected during the click, we maybe have an item to move !
  if ( mActionItem ) {
    MonthCell *currentCell = getCellFromPos( pos );

    // Initiate action if not already done
    if ( !mActionInitiated && mActionType != None ) {
      if ( mActionType == Move ) {
        mActionItem->beginMove();
      } else if ( mActionType == Resize ) {
        mActionItem->beginResize();
      }
      mActionInitiated = true;
    }
    view->setActionCursor( mActionType );

    // Move or resize action
    if ( currentCell && currentCell != mPreviousCell ) {

      bool ok = true;
      if ( mActionType == Move ) {
        mActionItem->moveBy( mPreviousCell->date().daysTo( currentCell->date() ) );
      } else if ( mActionType == Resize ) {
        ok = mActionItem->resizeBy( mPreviousCell->date().daysTo( currentCell->date() ) );
      }

      if ( ok ) {
        mPreviousCell = currentCell;
      }
      mActionItem->updateGeometry();
      update();
    }
    mouseEvent->accept();
  }
}

void MonthScene::mousePressEvent ( QGraphicsSceneMouseEvent *mouseEvent )
{
  QPointF pos = mouseEvent->scenePos();

  mClickedItem = 0;
  mCurrentIndicator = 0;

  MonthGraphicsItem *iItem = dynamic_cast<MonthGraphicsItem*>( itemAt( pos ) );
  if ( iItem ) {
    mClickedItem = iItem->monthItem();

    selectItem( mClickedItem );
    if ( mouseEvent->button() == Qt::RightButton ) {
      IncidenceMonthItem *tmp = qobject_cast<IncidenceMonthItem *>( mClickedItem );
      if ( tmp ) {
        emit showIncidencePopupSignal( tmp->akonadiItem(), tmp->realStartDate() );
      }
    }

    if ( mouseEvent->button() == Qt::LeftButton ) {
      // Basic initialization for resize and move
      mActionItem = mClickedItem;
      mStartCell = getCellFromPos( pos );
      mPreviousCell = mStartCell;
      mActionInitiated = false;

      // Move or resize ?
      if ( iItem->monthItem()->isResizable() &&
            iItem->isBeginItem() && iItem->mapFromScene( pos ).x() <= 10 ) {
        mActionType = Resize;
        mResizeType = ResizeLeft;
      } else if ( iItem->monthItem()->isResizable() &&
                    iItem->isEndItem() &&
                    iItem->mapFromScene( pos ).x() >= iItem->boundingRect().width() - 10 ) {
        mActionType = Resize;
        mResizeType = ResizeRight;
      } else if ( iItem->monthItem()->isMoveable() ) {
        mActionType = Move;
      }
    }
    mouseEvent->accept();
  } else if ( ScrollIndicator *scrollItem = dynamic_cast<ScrollIndicator*>( itemAt( pos ) ) ) {
    clickOnScrollIndicator( scrollItem );
    mCurrentIndicator = scrollItem;
    repeatTimer.start( AUTO_REPEAT_DELAY, this );
  } else {
    // unselect items when clicking somewhere else
    selectItem( 0 );

    MonthCell *cell = getCellFromPos( pos );
    if ( cell ) {
      mSelectedCellDate = cell->date();
      update();
      if ( mouseEvent->button() == Qt::RightButton ) {
        emit showNewEventPopupSignal();
      }
      mouseEvent->accept();
    }
  }
}

void MonthScene::timerEvent( QTimerEvent *e )
{
  if ( e->timerId() == repeatTimer.timerId() ) {
    if ( mCurrentIndicator->isVisible() ) {
      clickOnScrollIndicator( mCurrentIndicator );
      repeatTimer.start( AUTO_REPEAT_DELAY, this );
    } else {
      mCurrentIndicator = 0;
      repeatTimer.stop();
    }
  }
}

void MonthScene::mouseReleaseEvent ( QGraphicsSceneMouseEvent *mouseEvent )
{
  QPointF pos = mouseEvent->scenePos();

  static_cast<MonthGraphicsView*>( views().first() )->setActionCursor( None );

  repeatTimer.stop();
  mCurrentIndicator = 0;

  if ( mActionItem ) {
    MonthCell *currentCell = getCellFromPos( pos );

    const bool somethingChanged = currentCell && currentCell != mStartCell;

    if ( somethingChanged ) { // We want to act if a move really happened
      if ( mActionType == Resize ) {
        mActionItem->endResize();
      } else if ( mActionType == Move ) {
        mActionItem->endMove();
      }
    }

    mActionItem = 0;
    mActionType = None;
    mStartCell = 0;

    mouseEvent->accept();
  }
}

// returns true if the point is in the monthgrid (allows to avoid selecting a cell when
// a click is outside the month grid
bool MonthScene::isInMonthGrid( int x, int y ) const
{
  return x >= 0 && y >= 0 && x <= availableWidth() && y <= availableHeight();
}

// The function converts the coordinates to the month grid coordinates to
// be able to locate the cell.
MonthCell *MonthScene::getCellFromPos( const QPointF &pos )
{
  int y = sceneYToMonthGridY( static_cast<int> ( pos.y() ) );
  int x = sceneXToMonthGridX( static_cast<int> ( pos.x() ) );
  if ( !isInMonthGrid( x, y ) ) {
    return 0;
  }
  int id = ( int )( y / rowHeight() ) * 7 + ( int )( x / columnWidth() );

  return mMonthCellMap.value( mMonthView->actualStartDateTime().date().addDays( id ) );
}

void MonthScene::selectItem( MonthItem *item )
{
  /*
    if ( mSelectedItem == item ) {
      return;
    }

    I commented the above code so it's possible to selected a selected item.
    korg-mobile needs that, otherwise clicking on a selected item wont bring the editor up.
    Another solution would be to have two signals: incidenceSelected() and incidenceClicked()
  */

  IncidenceMonthItem *tmp = qobject_cast<IncidenceMonthItem*>( item );

  if ( !tmp ) {
    mSelectedItem = 0;
    emit incidenceSelected( Akonadi::Item(), QDate() );
    return;
  }

  mSelectedItem = item;
  Q_ASSERT( CalendarSupport::hasIncidence( tmp->akonadiItem() ) );

  if ( mMonthView->selectedIncidenceDates().isEmpty() ) {
    emit incidenceSelected( tmp->akonadiItem(), QDate() );
  } else {
    emit incidenceSelected( tmp->akonadiItem(), mMonthView->selectedIncidenceDates().first() );
  }
  update();
}

void MonthScene::removeIncidence( const QString &uid )
{
  foreach ( MonthItem *manager, mManagerList ) {
    IncidenceMonthItem *imi = qobject_cast<IncidenceMonthItem*>( manager );
    if ( !imi )
      continue;

    KCalCore::Incidence::Ptr incidence = imi->incidence();
    if ( !incidence )
      continue;
    if ( incidence->uid() == uid ) {
      foreach ( MonthGraphicsItem *gitem, imi->monthGraphicsItems() ) {
        removeItem( gitem );
      }
    }
  }
}

//----------------------------------------------------------
MonthGraphicsView::MonthGraphicsView( MonthView *parent )
  : QGraphicsView( parent ), mMonthView( parent )
{
  setMouseTracking( true );
}

void MonthGraphicsView::setActionCursor( MonthScene::ActionType actionType )
{
   switch ( actionType ) {
   case MonthScene::Move:
#ifndef QT_NO_CURSOR
     setCursor( Qt::ArrowCursor );
#endif
     break;
   case MonthScene::Resize:
#ifndef QT_NO_CURSOR
     setCursor( Qt::SizeHorCursor );
#endif
     break;
#ifndef QT_NO_CURSOR
   default:
     setCursor( Qt::ArrowCursor );
#endif
   }
}

void MonthGraphicsView::setScene( MonthScene *scene )
{
  mScene = scene;
  QGraphicsView::setScene( scene );
}

void MonthGraphicsView::resizeEvent( QResizeEvent *event )
{
  mScene->setSceneRect( 0, 0, event->size().width(), event->size().height() );
  mScene->updateGeometry();
}

#include "monthscene.moc"

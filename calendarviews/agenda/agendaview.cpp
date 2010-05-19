/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Kevin Krammer, krake@kdab.com

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

#include "agendaview.h"
#include "agenda.h"
#include "agendaitem.h"
#include "alternatelabel.h"
#include "prefs.h"
#include "timelabelszone.h"

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/kcal/utils.h>
#include <akonadi/kcal/incidencechanger.h>

#include <KCal/CalFilter>

#include <QStyle>
#include <KCalendarSystem>
#include <KGlobalSettings>
#include <KGlobal>
#include <KHBox>
#include <KVBox>
#include <KIconLoader>
#include <KComponentData>
#include <KConfigGroup>
#include <KMessageBox>
#include <KWordWrap>

#include <QApplication>
#include <QScrollBar>
#include <QScrollArea>
#include <QDrag>
#include <QGridLayout>
#include <QPainter>
#include <QSplitter>
#include <QTimer>
#include <QDialog>

using namespace Akonadi;
using namespace EventViews;

class EventIndicator::Private
{
  EventIndicator *const q;

  public:
    Private( EventIndicator *parent, EventIndicator::Location loc )
      : q( parent ), mColumns( 1 ), mLocation( loc )
    {
      mEnabled.resize( mColumns );

      if ( mLocation == Top ) {
        mPixmap = SmallIcon( "arrow-up-double" );
      } else {
        mPixmap = SmallIcon( "arrow-down-double" );
      }
    }

  public:
    int mColumns;
    Location mLocation;
    QPixmap mPixmap;
    QVector<bool> mEnabled;
};

EventIndicator::EventIndicator( Location loc, QWidget *parent )
  : QFrame( parent ), d( new Private( this, loc ) )
{
  setMinimumHeight( d->mPixmap.height() );
}

EventIndicator::~EventIndicator()
{
  delete d;
}

void EventIndicator::paintEvent( QPaintEvent *event )
{
  QFrame::paintEvent( event );

  QPainter painter( this );

  for ( int i = 0; i < d->mColumns; ++i ) {
    if ( d->mEnabled[ i ] ) {
      int cellWidth = contentsRect().right() / d->mColumns;
      int xOffset = QApplication::isRightToLeft() ?
                    ( d->mColumns - 1 - i ) * cellWidth + cellWidth / 2 - d->mPixmap.width() / 2 :
                    i * cellWidth + cellWidth / 2 - d->mPixmap.width() / 2;
      painter.drawPixmap( QPoint( xOffset, 0 ), d->mPixmap );
    }
  }
}

void EventIndicator::changeColumns( int columns )
{
  d->mColumns = columns;
  d->mEnabled.resize( d->mColumns );

  update();
}

void EventIndicator::enableColumn( int column, bool enable )
{
  d->mEnabled[ column ] = enable;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class AgendaView::Private : public Akonadi::Calendar::CalendarObserver
{
  AgendaView *const q;

  public:
    explicit Private( AgendaView *parent, bool isSideBySide )
      : q( parent ),
        mTopDayLabels( 0 ),
        mLayoutTopDayLabels( 0 ),
        mTopDayLabelsFrame( 0 ),
        mLayoutBottomDayLabels( 0 ),
        mBottomDayLabels( 0 ),
        mBottomDayLabelsFrame( 0 ),
        mTimeLabelsZone( 0 ),
        mAllowAgendaUpdate( true ),
        mUpdateItem( 0 ),
        mCollectionId( -1 ),
        mIsSideBySide( isSideBySide ),
        mPendingChanges( true )
    {
    }

  public:
    // view widgets
    QGridLayout *mGridLayout;
    QFrame *mTopDayLabels;
    QBoxLayout *mLayoutTopDayLabels;
    KHBox *mTopDayLabelsFrame;
    QBoxLayout *mLayoutBottomDayLabels;
    QFrame *mBottomDayLabels;
    KHBox *mBottomDayLabelsFrame;
    KHBox *mAllDayFrame;
    QWidget *mTimeBarHeaderFrame;
    QGridLayout *mAgendaLayout;
    QSplitter *mSplitterAgenda;
    QList<QLabel *> mTimeBarHeaders;

    Agenda *mAllDayAgenda;
    Agenda *mAgenda;

    TimeLabelsZone *mTimeLabelsZone;

    DateList mSelectedDates;  // List of dates to be displayed
    DateList mSaveSelectedDates; // Save the list of dates between updateViews
    int mViewType;
    EventIndicator *mEventIndicatorTop;
    EventIndicator *mEventIndicatorBottom;

    QVector<int> mMinY;
    QVector<int> mMaxY;

    QVector<bool> mHolidayMask;

    QDateTime mTimeSpanBegin;
    QDateTime mTimeSpanEnd;
    bool mTimeSpanInAllDay;
    bool mAllowAgendaUpdate;

    Akonadi::Item mUpdateItem;

    //CollectionSelection *mCollectionSelection;
    Akonadi::Collection::Id mCollectionId;

    bool mIsSideBySide;
    bool mPendingChanges;

  protected:
    /* reimplemented from KCal::Calendar::CalendarObserver */
    void calendarIncidenceAdded( const Akonadi::Item &incidence );
    void calendarIncidenceChanged( const Akonadi::Item &incidence );
    void calendarIncidenceRemoved( const Akonadi::Item &incidence );
};

void AgendaView::Private::calendarIncidenceAdded( const Item &incidence )
{
  Q_UNUSED( incidence );
  if ( !mPendingChanges ) {
    mPendingChanges = true;
    QMetaObject::invokeMethod( q, "updateView", Qt::QueuedConnection );
  }
}

void AgendaView::Private::calendarIncidenceChanged( const Item &incidence )
{
  Q_UNUSED( incidence );
  if ( !mPendingChanges ) {
    mPendingChanges = true;
    QMetaObject::invokeMethod( q, "updateView", Qt::QueuedConnection );
  }
}

void AgendaView::Private::calendarIncidenceRemoved( const Item &incidence )
{
  Q_UNUSED( incidence );
  if ( !mPendingChanges ) {
    mPendingChanges = true;
    QMetaObject::invokeMethod( q, "updateView", Qt::QueuedConnection );
  }
}

////////////////////////////////////////////////////////////////////////////

AgendaView::AgendaView( QWidget *parent, bool isSideBySide )
  : EventView( parent ), d( new Private( this, isSideBySide ) )
{
  d->mSelectedDates.append( QDate::currentDate() );

  d->mGridLayout = new QGridLayout( this );
  d->mGridLayout->setMargin( 0 );

  /* Create agenda splitter */
  d->mSplitterAgenda = new QSplitter( Qt::Vertical, this );
  d->mGridLayout->addWidget( d->mSplitterAgenda, 1, 0 );
  d->mSplitterAgenda->setOpaqueResize( KGlobalSettings::opaqueResize() );

  /* Create day name labels for agenda columns */
  d->mTopDayLabelsFrame = new KHBox( d->mSplitterAgenda );
  d->mTopDayLabelsFrame->setSpacing( 2 );

  /* Create all-day agenda widget */
  d->mAllDayFrame = new KHBox( d->mSplitterAgenda );
  d->mAllDayFrame->setSpacing( 2 );

  // Alignment and description widgets
  d->mTimeBarHeaderFrame = new KHBox( d->mAllDayFrame );

  // The widget itself
  QWidget *dummyAllDayLeft = new QWidget( d->mAllDayFrame );
  AgendaScrollArea *allDayScrollArea = new AgendaScrollArea( true, this, d->mAllDayFrame );
  d->mAllDayAgenda = allDayScrollArea->agenda();

  /* Create the main agenda widget and the related widgets */
  QWidget *agendaFrame = new QWidget( d->mSplitterAgenda );
  d->mAgendaLayout = new QGridLayout( agendaFrame );
  d->mAgendaLayout->setMargin( 0 );
  d->mAgendaLayout->setHorizontalSpacing( 2 );
  d->mAgendaLayout->setVerticalSpacing( 0 );
  if ( isSideBySide ) {
    d->mTimeBarHeaderFrame->hide();
  }

  // Create event indicator bars
  d->mEventIndicatorTop = new EventIndicator( EventIndicator::Top, agendaFrame );
  d->mAgendaLayout->addWidget( d->mEventIndicatorTop, 0, 1 );
  d->mEventIndicatorBottom = new EventIndicator( EventIndicator::Bottom, agendaFrame );
  d->mAgendaLayout->addWidget( d->mEventIndicatorBottom, 2, 1 );

  // Alignment and description widgets
  QWidget *dummyAgendaRight = new QWidget( agendaFrame );
  d->mAgendaLayout->addWidget( dummyAgendaRight, 0, 2 );

  // Create agenda
  AgendaScrollArea *scrollArea = new AgendaScrollArea( false, this, agendaFrame );
  d->mAgenda = scrollArea->agenda();

  d->mAgendaLayout->addWidget( scrollArea, 1, 1, 1, 2 );
  d->mAgendaLayout->setColumnStretch( 1, 1 );
  QWidget *dummyAllDayRight = new QWidget( d->mAllDayFrame );

  // Create time labels
  d->mTimeLabelsZone = new TimeLabelsZone( this, this, d->mAgenda );
  d->mAgendaLayout->addWidget( d->mTimeLabelsZone, 1, 0 );

  // Scrolling
  connect( d->mAgenda, SIGNAL(zoomView(const int,QPoint,const Qt::Orientation)),
           SLOT(zoomView(const int,QPoint,const Qt::Orientation)) );

  // Event indicator updates
  connect( d->mAgenda, SIGNAL(lowerYChanged(int)),
           SLOT(updateEventIndicatorTop(int)) );
  connect( d->mAgenda, SIGNAL(upperYChanged(int)),
           SLOT(updateEventIndicatorBottom(int)) );

  if ( isSideBySide ) {
    d->mTimeLabelsZone->hide();
  }

  /* Create a frame at the bottom which may be used by decorations */
  d->mBottomDayLabelsFrame = new KHBox( d->mSplitterAgenda );
  d->mBottomDayLabelsFrame->setSpacing( 2 );

  if ( !isSideBySide ) {
    /* Make the all-day and normal agendas line up with each other */
    dummyAllDayRight->setFixedWidth( style()->pixelMetric( QStyle::PM_ScrollBarExtent ) -
                                     d->mAgendaLayout->horizontalSpacing() );
    dummyAgendaRight->setFixedWidth( dummyAllDayRight->width() );
  }

  updateTimeBarWidth();
  // resize dummy widget so the allday agenda lines up with the hourly agenda
  dummyAllDayLeft->setFixedWidth( d->mTimeLabelsZone->timeLabelsWidth() -
                                  d->mTimeBarHeaderFrame->width() );

  createDayLabels( true );

  /* Connect the agendas */

  connectAgenda( d->mAgenda, d->mAllDayAgenda );
  connectAgenda( d->mAllDayAgenda, d->mAgenda );

  connect( d->mAgenda,
           SIGNAL(newTimeSpanSignal(const QPoint &,const QPoint &)),
           SLOT(newTimeSpanSelected(const QPoint &,const QPoint &)) );
  connect( d->mAllDayAgenda,
           SIGNAL(newTimeSpanSignal(const QPoint &,const QPoint &)),
           SLOT(newTimeSpanSelectedAllDay(const QPoint &,const QPoint &)) );
}

AgendaView::~AgendaView()
{
  if ( calendar() ) {
    calendar()->unregisterObserver( d );
  }

  delete d;
}

void AgendaView::setCalendar( Akonadi::Calendar *cal )
{
  if ( calendar() ) {
    calendar()->unregisterObserver( d );
  }
  Q_ASSERT( cal );
  EventView::setCalendar( cal );
  calendar()->registerObserver( d );
  d->mAgenda->setCalendar( calendar() );
  d->mAllDayAgenda->setCalendar( calendar() );
}

void AgendaView::connectAgenda( Agenda *agenda, Agenda *otherAgenda )
{
  connect( agenda, SIGNAL(showNewEventPopupSignal()),
           SIGNAL(showNewEventPopupSignal()) );

  connect( agenda, SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)),
           SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)));

  agenda->setCalendar( calendar() );

  // Create/Show/Edit/Delete Event
  // Is the newEventSignal even emitted? It doesn't seem to reach handleNewEventRequest()
  // at least.
  connect( agenda, SIGNAL(newEventSignal()), SLOT(handleNewEventRequest()) );

  connect( agenda, SIGNAL(newStartSelectSignal()),
           otherAgenda, SLOT(clearSelection()) );
  connect( agenda, SIGNAL(newStartSelectSignal()),
           SIGNAL(timeSpanSelectionChanged()) );

  connect( agenda, SIGNAL(editIncidenceSignal(Akonadi::Item)),
                   SIGNAL(editIncidenceSignal(Akonadi::Item)) );
  connect( agenda, SIGNAL(showIncidenceSignal(Akonadi::Item)),
                   SIGNAL(showIncidenceSignal(Akonadi::Item)) );
  connect( agenda, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
                   SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );

  connect( agenda, SIGNAL(startMultiModify(const QString &)),
                   SIGNAL(startMultiModify(const QString &)) );
  connect( agenda, SIGNAL(endMultiModify()),
                   SIGNAL(endMultiModify()) );

  connect( agenda, SIGNAL(itemModified(AgendaItem *)),
                   SLOT(updateEventDates(AgendaItem *)) );
  connect( agenda, SIGNAL(enableAgendaUpdate(bool)),
                   SLOT(enableAgendaUpdate(bool)) );

  // drag signals
  connect( agenda, SIGNAL(startDragSignal(Akonadi::Item)),
           SLOT(startDrag(Akonadi::Item)) );

  // synchronize selections
  connect( agenda, SIGNAL(incidenceSelected(const Akonadi::Item &, const QDate &)),
           otherAgenda, SLOT(deselectItem()) );
  connect( agenda, SIGNAL(incidenceSelected(const Akonadi::Item &, const QDate &)),
           SIGNAL(incidenceSelected(const Akonadi::Item &, const QDate &)) );

  // rescheduling of todos by d'n'd
  connect( agenda, SIGNAL(droppedToDos(QList<KCal::Todo::Ptr>,const QPoint &,bool)),
           SLOT(slotTodosDropped(QList<KCal::Todo::Ptr>,const QPoint &,bool)) );
  connect( agenda, SIGNAL(droppedToDos(QList<KUrl>,const QPoint &,bool)),
           SLOT(slotTodosDropped(QList<KUrl>,const QPoint &,bool)) );

}

void AgendaView::zoomInVertically( )
{
  if ( !d->mIsSideBySide ) {
    preferences()->setHourSize( preferences()->hourSize() + 1 );
  }
  d->mAgenda->updateConfig();
  d->mAgenda->checkScrollBoundaries();

  d->mTimeLabelsZone->updateAll();

  updateView();

}

void AgendaView::zoomOutVertically( )
{

  if ( preferences()->hourSize() > 4 || d->mIsSideBySide ) {
    if ( !d->mIsSideBySide ) {
      preferences()->setHourSize( preferences()->hourSize() - 1 );;
    }
    d->mAgenda->updateConfig();
    d->mAgenda->checkScrollBoundaries();

    d->mTimeLabelsZone->updateAll();
    updateView();
  }
}

void AgendaView::zoomInHorizontally( const QDate &date )
{
  QDate begin;
  QDate newBegin;
  QDate dateToZoom = date;
  int ndays, count;

  begin = d->mSelectedDates.first();
  ndays = begin.daysTo( d->mSelectedDates.last() );

  // zoom with Action and are there a selected Incidence?, Yes, I zoom in to it.
  if ( ! dateToZoom.isValid () ) {
    dateToZoom = d->mAgenda->selectedIncidenceDate();
  }

  if ( !dateToZoom.isValid() ) {
    if ( ndays > 1 ) {
      newBegin = begin.addDays(1);
      count = ndays - 1;
      emit zoomViewHorizontally ( newBegin, count );
    }
  } else {
    if ( ndays <= 2 ) {
      newBegin = dateToZoom;
      count = 1;
    } else {
      newBegin = dateToZoom.addDays( -ndays / 2 + 1 );
      count = ndays -1 ;
    }
    emit zoomViewHorizontally ( newBegin, count );
  }
}

void AgendaView::zoomOutHorizontally( const QDate &date )
{
  QDate begin;
  QDate newBegin;
  QDate dateToZoom = date;
  int ndays, count;

  begin = d->mSelectedDates.first();
  ndays = begin.daysTo( d->mSelectedDates.last() );

  // zoom with Action and are there a selected Incidence?, Yes, I zoom out to it.
  if ( ! dateToZoom.isValid () ) {
    dateToZoom = d->mAgenda->selectedIncidenceDate();
  }

  if ( !dateToZoom.isValid() ) {
    newBegin = begin.addDays( -1 );
    count = ndays + 3 ;
  } else {
    newBegin = dateToZoom.addDays( -ndays / 2 - 1 );
    count = ndays + 3;
  }

  if ( abs( count ) >= 31 ) {
    kDebug() << "change to the month view?";
  } else {
    //We want to center the date
    emit zoomViewHorizontally( newBegin, count );
  }
}

void AgendaView::zoomView( const int delta, const QPoint &pos, const Qt::Orientation orient )
{
  // TODO find out why this is necessary. seems to be some kind of performance hack
  static QDate zoomDate;
  static QTimer *t = new QTimer( this );

  //Zoom to the selected incidence, on the other way
  // zoom to the date on screen after the first mousewheel move.
  if ( orient == Qt::Horizontal ) {
    const QDate date = d->mAgenda->selectedIncidenceDate();
    if ( date.isValid() ) {
      zoomDate=date;
    } else {
      if ( !t->isActive() ) {
        zoomDate= d->mSelectedDates[ pos.x() ];
      }
      t->setSingleShot( true );
      t->start ( 1000 );
    }
    if ( delta > 0 ) {
      zoomOutHorizontally( zoomDate );
    } else {
      zoomInHorizontally( zoomDate );
    }
  } else {
    // Vertical zoom
    const QPoint posConstentsOld = d->mAgenda->gridToContents( pos );
    if ( delta > 0 ) {
      zoomOutVertically();
    } else {
      zoomInVertically();
    }
    const QPoint posConstentsNew = d->mAgenda->gridToContents( pos );
    d->mAgenda->verticalScrollBar()->scroll( 0, posConstentsNew.y() - posConstentsOld.y() );
  }
}

void AgendaView::placeDecorationsFrame( KHBox *frame, bool decorationsFound, bool isTop )
{
  if ( decorationsFound ) {

    if ( isTop ) {
      // inserts in the first position
      d->mSplitterAgenda->insertWidget( 0, frame );
    } else {
      // inserts in the last position
      frame->setParent( d->mSplitterAgenda );
    }
  } else {
    frame->setParent( this );
    d->mGridLayout->addWidget( frame, 0, 0 );
  }
}

void AgendaView::createDayLabels( bool force )
{
  // Check if mSelectedDates has changed, if not just return
  // Removes some flickering and gains speed (since this is called by each updateView())
  if ( !force && d->mSaveSelectedDates == d->mSelectedDates ) {
    return;
  }
  d->mSaveSelectedDates = d->mSelectedDates;

  delete d->mTopDayLabels;
  delete d->mBottomDayLabels;

  QFontMetrics fm = fontMetrics();

  d->mTopDayLabels = new QFrame ( d->mTopDayLabelsFrame );
  d->mTopDayLabelsFrame->setStretchFactor( d->mTopDayLabels, 1 );
  d->mLayoutTopDayLabels = new QHBoxLayout( d->mTopDayLabels );
  d->mLayoutTopDayLabels->setMargin( 0 );
  // this spacer moves the day labels over to line up with the day columns
  QSpacerItem *spacer =
    new QSpacerItem( d->mTimeLabelsZone->timeLabelsWidth(), 1, QSizePolicy::Fixed );
  d->mLayoutTopDayLabels->addSpacerItem( spacer );
  KVBox *topWeekLabelBox = new KVBox( d->mTopDayLabels );
  d->mLayoutTopDayLabels->addWidget( topWeekLabelBox );
  if ( d->mIsSideBySide ) {
    topWeekLabelBox->hide();
  }

  d->mBottomDayLabels = new QFrame( d->mBottomDayLabelsFrame );
  d->mBottomDayLabelsFrame->setStretchFactor( d->mBottomDayLabels, 1 );
  d->mLayoutBottomDayLabels = new QHBoxLayout( d->mBottomDayLabels );
  d->mLayoutBottomDayLabels->setMargin( 0 );
  KVBox *bottomWeekLabelBox = new KVBox( d->mBottomDayLabels );
  d->mLayoutBottomDayLabels->addWidget( bottomWeekLabelBox );

  const KCalendarSystem *calsys = KGlobal::locale()->calendar();

  Q_FOREACH( const QDate &date, d->mSelectedDates ) {
    KVBox *topDayLabelBox = new KVBox( d->mTopDayLabels );
    d->mLayoutTopDayLabels->addWidget( topDayLabelBox );
    KVBox *bottomDayLabelBox = new KVBox( d->mBottomDayLabels );
    d->mLayoutBottomDayLabels->addWidget( bottomDayLabelBox );

    int dW = calsys->dayOfWeek( date );
    QString veryLongStr = KGlobal::locale()->formatDate( date );
    QString longstr = i18nc( "short_weekday date (e.g. Mon 13)","%1 %2",
                             calsys->weekDayName( dW, KCalendarSystem::ShortDayName ),
                             calsys->day( date ) );
    QString shortstr = QString::number( calsys->day( date ) );

    AlternateLabel *dayLabel =
      new AlternateLabel( shortstr, longstr, veryLongStr, topDayLabelBox );
    dayLabel->setMinimumWidth( 1 );
    dayLabel->setAlignment( Qt::AlignHCenter );
    if ( date == QDate::currentDate() ) {
      QFont font = dayLabel->font();
      font.setBold( true );
      dayLabel->setFont( font );
    }

    // if a holiday region is selected, show the holiday name
    const QStringList texts = holidayNames( date );
    Q_FOREACH( const QString &text, texts ) {
      // Compute a small version of the holiday string for AlternateLabel
      const KWordWrap *ww = KWordWrap::formatText( fm, topDayLabelBox->rect(), 0, text, -1 );
      AlternateLabel *label =
        new AlternateLabel( ww->truncatedString(), text, text, topDayLabelBox );
      label->setMinimumWidth( 1 );
      label->setAlignment( Qt::AlignCenter );
      delete ww;
    }
  }

  if ( !d->mIsSideBySide ) {
    d->mLayoutTopDayLabels->addSpacing( d->mAgenda->verticalScrollBar()->width() );
    d->mLayoutBottomDayLabels->addSpacing( d->mAgenda->verticalScrollBar()->width() );
  }
  d->mTopDayLabels->show();
  d->mBottomDayLabels->show();
}

void AgendaView::enableAgendaUpdate( bool enable )
{
  d->mAllowAgendaUpdate = enable;
}

int AgendaView::currentDateCount() const
{
  return d->mSelectedDates.count();
}

Akonadi::Item::List AgendaView::selectedIncidences() const
{
  Akonadi::Item::List selected;

  Akonadi::Item agendaitem = d->mAgenda->selectedIncidence();
  if ( agendaitem.isValid() ) {
    selected.append( agendaitem );
  }

  Akonadi::Item dayitem = d->mAllDayAgenda->selectedIncidence();
  if ( dayitem.isValid() ) {
    selected.append( dayitem );
  }

  return selected;
}

DateList AgendaView::selectedIncidenceDates() const
{
  DateList selected;
  QDate qd;

  qd = d->mAgenda->selectedIncidenceDate();
  if ( qd.isValid() ) {
    selected.append( qd );
  }

  qd = d->mAllDayAgenda->selectedIncidenceDate();
  if ( qd.isValid() ) {
    selected.append( qd );
  }

  return selected;
}

bool AgendaView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay ) const
{
  if ( selectionStart().isValid() ) {
    QDateTime start = selectionStart();
    QDateTime end = selectionEnd();

    if ( start.secsTo( end ) == 15 * 60 ) {
      // One cell in the agenda view selected, e.g.
      // because of a double-click, => Use the default duration
      QTime defaultDuration( preferences()->defaultDuration().time() );
      int addSecs = ( defaultDuration.hour() * 3600 ) + ( defaultDuration.minute() * 60 );
      end = start.addSecs( addSecs );
    }

    startDt = start;
    endDt = end;
    allDay = selectedIsAllDay();
    return true;
  }
  return false;
}

/** returns if only a single cell is selected, or a range of cells */
bool AgendaView::selectedIsSingleCell() const
{
  if ( !selectionStart().isValid() || !selectionEnd().isValid() ) {
    return false;
  }

  if ( selectedIsAllDay() ) {
    int days = selectionStart().daysTo( selectionEnd() );
    return ( days < 1 );
  } else {
    int secs = selectionStart().secsTo( selectionEnd() );
    return ( secs <= 24 * 60 * 60 / d->mAgenda->rows() );
  }
}

void AgendaView::updateView()
{
  fillAgenda();
}

/*
  Update configuration settings for the agenda view. This method is not
  complete.
*/
void AgendaView::updateConfig()
{
  d->mAgenda->updateConfig();

  d->mAllDayAgenda->updateConfig();

  d->mTimeLabelsZone->updateAll();

  updateTimeBarWidth();

  setHolidayMasks();

  createDayLabels( true );

  updateView();
}

void AgendaView::createTimeBarHeaders()
{
  qDeleteAll( d->mTimeBarHeaders );
  d->mTimeBarHeaders.clear();

  foreach ( QScrollArea *area, d->mTimeLabelsZone->timeLabels() ) {
    TimeLabels *timeLabel = static_cast<TimeLabels*>( area->widget() );
    QLabel *label = new QLabel( timeLabel->header().replace( '/', "/ " ),
                                d->mTimeBarHeaderFrame );
    label->setAlignment( Qt::AlignBottom | Qt::AlignLeft );
    label->setMargin( 2 );
    label->setWordWrap( true );
    label->setToolTip( timeLabel->headerToolTip() );
    d->mTimeBarHeaders.append( label );
  }
}

void AgendaView::updateTimeBarWidth()
{
  createTimeBarHeaders();

  QFontMetrics fm( font() );

  int num = 0;
  int width = d->mTimeLabelsZone->timeLabelsWidth();
  foreach ( QLabel *l, d->mTimeBarHeaders ) {
    num++;
    foreach ( const QString &word, l->text().split( ' ' ) ) {
      width = qMax( width, fm.width( word ) );
    }
  }

  width = width + fm.width( "/" );
  int timeBarWidth = ( width + 1 ) * num;

  d->mTimeBarHeaderFrame->setFixedWidth( timeBarWidth );
  d->mTimeLabelsZone->setTimeLabelsWidth( width );
}

void AgendaView::updateEventDates( AgendaItem *item )
{
  kDebug() << item->text();

  KDateTime startDt, endDt;

  // Start date of this incidence, calculate the offset from it
  // (so recurring and non-recurring items can be treated exactly the same,
  // we never need to check for recurs(), because we only move the start day
  // by the number of days the agenda item was really moved. Smart, isn't it?)
  QDate thisDate;
  if ( item->cellXLeft() < 0 ) {
    thisDate = ( d->mSelectedDates.first() ).addDays( item->cellXLeft() );
  } else {
    thisDate = d->mSelectedDates[ item->cellXLeft() ];
  }
  QDate oldThisDate( item->itemDate() );
  int daysOffset = 0;

  // daysOffset should only be calculated if item->cellXLeft() is positive which doesn't happen
  // if the event's start isn't visible.
  if ( item->cellXLeft() >= 0 ) {
    daysOffset = oldThisDate.daysTo( thisDate );
  }

  int daysLength = 0;
//  startDt.setDate( startDate );

  const Item aitem = item->incidence();
  Incidence::Ptr incidence = Akonadi::incidence( aitem );
  if ( !incidence ) {
    return;
  }
  if ( !changer() ) {
    return;
  }
  Incidence::Ptr oldIncidence( incidence->clone() );

  QTime startTime( 0, 0, 0 ), endTime( 0, 0, 0 );
  if ( incidence->allDay() ) {
    daysLength = item->cellWidth() - 1;
  } else {
    startTime = d->mAgenda->gyToTime( item->cellYTop() );
    if ( item->lastMultiItem() ) {
      endTime = d->mAgenda->gyToTime( item->lastMultiItem()->cellYBottom() + 1 );
      daysLength = item->lastMultiItem()->cellXLeft() - item->cellXLeft();
    } else {
      endTime = d->mAgenda->gyToTime( item->cellYBottom() + 1 );
    }
  }

  // FIXME: use a visitor here
  if ( const Event::Ptr ev = Akonadi::event( aitem ) ) {
    startDt = incidence->dtStart();
    // convert to calendar timespec because we then manipulate it
    // with time coming from the calendar
    startDt = startDt.toTimeSpec( preferences()->timeSpec() );
    startDt = startDt.addDays( daysOffset );
    if ( !startDt.isDateOnly() ) {
      startDt.setTime( startTime );
    }
    endDt = startDt.addDays( daysLength );
    if ( !endDt.isDateOnly() ) {
      endDt.setTime( endTime );
    }
    if ( incidence->dtStart().toTimeSpec( preferences()->timeSpec() ) == startDt &&
         ev->dtEnd().toTimeSpec( preferences()->timeSpec() ) == endDt ) {
      // No change
      QTimer::singleShot( 0, this, SLOT(updateView()) );
      return;
    }
  } else if ( const Todo::Ptr td = Akonadi::todo( aitem ) ) {
    startDt = td->hasStartDate() ? td->dtStart() : td->dtDue();
    // convert to calendar timespec because we then manipulate it with time coming from
    // the calendar
    startDt = startDt.toTimeSpec( preferences()->timeSpec() );
    startDt.setDate( thisDate.addDays( td->dtDue().daysTo( startDt ) ) );
    if ( !startDt.isDateOnly() ) {
      startDt.setTime( startTime );
    }

    endDt = startDt;
    endDt.setDate( thisDate );
    if ( !endDt.isDateOnly() ) {
      endDt.setTime( endTime );
    }

    if ( td->dtDue().toTimeSpec( preferences()->timeSpec() )  == endDt ) {
      // No change
      QMetaObject::invokeMethod( this, "updateView", Qt::QueuedConnection );
      return;
    }
  }
  // FIXME: Adjusting the recurrence should really go to CalendarView so this
  // functionality will also be available in other views!
  // TODO_Recurrence: This does not belong here, and I'm not really sure
  // how it's supposed to work anyway.
/*
  Recurrence *recur = incidence->recurrence();
  if ( recur->recurs() && daysOffset != 0 ) {
    switch ( recur->recurrenceType() ) {
    case Recurrence::rYearlyPos:
    {
      int freq = recur->frequency();
      int duration = recur->duration();
      QDate endDt( recur->endDate() );
      bool negative = false;

      QPtrList<Recurrence::rMonthPos> monthPos( recur->yearMonthPositions() );
      if ( monthPos.first() ) {
        negative = monthPos.first()->negative;
      }
      QBitArray days( 7 );
      int pos = 0;
      days.fill( false );
      days.setBit( thisDate.dayOfWeek() - 1 );
      if ( negative ) {
        pos =  - ( thisDate.daysInMonth() - thisDate.day() - 1 ) / 7 - 1;
      } else {
        pos =  ( thisDate.day()-1 ) / 7 + 1;
      }
      // Terrible hack: to change the month days,
      // I have to unset the recurrence, and set all days manually again
      recur->unsetRecurs();
      if ( duration != 0 ) {
        recur->setYearly( Recurrence::rYearlyPos, freq, duration );
      } else {
        recur->setYearly( Recurrence::rYearlyPos, freq, endDt );
      }
      recur->addYearlyMonthPos( pos, days );
      recur->addYearlyNum( thisDate.month() );

      break;
    }
    case Recurrence::rYearlyDay:
    {
      int freq = recur->frequency();
      int duration = recur->duration();
      QDate endDt( recur->endDate() );
      // Terrible hack: to change the month days,
      // I have to unset the recurrence, and set all days manually again
      recur->unsetRecurs();
      if ( duration == 0 ) { // end by date
        recur->setYearly( Recurrence::rYearlyDay, freq, endDt );
      } else {
        recur->setYearly( Recurrence::rYearlyDay, freq, duration );
      }
      recur->addYearlyNum( thisDate.dayOfYear() );
      break;
    }
    case Recurrence::rYearlyMonth:
    {
      int freq = recur->frequency();
      int duration = recur->duration();
      QDate endDt( recur->endDate() );
      // Terrible hack: to change the month days,
      // I have to unset the recurrence, and set all days manually again
      recur->unsetRecurs();
      if ( duration != 0 ) {
        recur->setYearlyByDate( thisDate.day(), recur->feb29YearlyType(), freq, duration );
      } else {
        recur->setYearlyByDate( thisDate.day(), recur->feb29YearlyType(), freq, endDt );
      }
      recur->addYearlyNum( thisDate.month() );
      break; }
    case Recurrence::rMonthlyPos:
    {
      int freq = recur->frequency();
      int duration = recur->duration();
      QDate endDt( recur->endDate() );
      QPtrList<Recurrence::rMonthPos> monthPos( recur->monthPositions() );
      if ( !monthPos.isEmpty() ) {
        // FIXME: How shall I adapt the day x of week Y if we move the
        // date across month borders??? for now, just use the date of
        // the moved item and assume the recurrence only occurs on that day.
        // That's fine for korganizer, but might mess up other organizers.
        QBitArray rDays( 7 );
        rDays = monthPos.first()->rDays;
        bool negative = monthPos.first()->negative;
        int newPos;
        rDays.fill( false );
        rDays.setBit( thisDate.dayOfWeek() - 1 );
        if ( negative ) {
          newPos =  - ( thisDate.daysInMonth() - thisDate.day() - 1 ) / 7 - 1;
        } else {
          newPos =  ( thisDate.day()-1 ) / 7 + 1;
        }

        // Terrible hack: to change the month days,
        // I have to unset the recurrence, and set all days manually again
        recur->unsetRecurs();
        if ( duration == 0 ) { // end by date
          recur->setMonthly( Recurrence::rMonthlyPos, freq, endDt );
        } else {
          recur->setMonthly( Recurrence::rMonthlyPos, freq, duration );
        }
        recur->addMonthlyPos( newPos, rDays );
      }
      break;
    }
    case Recurrence::rMonthlyDay:
    {
      int freq = recur->frequency();
      int duration = recur->duration();
      QDate endDt( recur->endDate() );
      QPtrList<int> monthDays( recur->monthDays() );
      // Terrible hack: to change the month days,
      // I have to unset the recurrence, and set all days manually again
      recur->unsetRecurs();
      if ( duration == 0 ) { // end by date
        recur->setMonthly( Recurrence::rMonthlyDay, freq, endDt );
      } else {
        recur->setMonthly( Recurrence::rMonthlyDay, freq, duration );
      }
      // FIXME: How shall I adapt the n-th day if we move the date across
      // month borders??? for now, just use the date of the moved item and
      // assume the recurrence only occurs on that day.
      // That's fine for korganizer, but might mess up other organizers.
      recur->addMonthlyDay( thisDate.day() );

      break;
    }
    case Recurrence::rWeekly:
    {
      QBitArray days(7), oldDays( recur->days() );
      int offset = daysOffset % 7;
      if ( offset < 0 ) {
        offset = ( offset + 7 ) % 7;
      }
      // rotate the days
      for ( int d=0; d<7; d++ ) {
        days.setBit( ( d + offset ) % 7, oldDays.at( d ) );
      }
      if ( recur->duration() == 0 ) { // end by date
        recur->setWeekly( recur->frequency(), days, recur->endDate(), recur->weekStart() );
      } else { // duration or no end
        recur->setWeekly( recur->frequency(), days, recur->duration(), recur->weekStart() );
      }
      break;
    }
      // nothing to be done for the following:
    case Recurrence::rDaily:
    case Recurrence::rHourly:
    case Recurrence::rMinutely:
    case Recurrence::rNone:
    default:
      break;
    }
    if ( recur->duration() == 0 ) { // end by date
      recur->setEndDate( recur->endDate().addDays( daysOffset ) );
    }
    KMessageBox::information( this,
                              i18n( "A recurring calendar item was moved to a "
                                    "different day. The recurrence settings "
                                    "have been updated with that move. Please "
                                    "check them in the editor." ),
                              i18n( "Recurrence Moved" ),
                              "RecurrenceMoveInAgendaWarning" );
  }
*/

  // FIXME: use a visitor here
  if ( const Event::Ptr ev = Akonadi::event( aitem ) ) {
    /* setDtEnd() must be called before setDtStart(), otherwise, when moving
     * events, CalendarLocal::incidenceUpdated() will not remove the old hash
     * and that causes the event to be shown in the old date also (bug #179157).
     *
     * TODO: We need a better hashing mechanism for CalendarLocal.
     */
    ev->setDtEnd(
      endDt.toTimeSpec( incidence->dtEnd().timeSpec() ) );
    incidence->setDtStart( startDt.toTimeSpec( incidence->dtStart().timeSpec() ) );
  } else if ( const Todo::Ptr td = Akonadi::todo( aitem ) ) {
    if ( td->hasStartDate() ) {
      td->setDtStart( startDt.toTimeSpec( incidence->dtStart().timeSpec() ) );
    }
    td->setDtDue( endDt.toTimeSpec( td->dtDue().timeSpec() ) );
  }
  item->setItemDate( startDt.toTimeSpec( preferences()->timeSpec() ).date() );

  const bool result = changer()->changeIncidence( oldIncidence, aitem,
                                                 IncidenceChanger::DATE_MODIFIED, this );

  // Update the view correctly if an agenda item move was aborted by
  // cancelling one of the subsequent dialogs.
  if ( !result ) {
    d->mPendingChanges = true;
    QMetaObject::invokeMethod( this, "updateView", Qt::QueuedConnection );
    return;
  }

  // don't update the agenda as the item already has the correct coordinates.
  // an update would delete the current item and recreate it, but we are still
  // using a pointer to that item! => CRASH
  enableAgendaUpdate( false );
  // We need to do this in a timer to make sure we are not deleting the item
  // we are currently working on, which would lead to crashes
  // Only the actually moved agenda item is already at the correct position and mustn't be
  // recreated. All others have to!!!
  if ( incidence->recurs() ) {
    d->mUpdateItem = aitem;
    QMetaObject::invokeMethod( this, "updateView", Qt::QueuedConnection );
  }

  enableAgendaUpdate( true );
}

void AgendaView::doUpdateItem()
{
  if ( Akonadi::hasIncidence( d->mUpdateItem ) ) {
    changeIncidenceDisplay( d->mUpdateItem, IncidenceChanger::INCIDENCEEDITED );
    d->mUpdateItem = Item();
  }
}

QDate AgendaView::startDate() const
{
  if ( d->mSelectedDates.isEmpty() )
    return QDate();
  return d->mSelectedDates.first();
}

QDate AgendaView::endDate() const
{
  if ( d->mSelectedDates.isEmpty() )
    return QDate();
  return d->mSelectedDates.last();
}

void AgendaView::showDates( const QDate &start, const QDate &end )
{
  if ( !d->mSelectedDates.isEmpty() &&
       d->mSelectedDates.first() == start &&
       d->mSelectedDates.last() == end &&
       !d->mPendingChanges ) {
    return;
  }

  if ( !start.isValid() || !end.isValid() || start > end || start.daysTo( end ) > 31 ) {
    kWarning() << "got bizare parameters: " << start << end << " - aborting here";
    return;
  }

  d->mSelectedDates.clear();

  QDate date = start;
  while ( date <= end ) {
    d->mSelectedDates.append( date );
    date = date.addDays( 1 );
  }

  // and update the view
  fillAgenda();
}

void AgendaView::showIncidences( const Item::List &incidences, const QDate &date )
{
  Q_UNUSED( date );

  if ( !calendar() ) {
    kError() << "No Calendar set";
    return;
  }

  // we must check if they are not filtered; if they are, remove the filter
  CalFilter *filter = calendar()->filter();
  bool wehaveall = true;
  if ( filter ) {
    Q_FOREACH ( const Item &aitem, incidences ) {
      if ( !( wehaveall = filter->filterIncidence( Akonadi::incidence( aitem ).get() ) ) ) {
        break;
      }
    }
  }

  if ( !wehaveall ) {
    calendar()->setFilter( 0 );
  }

  const KDateTime::Spec timeSpec = preferences()->timeSpec();
  KDateTime start = Akonadi::incidence( incidences.first() )->dtStart().toTimeSpec( timeSpec );
  KDateTime end = Akonadi::incidence( incidences.first() )->dtEnd().toTimeSpec( timeSpec );
  Item first = incidences.first();
  Q_FOREACH( const Item &aitem, incidences ) {
    if ( Akonadi::incidence( aitem )->dtStart().toTimeSpec( timeSpec ) < start ) {
      first = aitem;
    }
    start = qMin( start, Akonadi::incidence( aitem )->dtStart().toTimeSpec( timeSpec ) );
    end = qMax( start, Akonadi::incidence( aitem )->dtEnd().toTimeSpec( timeSpec ) );
  }

  end.toTimeSpec( start );    // allow direct comparison of dates
  if ( start.date().daysTo( end.date() ) + 1 <= currentDateCount() ) {
    showDates( start.date(), end.date() );
  } else {
    showDates( start.date(), start.date().addDays( currentDateCount() - 1 ) );
  }

  d->mAgenda->selectItem( first );
}

void AgendaView::insertIncidence( const Item &aitem, const QDate &curDate )
{
  if ( !filterByCollectionSelection( aitem ) ) {
    return;
  }

  // FIXME: Use a visitor here, or some other method to get rid of the dynamic_cast's
  Incidence::Ptr incidence = Akonadi::incidence( aitem );
  Event::Ptr event = Akonadi::event( aitem );
  Todo::Ptr todo = Akonadi::todo( aitem );

  int curCol = d->mSelectedDates.first().daysTo( curDate );

  // In case incidence->dtStart() isn't visible (crosses bounderies)
  if ( curCol < 0 ) {
    curCol = 0;
  }

  // The date for the event is not displayed, just ignore it
  if ( curCol >= d->mSelectedDates.count() ) {
    return;
  }

  // Default values, which can never be reached
  d->mMinY[curCol] = d->mAgenda->timeToY( QTime( 23, 59 ) ) + 1;
  d->mMaxY[curCol] = d->mAgenda->timeToY( QTime( 0, 0 ) ) - 1;

  int beginX;
  int endX;
  QDate columnDate;
  if ( event ) {
    QDate firstVisibleDate = d->mSelectedDates.first();
    // its crossing bounderies, lets calculate beginX and endX
    if ( curDate < firstVisibleDate ) {
      beginX = curCol + firstVisibleDate.daysTo( curDate );
      endX   = beginX + event->dtStart().daysTo( event->dtEnd() );
      columnDate = firstVisibleDate;
    } else {
      beginX = curCol;
      endX   = beginX + event->dtStart().daysTo( event->dtEnd() );
      columnDate = curDate;
    }
  } else if ( todo ) {
    if ( !todo->hasDueDate() ) {
      return;  // todo shall not be displayed if it has no date
    }
    columnDate = curDate;
    beginX = endX = curCol;

  } else {
    return;
  }

  const KDateTime::Spec timeSpec = preferences()->timeSpec();

  if ( todo && todo->isOverdue() ) {
    d->mAllDayAgenda->insertAllDayItem( aitem, columnDate, curCol, curCol );
  } else if ( incidence->allDay() ) {
      d->mAllDayAgenda->insertAllDayItem( aitem, columnDate, beginX, endX );
  } else if ( event && event->isMultiDay( timeSpec ) ) {
    int startY = d->mAgenda->timeToY( event->dtStart().toTimeSpec( timeSpec ).time() );
    QTime endtime( event->dtEnd().toTimeSpec( timeSpec ).time() );
    if ( endtime == QTime( 0, 0, 0 ) ) {
      endtime = QTime( 23, 59, 59 );
    }
    int endY = d->mAgenda->timeToY( endtime ) - 1;
    if ( ( beginX <= 0 && curCol == 0 ) || beginX == curCol ) {
      d->mAgenda->insertMultiItem( aitem, columnDate, beginX, endX, startY, endY );

    }
    if ( beginX == curCol ) {
      d->mMaxY[curCol] = d->mAgenda->timeToY( QTime( 23, 59 ) );
      if ( startY < d->mMinY[curCol] ) {
        d->mMinY[curCol] = startY;
      }
    } else if ( endX == curCol ) {
      d->mMinY[curCol] = d->mAgenda->timeToY( QTime( 0, 0 ) );
      if ( endY > d->mMaxY[curCol] ) {
        d->mMaxY[curCol] = endY;
      }
    } else {
      d->mMinY[curCol] = d->mAgenda->timeToY( QTime( 0, 0 ) );
      d->mMaxY[curCol] = d->mAgenda->timeToY( QTime( 23, 59 ) );
    }
  } else {
    int startY = 0, endY = 0;
    if ( event ) {
      startY = d->mAgenda->timeToY( incidence->dtStart().toTimeSpec( timeSpec ).time() );
      QTime endtime( event->dtEnd().toTimeSpec( timeSpec ).time() );
      if ( endtime == QTime( 0, 0, 0 ) ) {
        endtime = QTime( 23, 59, 59 );
      }
      endY = d->mAgenda->timeToY( endtime ) - 1;
    }
    if ( todo ) {
      QTime t = todo->dtDue().toTimeSpec( timeSpec ).time();

      if ( t == QTime( 0, 0 ) ) {
        t = QTime( 23, 59 );
      }

      int halfHour = 1800;
      if ( t.addSecs( -halfHour ) < t ) {
        startY = d->mAgenda->timeToY( t.addSecs( -halfHour ) );
        endY   = d->mAgenda->timeToY( t ) - 1;
      } else {
        startY = 0;
        endY   = d->mAgenda->timeToY( t.addSecs( halfHour ) ) - 1;
      }
    }
    if ( endY < startY ) {
      endY = startY;
    }
    d->mAgenda->insertItem( aitem, columnDate, curCol, startY, endY );
    if ( startY < d->mMinY[curCol] ) {
      d->mMinY[curCol] = startY;
    }
    if ( endY > d->mMaxY[curCol] ) {
      d->mMaxY[curCol] = endY;
    }
  }
}

void AgendaView::changeIncidenceDisplayAdded( const Item &aitem )
{
  if ( !calendar() ) {
    kError() << "No Calendar set";
    return;
  }
  Todo::Ptr todo = Akonadi::todo( aitem );
  CalFilter *filter = calendar()->filter();
  if ( ( filter && !filter->filterIncidence( Akonadi::incidence( aitem ).get() ) ) ||
       ( ( todo && !preferences()->showTodosAgendaView() ) ) ) {
    return;
  }

  displayIncidence( aitem );
}

void AgendaView::changeIncidenceDisplay( const Item &aitem, int mode )
{
  switch ( mode ) {
    case IncidenceChanger::INCIDENCEADDED:
    {
      // Add an event. No need to recreate the whole view!
      // recreating everything even causes troubles: dropping to the
      // day matrix recreates the agenda items, but the evaluation is
      // still in an agendaItems' code, which was deleted in the mean time.
      // Thus KOrg crashes...
      changeIncidenceDisplayAdded( aitem );
      updateEventIndicators();
      break;
    }
    case IncidenceChanger::INCIDENCEEDITED:
    {
      if ( d->mAllowAgendaUpdate ) {
        //PENDING(AKONADI_PORT) try harder not to recreate the items here, this causes flicker with the delayed notification from Akonadi, after a dnd operation
        removeIncidence( aitem );
        changeIncidenceDisplayAdded( aitem );
      }
      updateEventIndicators();
      break;
    }
    case IncidenceChanger::INCIDENCEDELETED:
    {
      removeIncidence( aitem );
      updateEventIndicators();
      break;
    }
    default:
      return;
  }

  // HACK: Update the view if the all-day agenda has been modified.
  // Do this because there are some layout problems in the
  // all-day agenda that are not easily solved, but clearing
  // and redrawing works ok.
  Incidence::Ptr incidence = Akonadi::incidence( aitem );
  if ( incidence && incidence->allDay() ) {
    updateView();
  }
}

void AgendaView::fillAgenda( const QDate & )
{
  fillAgenda();
}

void AgendaView::fillAgenda()
{
  d->mPendingChanges = false;

  /* Remember the item Ids of the selected items. In case one of the
   * items was deleted and re-added, we want to reselect it. */
  const Item::Id selectedAgendaId = d->mAgenda->lastSelectedItemId();
  const Item::Id selectedAllDayAgendaId = d->mAllDayAgenda->lastSelectedItemId();

  enableAgendaUpdate( true );
  clearView();

  d->mAllDayAgenda->changeColumns( d->mSelectedDates.count() );
  d->mAgenda->changeColumns( d->mSelectedDates.count() );
  d->mEventIndicatorTop->changeColumns( d->mSelectedDates.count() );
  d->mEventIndicatorBottom->changeColumns( d->mSelectedDates.count() );

  createDayLabels( false );
  setHolidayMasks();

  d->mMinY.resize( d->mSelectedDates.count() );
  d->mMaxY.resize( d->mSelectedDates.count() );

  d->mAgenda->setDateList( d->mSelectedDates );

  bool somethingReselected = false;
  const Item::List incidences = calendar() ? calendar()->incidences() : Item::List();

  foreach ( const Item &aitem, incidences ) {
    displayIncidence( aitem );
    if ( aitem.id() == selectedAgendaId ) {
      d->mAgenda->selectItem( aitem );
      somethingReselected = true;
    }

    if ( aitem.id() == selectedAllDayAgendaId ) {
      d->mAllDayAgenda->selectItem( aitem );
      somethingReselected = true;
    }
  }

  d->mAgenda->checkScrollBoundaries();
  updateEventIndicators();

  //  mAgenda->viewport()->update();
  //  mAllDayAgenda->viewport()->update();

  // make invalid
  deleteSelectedDateTime();

  if ( !somethingReselected ) {
    emit incidenceSelected( Item(), QDate() );
  }
}

void AgendaView::displayIncidence( const Item &aitem )
{
  QDate today = QDate::currentDate();
  DateTimeList::iterator t;

  // FIXME: use a visitor here
  Incidence::Ptr incidence = Akonadi::incidence( aitem );
  Todo::Ptr todo = Akonadi::todo( aitem );
  Event::Ptr event = Akonadi::event( aitem );

  const KDateTime::Spec timeSpec = preferences()->timeSpec();

  KDateTime firstVisibleDateTime( d->mSelectedDates.first(), timeSpec );
  KDateTime lastVisibleDateTime( d->mSelectedDates.last(), timeSpec );

  lastVisibleDateTime.setTime( QTime( 23, 59, 59, 59 ) );
  firstVisibleDateTime.setTime( QTime( 0, 0 ) );
  DateTimeList dateTimeList;

  const KDateTime incDtStart = incidence->dtStart().toTimeSpec( timeSpec );
  const KDateTime incDtEnd   = incidence->dtEnd().toTimeSpec( timeSpec );

  if ( todo && ( !preferences()->showTodosAgendaView() || !todo->hasDueDate() ) ) {
    return;
  }

  if ( incidence->recurs() ) {
    int eventDuration = event ? incDtStart.daysTo( incDtEnd ) : 0;

    // if there's a multiday event that starts before firstVisibleDateTime but ends after
    // lets include it. timesInInterval() ignores incidences that aren't totaly inside
    // the range
    const KDateTime startDateTimeWithOffset = firstVisibleDateTime.addDays( -eventDuration );
    dateTimeList = incidence->recurrence()->timesInInterval( startDateTimeWithOffset,
                                                             lastVisibleDateTime );
  } else {
    KDateTime dateToAdd; // date to add to our date list
    KDateTime incidenceStart;
    KDateTime incidenceEnd;

    if ( todo && todo->hasDueDate() && !todo->isOverdue() ) {
      // If it's not overdue it will be shown at the original date (not today)
      dateToAdd = todo->dtDue().toTimeSpec( timeSpec );

      // To-dos are drawn with the bottom of the rectangle at dtDue
      // if dtDue is at 00:00, then it should be displayed in the previous day, at 23:59
      if ( dateToAdd.time() == QTime( 0, 0 ) ) {
        dateToAdd = dateToAdd.addSecs( -1 );
      }

      incidenceEnd = dateToAdd;
    } else if ( event ) {
      dateToAdd = incDtStart;
      incidenceEnd = incDtEnd;
    }

    if ( dateToAdd.isValid() && dateToAdd.isDateOnly() ) {
      // so comparisons with < > actually work
      dateToAdd.setTime( QTime( 0, 0 ) );
      incidenceEnd.setTime( QTime( 23, 59, 59, 59 ) );
    }

    if  ( dateToAdd <= lastVisibleDateTime && incidenceEnd > firstVisibleDateTime ) {
      dateTimeList += dateToAdd;
    }
  }

  // ToDo items shall be displayed today if they are already overdude
  const KDateTime dateTimeToday = KDateTime( today, timeSpec );
  if ( todo &&
       todo->isOverdue() &&
       dateTimeToday >= firstVisibleDateTime &&
       dateTimeToday <= lastVisibleDateTime ) {

    bool doAdd = true;

    if ( todo->recurs() ) {
      /* If there's a recurring instance showing up today don't add "today" again
       * we don't want the event to appear duplicated */
      for ( t = dateTimeList.begin(); t != dateTimeList.end(); ++t ) {
        if ( t->toTimeSpec( timeSpec ).date() == today ) {
          doAdd = false;
          break;
        }
      }
    }

    if ( doAdd ) {
      dateTimeList += dateTimeToday;
    }
  }

  for ( t = dateTimeList.begin(); t != dateTimeList.end(); ++t ) {
    insertIncidence( aitem, t->toTimeSpec( timeSpec ).date() );
  }
}

void AgendaView::clearView()
{
  d->mAllDayAgenda->clear();
  d->mAgenda->clear();
}

void AgendaView::updateEventIndicatorTop( int newY )
{
  for ( int i = 0; i < d->mMinY.size(); ++i ) {
    d->mEventIndicatorTop->enableColumn( i, newY > d->mMinY[i] );
  }
  d->mEventIndicatorTop->update();
}

void AgendaView::updateEventIndicatorBottom( int newY )
{
  for ( int i = 0; i < d->mMaxY.size(); ++i ) {
    d->mEventIndicatorBottom->enableColumn( i, newY <= d->mMaxY[i] );
  }
  d->mEventIndicatorBottom->update();
}

void AgendaView::slotTodosDropped( const QList<KUrl> &items, const QPoint &gpos, bool allDay )
{

#ifdef AKONADI_PORT_DISABLED // one item -> multiple items, Incidence* -> akonadi item url (we might have to fetch the items here first!)
  if ( gpos.x() < 0 || gpos.y() < 0 ) {
    return;
  }

  const QDate day = d->mSelectedDates[gpos.x()];
  const QTime time = d->mAgenda->gyToTime( gpos.y() );
  KDateTime newTime( day, time, preferences()->timeSpec() );
  newTime.setDateOnly( allDay );

  Todo::Ptr todo = Akonadi::todo( todoItem );
  if ( todo &&  dynamic_cast<Akonadi::Calendar*>( calendar() ) ) {
    const Item existingTodoItem = calendar()->itemForIncidence( calendar()->todo( todo->uid() ) );
    if ( Todo::Ptr existingTodo = Akonadi::todo( existingTodoItem ) ) {
      kDebug() << "Drop existing Todo";
      Todo::Ptr oldTodo( existingTodo->clone() );
      if ( changer() ) {
        existingTodo->setDtDue( newTime );
        existingTodo->setAllDay( allDay );
        existingTodo->setHasDueDate( true );
        changer()->changeIncidence( oldTodo, existingTodoItem, IncidenceChanger::DATE_MODIFIED, this );
      } else {
        KMessageBox::sorry( this, i18n( "Unable to modify this to-do, "
                                        "because it cannot be locked." ) );
      }
    } else {
      kDebug() << "Drop new Todo";
      todo->setDtDue( newTime );
      todo->setAllDay( allDay );
      todo->setHasDueDate( true );
      if ( !changer()->addIncidence( todo, this ) ) {
        KMessageBox::sorry( this,
                            i18n( "Unable to save %1 \"%2\".",
                            i18n( todo->type() ), todo->summary() ) );
      }
    }
  }
#else
  kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
}

void AgendaView::slotTodosDropped( const QList<Todo::Ptr> &items, const QPoint &gpos, bool allDay )
{
  if ( gpos.x() < 0 || gpos.y() < 0 ) {
    return;
  }

  const QDate day = d->mSelectedDates[gpos.x()];
  const QTime time = d->mAgenda->gyToTime( gpos.y() );
  KDateTime newTime( day, time, preferences()->timeSpec() );
  newTime.setDateOnly( allDay );

  Q_FOREACH( const Todo::Ptr &todo, items ) {
    todo->setDtDue( newTime );
    todo->setAllDay( allDay );
    todo->setHasDueDate( true );

    Akonadi::Collection selectedCollection;
    int dialogCode;
    if ( !changer()->addIncidence( todo, this, selectedCollection, dialogCode ) ) {
      if ( dialogCode != QDialog::Rejected ) {
        KMessageBox::sorry( this,
                            i18n( "Unable to save %1 \"%2\".",
                            i18n( todo->type() ), todo->summary() ) );
      }
    }
  }
}
void AgendaView::startDrag( const Item &incidence )
{
  if ( !calendar() ) {
    kError() << "No Calendar set";
    return;
  }
  if ( QDrag *drag = Akonadi::createDrag( incidence, calendar()->timeSpec(), this ) ) {
    drag->exec();
  }
}

void AgendaView::readSettings()
{
  readSettings( KGlobal::activeComponent().config().data() );
}

void AgendaView::readSettings( const KConfig *config )
{
  const KConfigGroup group = config->group( "Views" );

  const QList<int> sizes = group.readEntry( "Separator AgendaView", QList<int>() );

  // the size depends on the number of plugins used
  // we don't want to read invalid/corrupted settings or else agenda becomes invisible
  if ( sizes.count() >= 2 && !sizes.contains( 0 ) ) {
    d->mSplitterAgenda->setSizes( sizes );
  }

  updateConfig();
}

void AgendaView::writeSettings( KConfig *config )
{
  KConfigGroup group = config->group( "Views" );

  QList<int> list = d->mSplitterAgenda->sizes();
  group.writeEntry( "Separator AgendaView", list );
}

void AgendaView::setHolidayMasks()
{
  if ( d->mSelectedDates.isEmpty() || !d->mSelectedDates[0].isValid() ) {
    return;
  }

  d->mHolidayMask.resize( d->mSelectedDates.count() + 1 );

  for ( int i = 0; i < d->mSelectedDates.count(); ++i ) {
    d->mHolidayMask[i] = !isWorkDay( d->mSelectedDates[ i ] );
  }

  // Store the information about the day before the visible area (needed for
  // overnight working hours) in the last bit of the mask:
  bool showDay = !isWorkDay( d->mSelectedDates[ 0 ].addDays( -1 ) );
  d->mHolidayMask[ d->mSelectedDates.count() ] = showDay;

  d->mAgenda->setHolidayMask( &d->mHolidayMask );
  d->mAllDayAgenda->setHolidayMask( &d->mHolidayMask );
}

void AgendaView::setContentsPos( int y )
{
  if ( y != d->mAgenda->contentsY() ) {
    d->mAgenda->setContentsPos( 0, y );
  }
}

void AgendaView::clearSelection()
{
  d->mAgenda->deselectItem();
  d->mAllDayAgenda->deselectItem();
}

void AgendaView::newTimeSpanSelectedAllDay( const QPoint &start, const QPoint &end )
{
  newTimeSpanSelected( start, end );
  d->mTimeSpanInAllDay = true;
}

void AgendaView::handleNewEventRequest()
{
  emit newEventSignal( Akonadi::Collection::List() << Collection( collection() ) );
}

void AgendaView::newTimeSpanSelected( const QPoint &start, const QPoint &end )
{
  if ( !d->mSelectedDates.count() ) {
    return;
  }

  d->mTimeSpanInAllDay = false;

  const QDate dayStart =d-> mSelectedDates[ qBound( 0, start.x(), (int)d->mSelectedDates.size() - 1 ) ];
  const QDate dayEnd = d->mSelectedDates[ qBound( 0, end.x(), (int)d->mSelectedDates.size() - 1 ) ];

  const QTime timeStart = d->mAgenda->gyToTime( start.y() );
  const QTime timeEnd = d->mAgenda->gyToTime( end.y() + 1 );

  d->mTimeSpanBegin = QDateTime( dayStart, timeStart );;
  d->mTimeSpanEnd = QDateTime( dayEnd, timeEnd );
}

QDateTime AgendaView::selectionStart() const
{
  return d->mTimeSpanBegin;
}

QDateTime AgendaView::selectionEnd() const
{
  return d->mTimeSpanEnd;
}

bool AgendaView::selectedIsAllDay() const
{
  return d->mTimeSpanInAllDay;
}

void AgendaView::deleteSelectedDateTime()
{
  d->mTimeSpanBegin.setDate( QDate() );
  d->mTimeSpanEnd.setDate( QDate() );
  d->mTimeSpanInAllDay = false;
}

void AgendaView::removeIncidence( const Item &incidence )
{
  d->mAgenda->removeIncidence( incidence );
  d->mAllDayAgenda->removeIncidence( incidence );
}

void AgendaView::updateEventIndicators()
{
  d->mMinY = d->mAgenda->minContentsY();
  d->mMaxY = d->mAgenda->maxContentsY();

  d->mAgenda->checkScrollBoundaries();
  updateEventIndicatorTop( d->mAgenda->visibleContentsYMin() );
  updateEventIndicatorBottom( d->mAgenda->visibleContentsYMax() );
}

void AgendaView::setIncidenceChanger( IncidenceChanger *changer )
{
  EventView::setIncidenceChanger( changer );
  d->mAgenda->setIncidenceChanger( changer );
  d->mAllDayAgenda->setIncidenceChanger( changer );
}

void AgendaView::clearTimeSpanSelection()
{
  d->mAgenda->clearSelection();
  d->mAllDayAgenda->clearSelection();
  deleteSelectedDateTime();
}

#if 0
void AgendaView::setCollectionSelection( CollectionSelection *sel )
{
  if ( mCollectionSelection == sel ) {
    return;
  }
  mCollectionSelection = sel;
}
#endif

void AgendaView::setCollection( Collection::Id coll )
{
  if ( d->mCollectionId != coll ) {
    d->mCollectionId = coll;
  }
}

Akonadi::Collection::Id AgendaView::collection() const
{
  return d->mCollectionId;
}

Agenda *AgendaView::agenda() const
{
  return d->mAgenda;
}

QSplitter *AgendaView::splitter() const
{
  return d->mSplitterAgenda;
}

bool AgendaView::filterByCollectionSelection( const Item &incidence )
{
  if ( customCollectionSelection() ) {
    return customCollectionSelection()->contains( incidence.parentCollection().id() );
  }

  if ( d->mCollectionId < 0 ) {
    return true;
  } else {
    return d->mCollectionId == incidence.storageCollectionId();
  }
}

void AgendaView::setUpdateNeeded()
{
  d->mPendingChanges = true;
}

#include "agendaview.moc"
// kate: space-indent on; indent-width 2; replace-tabs on;

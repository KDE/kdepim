/*
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Kevin Krammer, krake@kdab.com
  Author: Sergio Martins, sergio.martins@kdab.com

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
#include "calendardecoration.h"
#include "decorationlabel.h"
#include "prefs.h"
#include "timelabels.h"
#include "timelabelszone.h"

#include <Akonadi/Calendar/ETMCalendar>
#include <calendarsupport/collectionselection.h>
#include <Akonadi/Calendar/IncidenceChanger>
#include <calendarsupport/utils.h>
#include <calendarsupport/kcalprefs.h>

#include <KCalCore/CalFilter>
#include <KCalCore/CalFormat>
#include <KCalCore/OccurrenceIterator>

#include <KCalendarSystem>
#include <KIconLoader> // for SmallIcon()
#include <KGlobalSettings>
#include <KHBox>
#include <KMessageBox>
#include <KServiceTypeTrader>
#include <KVBox>
#include <KWordWrap>
#include <QDebug>
#include <KGlobal>
#include <KComponentData>

#include <QApplication>
#include <QDrag>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QScrollBar>
#include <QSplitter>
#include <QStyle>
#include <QTimer>

using namespace EventViews;

enum {
  SPACING = 2
};
enum {
  SHRINKDOWN = 2  // points less for the timezone font
};

class EventIndicator::Private
{
  EventIndicator *const q;

  public:
    Private( EventIndicator *parent, EventIndicator::Location loc )
      : q( parent ), mColumns( 1 ), mLocation( loc )
    {
      mEnabled.resize( mColumns );

      QChar ch;
      // Dashed up and down arrow characters
      ch = QChar( mLocation == Top ? 0x21e1 : 0x21e3 );
      QFont font = q->font();
      font.setPixelSize( KIconLoader::global()->currentSize( KIconLoader::Dialog ) );
      QFontMetrics fm( font );
      QRect rect = fm.boundingRect( ch ).adjusted( -2, -2, 2, 2 );
      mPixmap = QPixmap( rect.size() );
      mPixmap.fill( Qt::transparent );
      QPainter p( &mPixmap );
      p.setOpacity( 0.33 );
      p.setFont( font );
      p.setPen( q->palette().text().color() );
      p.drawText( -rect.left(), -rect.top(), ch );
    }

    void adjustGeometry()
    {
      QRect rect;
      rect.setWidth( q->parentWidget()->width() );
      rect.setHeight( q->height() );
      rect.setLeft( 0 );
      rect.setTop( mLocation == EventIndicator::Top ?
                     0 :
                     q->parentWidget()->height() - rect.height() );
      q->setGeometry( rect );
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
  setAttribute( Qt::WA_TransparentForMouseEvents );
  setFixedHeight( d->mPixmap.height() );
  parent->installEventFilter( this );
}

EventIndicator::~EventIndicator()
{
  delete d;
}

void EventIndicator::paintEvent( QPaintEvent * )
{
  QPainter painter( this );

  const double cellWidth = static_cast<double>( width() ) / d->mColumns;
  const bool isRightToLeft = QApplication::isRightToLeft();
  const uint pixmapOffset = isRightToLeft ? 0 : ( cellWidth - d->mPixmap.width() );
  for ( int i = 0; i < d->mColumns; ++i ) {
    if ( d->mEnabled[ i ] ) {
      const int xOffset = ( isRightToLeft ? ( d->mColumns - 1 - i ) : i ) * cellWidth;
      painter.drawPixmap( xOffset + pixmapOffset, 0, d->mPixmap );
    }
  }
}

bool EventIndicator::eventFilter( QObject *, QEvent * event )
{
  if ( event->type() == QEvent::Resize ) {
    d->adjustGeometry();
  }
  return false;
}

void EventIndicator::changeColumns( int columns )
{
  d->mColumns = columns;
  d->mEnabled.resize( d->mColumns );

  show();
  raise();
  update();
}

void EventIndicator::enableColumn( int column, bool enable )
{
  Q_ASSERT( column < d->mEnabled.count() );
  d->mEnabled[ column ] = enable;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class AgendaView::Private : public Akonadi::ETMCalendar::CalendarObserver
{
  AgendaView *const q;

  public:
    explicit Private( AgendaView *parent, bool isInteractive, bool isSideBySide )
      : q( parent ),
        mTopDayLabels( 0 ),
        mLayoutTopDayLabels( 0 ),
        mTopDayLabelsFrame( 0 ),
        mLayoutBottomDayLabels( 0 ),
        mBottomDayLabels( 0 ),
        mBottomDayLabelsFrame( 0 ),
        mTimeBarHeaderFrame( 0 ),
        mAllDayAgenda( 0 ),
        mAgenda( 0 ),
        mTimeLabelsZone( 0 ),
        mAllowAgendaUpdate( true ),
        mUpdateItem( 0 ),
        mIsSideBySide( isSideBySide ),
        mDummyAllDayLeft( 0 ),
        mUpdateAllDayAgenda( true ),
        mUpdateAgenda( true ),
        mIsInteractive( isInteractive ),
        mUpdateEventIndicatorsScheduled( false )
    {
    }

  public:
    // view widgets
    QGridLayout *mGridLayout;
    QFrame *mTopDayLabels;
    QBoxLayout *mLayoutTopDayLabels;
    KHBox *mTopDayLabelsFrame;
    QList<AlternateLabel*> mDateDayLabels;
    QBoxLayout *mLayoutBottomDayLabels;
    QFrame *mBottomDayLabels;
    KHBox *mBottomDayLabelsFrame;
    KHBox *mAllDayFrame;
    QWidget *mTimeBarHeaderFrame;
    QSplitter *mSplitterAgenda;
    QList<QLabel *> mTimeBarHeaders;

    Agenda *mAllDayAgenda;
    Agenda *mAgenda;

    TimeLabelsZone *mTimeLabelsZone;

    KCalCore::DateList mSelectedDates;  // List of dates to be displayed
    KCalCore::DateList mSaveSelectedDates; // Save the list of dates between updateViews
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

    const bool mIsSideBySide;

    QWidget *mDummyAllDayLeft;
    bool mUpdateAllDayAgenda;
    bool mUpdateAgenda;
    bool mIsInteractive;
    bool mUpdateEventIndicatorsScheduled;

    // Contains days that have at least one all-day Event with TRANSP: OPAQUE ( busy )
    // that has you as organizer or attendee so we can color background with a different
    // color
    QMap<QDate, KCalCore::Event::List > mBusyDays;

    bool makesWholeDayBusy( const KCalCore::Incidence::Ptr &incidence ) const;
    CalendarDecoration::Decoration *loadCalendarDecoration( const QString &name );
    void clearView();
    void setChanges( EventView::Changes changes,
                     const KCalCore::Incidence::Ptr &incidence =
                                    KCalCore::Incidence::Ptr() );

    /**
        Returns a list of consecutive dates, starting with @p start and ending
        with @p end. If either start or end are invalid, a list with
        QDate::currentDate() is returned */
    static QList<QDate> generateDateList( const QDate &start, const QDate &end );

    void changeColumns( int numColumns );

    AgendaItem::List agendaItems( Akonadi::Entity::Id id ) const;

    // insertAtDateTime is in the view's timezone
    void insertIncidence( const Akonadi::Item &,
                          const KDateTime &insertAtDateTime, bool createSelected );
    void reevaluateIncidence( const KCalCore::Incidence::Ptr &incidence );

    bool datesEqual( const KCalCore::Incidence::Ptr &one, const KCalCore::Incidence::Ptr &two ) const;

    /**
     * Returns false if the incidence is for sure outside of the visible timespan.
     * Returns true if it might be, meaning that to be sure, timezones must be
     * taken into account.
     * This is a very fast way of discarding incidences that are outside of the
     * timespan and only performing expensive timezone operations on the ones
     * that might be viisble
     */
    bool mightBeVisible( const KCalCore::Incidence::Ptr &incidence ) const;

  protected:
    /* reimplemented from KCalCore::Calendar::CalendarObserver */
    void calendarIncidenceAdded( const KCalCore::Incidence::Ptr &incidence );
    void calendarIncidenceChanged( const KCalCore::Incidence::Ptr &incidence );
    void calendarIncidenceDeleted( const KCalCore::Incidence::Ptr &incidence );
};

bool AgendaView::Private::datesEqual( const KCalCore::Incidence::Ptr &one, const KCalCore::Incidence::Ptr &two ) const
{
  const KDateTime start1 = one->dtStart();
  const KDateTime start2 = two->dtStart();
  const KDateTime end1   = one->dateTime( KCalCore::Incidence::RoleDisplayEnd );
  const KDateTime end2   = two->dateTime( KCalCore::Incidence::RoleDisplayEnd );

  if ( start1.isValid() ^ start2.isValid() )
    return false;

  if ( end1.isValid() ^ end2.isValid() )
    return false;

  if ( start1.isValid() && start1 != start2 )
    return false;

  if ( end1.isValid() && end1 != end2 )
    return false;

  return true;
}

AgendaItem::List AgendaView::Private::agendaItems( Akonadi::Item::Id id ) const
{
  AgendaItem::List allDayAgendaItems = mAllDayAgenda->agendaItems( id );
  return allDayAgendaItems.isEmpty() ? mAgenda->agendaItems( id ) : allDayAgendaItems;
}

bool AgendaView::Private::mightBeVisible( const KCalCore::Incidence::Ptr &incidence ) const
{
  KCalCore::Todo::Ptr todo = incidence.dynamicCast<KCalCore::Todo>();
  const KDateTime::Spec timeSpec = q->preferences()->timeSpec();
  KDateTime firstVisibleDateTime( mSelectedDates.first(), timeSpec );
  KDateTime lastVisibleDateTime( mSelectedDates.last(), timeSpec );

  // KDateTime::toTimeSpec() is expensive, so lets first compare only the date,
  // to see if the incidence is visible.
  // If it's more than 48h of diff, then for sure it won't be visible,
  // independently of timezone.
  // The largest difference between two timezones is about 24 hours.

  if ( todo && todo->isOverdue() ) {
    // Don't optimize this case. Overdue to-dos have their own rules for displaying themselves
    return true;
  }

  if ( !incidence->recurs() ) {
    // If DTEND/DTDUE is before the 1st visible column
    if ( incidence->dateTime( KCalCore::Incidence::RoleEnd ).date().daysTo( firstVisibleDateTime.date() ) > 2 ) {
      return false;
    }

    // if DTSTART is after the last visible column
    if ( !todo && lastVisibleDateTime.date().daysTo( incidence->dtStart().date() ) > 2 ) {
      return false;
    }

    // if DTDUE is after the last visible column
    if ( todo && lastVisibleDateTime.date().daysTo( todo->dtDue().date() ) > 2 ) {
      return false;
    }
  }

  return true;
}

void AgendaView::Private::changeColumns( int numColumns )
{
  // mMinY, mMaxY and mEnabled must all have the same size.
  // Make sure you preserve this order because mEventIndicatorTop->changeColumns()
  // can trigger a lot of stuff, and code will be executed when mMinY wasn't resized yet.
  mMinY.resize( numColumns );
  mMaxY.resize( numColumns );
  mEventIndicatorTop->changeColumns( numColumns );
  mEventIndicatorBottom->changeColumns( numColumns );
}

/** static */
QList<QDate> AgendaView::Private::generateDateList( const QDate &start, const QDate &end )
{
  QList<QDate> list;

  if ( start.isValid() && end.isValid() && end >= start &&
       start.daysTo( end ) < AgendaView::MAX_DAY_COUNT ) {
    QDate date = start;
    while ( date <= end ) {
      list.append( date );
      date = date.addDays( 1 );
    }
  } else {
    list.append( QDate::currentDate() );
  }

  return list;
}

void AgendaView::Private::reevaluateIncidence( const KCalCore::Incidence::Ptr &incidence )
{
  const Akonadi::Item item = q->calendar()->item( incidence );
  if ( !incidence || !item.isValid() ) {
    qWarning() << "invalid incidence or item not found." << item.isValid() << incidence;
    return;
  }

  q->removeIncidence( incidence );
  q->displayIncidence( item, false );
  mAgenda->checkScrollBoundaries();
  q->updateEventIndicators();
}

void AgendaView::Private::calendarIncidenceAdded( const KCalCore::Incidence::Ptr &incidence )
{
  Akonadi::Item item = q->calendar()->item( incidence );
  if ( !incidence || incidence->uid().isEmpty() || !item.isValid()) {
    qCritical() << "AgendaView::Private::calendarIncidenceAdded() Invalid incidence or item:" << incidence << item.isValid();
    Q_ASSERT( false );
    return;
  }

  if ( incidence->hasRecurrenceId() ) {
    // Reevaluate the main event instead, if it was inserted before this one
    KCalCore::Incidence::Ptr mainIncidence = q->calendar()->incidence( incidence->uid() );
    if ( mainIncidence ) {
      reevaluateIncidence( mainIncidence );
    }
  } else if ( q->displayIncidence( item, false ) ) {
    mAgenda->checkScrollBoundaries();
    q->scheduleUpdateEventIndicators();
  }
}

void AgendaView::Private::calendarIncidenceChanged( const KCalCore::Incidence::Ptr &incidence )
{
  if ( !incidence || incidence->uid().isEmpty() ) {
    qCritical() << "AgendaView::calendarIncidenceChanged() Invalid incidence or empty UID. " << incidence;
    Q_ASSERT( false );
    return;
  }

  Akonadi::Item item = q->calendar()->item( incidence->instanceIdentifier() );
  if ( !item.isValid() ) {
    qWarning() << "AgendaView::calendarIncidenceChanged() Invalid item for incidence " << incidence->uid();
    return;
  }

  AgendaItem::List agendaItems = this->agendaItems( item.id() );
  if ( agendaItems.isEmpty() ) {
    qWarning() << "AgendaView::calendarIncidenceChanged() Invalid agendaItem for incidence " << incidence->uid();
    return;
  }

  // Optimization: If the dates didn't change, just repaint it.
  // This optimization for now because we need to process collisions between agenda items.
  if ( false && !incidence->recurs() && agendaItems.count() == 1 ) {
    KCalCore::Incidence::Ptr originalIncidence = agendaItems.first()->incidence().payload<KCalCore::Incidence::Ptr>();

    if ( datesEqual( originalIncidence, incidence ) ) {
      foreach ( const AgendaItem::QPtr &agendaItem, agendaItems ) {
        Akonadi::Item itemClone = item;
        itemClone.setPayload<KCalCore::Incidence::Ptr>( KCalCore::Incidence::Ptr( incidence->clone() ) );
        agendaItem->setIncidence( itemClone );
        agendaItem->update();
      }
      return;
    }
  }

  if ( incidence->hasRecurrenceId() ) {
    // Reevaluate the main event instead, if it exists
    KCalCore::Incidence::Ptr mainIncidence = q->calendar()->incidence( incidence->uid() );
    reevaluateIncidence( mainIncidence ? mainIncidence : incidence );
  } else {
    reevaluateIncidence( incidence );
  }

  // No need to call setChanges(), that triggers a fillAgenda()
  // setChanges( q->changes() | IncidencesEdited, incidence );
}

void AgendaView::Private::calendarIncidenceDeleted( const KCalCore::Incidence::Ptr &incidence )
{
  if ( !incidence || incidence->uid().isEmpty() ) {
    qWarning() << "invalid incidence or empty uid: " << incidence;
    Q_ASSERT( false );
    return;
  }

  q->removeIncidence( incidence );

  if ( incidence->hasRecurrenceId()) {
    // Reevaluate the main event, if it exists. The exception was removed so the main recurrent series
    // will no be bigger.
    KCalCore::Incidence::Ptr mainIncidence = q->calendar()->incidence( incidence->uid() );
    if ( mainIncidence ) {
      reevaluateIncidence( mainIncidence  );
    }
  } else if ( mightBeVisible( incidence ) ) {
    // No need to call setChanges(), that triggers a fillAgenda()
    // setChanges( q->changes() | IncidencesDeleted, CalendarSupport::incidence( incidence ) );
    mAgenda->checkScrollBoundaries();
    q->scheduleUpdateEventIndicators();
  }
}

void EventViews::AgendaView::Private::setChanges( EventView::Changes changes,
                                                  const KCalCore::Incidence::Ptr &incidence )
{
  // We could just call EventView::setChanges(...) but we're going to do a little
  // optimization. If only an all day item was changed, only all day agenda
  // should be updated.

  // all bits = 1
  const int ones = ~0;

  const int incidenceOperations = IncidencesAdded | IncidencesEdited | IncidencesDeleted;

  // If changes has a flag turned on, other than incidence operations, then update both agendas
  if ( ( ones ^ incidenceOperations ) & changes ) {
    mUpdateAllDayAgenda = true;
    mUpdateAgenda = true;
  } else if ( incidence ) {
    mUpdateAllDayAgenda = mUpdateAllDayAgenda | incidence->allDay();
    mUpdateAgenda = mUpdateAgenda | !incidence->allDay();
  }

  q->EventView::setChanges( changes );
}

void AgendaView::Private::clearView()
{
  if ( mUpdateAllDayAgenda ) {
    mAllDayAgenda->clear();
  }

  if ( mUpdateAgenda ) {
    mAgenda->clear();
  }

  mBusyDays.clear();
}

void AgendaView::Private::insertIncidence( const Akonadi::Item &aitem,
                                           const KDateTime &insertAtDateTime,
                                           bool createSelected )
{
  if ( !q->filterByCollectionSelection( aitem ) ) {
    return;
  }

  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  KCalCore::Event::Ptr event = CalendarSupport::event( aitem );
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( aitem );

  const QDate insertAtDate = insertAtDateTime.date();

  // In case incidence->dtStart() isn't visible (crosses bounderies)
  const int curCol = qMax( mSelectedDates.first().daysTo( insertAtDate ), qint64(0) );

  // The date for the event is not displayed, just ignore it
  if ( curCol >= mSelectedDates.count() ) {
    return;
  }

  if ( mMinY.count() <= curCol ) {
    mMinY.resize( mSelectedDates.count() );
  }

  if ( mMaxY.count() <= curCol ) {
    mMaxY.resize( mSelectedDates.count() );
  }

  // Default values, which can never be reached
  mMinY[curCol] = mAgenda->timeToY( QTime( 23, 59 ) ) + 1;
  mMaxY[curCol] = mAgenda->timeToY( QTime( 0, 0 ) ) - 1;

  int beginX;
  int endX;
  if ( event ) {
    const QDate firstVisibleDate = mSelectedDates.first();
    // its crossing bounderies, lets calculate beginX and endX
    const int duration = event->dtStart().toTimeSpec( q->preferences()->timeSpec() ).daysTo( event->dtEnd() );
    if ( insertAtDate < firstVisibleDate ) {
      beginX = curCol + firstVisibleDate.daysTo( insertAtDate );
      endX   = beginX + duration;
    } else {
      beginX = curCol;
      endX   = beginX + duration;
    }
  } else if ( todo ) {
    if ( !todo->hasDueDate() ) {
      return;  // todo shall not be displayed if it has no date
    }
    beginX = endX = curCol;
  } else {
    return;
  }

  const KDateTime::Spec timeSpec = q->preferences()->timeSpec();
  const QDate today = KDateTime::currentDateTime( timeSpec ).date();
  if ( todo && todo->isOverdue() && today >= insertAtDate ) {
    mAllDayAgenda->insertAllDayItem( aitem, insertAtDateTime, curCol, curCol,
                                     createSelected );
  } else if ( incidence->allDay() ) {
    mAllDayAgenda->insertAllDayItem( aitem, insertAtDateTime, beginX, endX,
                                     createSelected );
  } else if ( event && event->isMultiDay( timeSpec ) ) {
    // TODO: We need a better isMultiDay(), one that receives the occurrence.

    // In the single-day handling code there's a neat comment on why
    // we're calculating the start time this way
    const QTime startTime = insertAtDateTime.time();

    // In the single-day handling code there's a neat comment on why we use the
    // duration instead of fetching the end time directly
    const int durationOfFirstOccurrence = event->dtStart().secsTo( event->dtEnd() );
    QTime endTime = startTime.addSecs( durationOfFirstOccurrence );

    const int startY = mAgenda->timeToY( startTime );

    if ( endTime == QTime( 0, 0, 0 ) ) {
      endTime = QTime( 23, 59, 59 );
    }
    const int endY = mAgenda->timeToY( endTime ) - 1;
    if ( ( beginX <= 0 && curCol == 0 ) || beginX == curCol ) {
      mAgenda->insertMultiItem( aitem, insertAtDateTime, beginX, endX, startY, endY,
                                createSelected );
    }
    if ( beginX == curCol ) {
      mMaxY[curCol] = mAgenda->timeToY( QTime( 23, 59 ) );
      if ( startY < mMinY[curCol] ) {
        mMinY[curCol] = startY;
      }
    } else if ( endX == curCol ) {
      mMinY[curCol] = mAgenda->timeToY( QTime( 0, 0 ) );
      if ( endY > mMaxY[curCol] ) {
        mMaxY[curCol] = endY;
      }
    } else {
      mMinY[curCol] = mAgenda->timeToY( QTime( 0, 0 ) );
      mMaxY[curCol] = mAgenda->timeToY( QTime( 23, 59 ) );
    }
  } else {
    int startY = 0, endY = 0;
    if ( event ) { // Single day events fall here
      // Don't use event->dtStart().toTimeSpec( timeSpec ).time().
      // If it's a UTC recurring event it should have a different time when it crosses DST,
      // so we must use insertAtDate here, so we get the correct time.
      //
      // The nth occurrence doesn't always have the same time as the 1st occurrence.
      const QTime startTime = insertAtDateTime.time();

      // We could just fetch the end time directly from dtEnd() instead of adding a duration to the
      // start time. This way is best because it preserves the duration of the event. There are some
      // corner cases where the duration would be messed up, for example a UTC event that when
      // converted to local has dtStart() in day light saving time, but dtEnd() outside DST.
      // It could create events with 0 duration.
      const int durationOfFirstOccurrence = event->dtStart().secsTo( event->dtEnd() );
      QTime endTime = startTime.addSecs( durationOfFirstOccurrence );

      startY = mAgenda->timeToY( startTime );
      if ( endTime == QTime( 0, 0, 0 ) ) {
        endTime = QTime( 23, 59, 59 );
      }
      endY = mAgenda->timeToY( endTime ) - 1;
    }
    if ( todo ) {
      QTime t;
      if ( todo->recurs() ) {
        // The time we get depends on the insertAtDate, because of daylight savings changes
        const KDateTime ocurrrenceDateTime = KDateTime( insertAtDate, todo->dtDue().time(),
                                                        todo->dtDue().timeSpec() );
        t = ocurrrenceDateTime.toTimeSpec( timeSpec ).time();
      } else {
        t = todo->dtDue().toTimeSpec( timeSpec ).time();
      }

      if ( t == QTime( 0, 0 ) && !todo->recurs() ) {
        // To-dos due at 00h00 are drawn at the previous day and ending at
        // 23h59. For recurring to-dos, that's not being done because it wasn't
        // implemented yet in ::fillAgenda().
        t = QTime( 23, 59 );
      }

      const int halfHour = 1800;
      if ( t.addSecs( -halfHour ) < t ) {
        startY = mAgenda->timeToY( t.addSecs( -halfHour ) );
        endY   = mAgenda->timeToY( t ) - 1;
      } else {
        startY = 0;
        endY   = mAgenda->timeToY( t.addSecs( halfHour ) ) - 1;
      }
    }
    if ( endY < startY ) {
      endY = startY;
    }
    mAgenda->insertItem( aitem, insertAtDateTime, curCol, startY, endY, 1, 1,
                         createSelected );
    if ( startY < mMinY[curCol] ) {
      mMinY[curCol] = startY;
    }
    if ( endY > mMaxY[curCol] ) {
      mMaxY[curCol] = endY;
    }
  }
}

////////////////////////////////////////////////////////////////////////////

AgendaView::AgendaView( const QDate &start,
                        const QDate &end,
                        bool isInteractive,
                        bool isSideBySide,
                        QWidget *parent )
  : EventView( parent ), d( new Private( this, isInteractive, isSideBySide ) )
{
  init( start, end );
}

AgendaView::AgendaView( const PrefsPtr &prefs,
                        const QDate &start,
                        const QDate &end,
                        bool isInteractive,
                        bool isSideBySide,
                        QWidget *parent )
  : EventView( parent ), d( new Private( this, isInteractive, isSideBySide ) )
{
  setPreferences( prefs );
  init( start, end );
}

void AgendaView::init( const QDate &start, const QDate &end )
{
  d->mSelectedDates = Private::generateDateList( start, end );

  d->mGridLayout = new QGridLayout( this );
  d->mGridLayout->setMargin( 0 );

  /* Create agenda splitter */
  d->mSplitterAgenda = new QSplitter( Qt::Vertical, this );
  d->mGridLayout->addWidget( d->mSplitterAgenda, 1, 0 );
  d->mSplitterAgenda->setOpaqueResize( KGlobalSettings::opaqueResize() );

  /* Create day name labels for agenda columns */
  d->mTopDayLabelsFrame = new KHBox( d->mSplitterAgenda );
  d->mTopDayLabelsFrame->setSpacing( SPACING );

  /* Create all-day agenda widget */
  d->mAllDayFrame = new KHBox( d->mSplitterAgenda );
  d->mAllDayFrame->setSpacing( SPACING );

  // Alignment and description widgets
  if ( !d->mIsSideBySide ) {
    d->mTimeBarHeaderFrame = new KHBox( d->mAllDayFrame );
  }

  // The widget itself
  d->mDummyAllDayLeft = new QWidget( d->mAllDayFrame );
  AgendaScrollArea *allDayScrollArea = new AgendaScrollArea( true, this,
                                                             d->mIsInteractive, d->mAllDayFrame );
  d->mAllDayAgenda = allDayScrollArea->agenda();

  /* Create the main agenda widget and the related widgets */
  QWidget *agendaFrame = new QWidget( d->mSplitterAgenda );
  QHBoxLayout *agendaLayout = new QHBoxLayout( agendaFrame );
  agendaLayout->setMargin( 0 );
  agendaLayout->setSpacing( SPACING );

  // Create agenda
  AgendaScrollArea *scrollArea = new AgendaScrollArea( false, this, d->mIsInteractive,
                                                       agendaFrame );
  d->mAgenda = scrollArea->agenda();

  // Create event indicator bars
  d->mEventIndicatorTop = new EventIndicator( EventIndicator::Top, scrollArea->viewport() );
  d->mEventIndicatorBottom = new EventIndicator( EventIndicator::Bottom, scrollArea->viewport() );

  // Create time labels
  d->mTimeLabelsZone = new TimeLabelsZone( this, preferences(), d->mAgenda );

  // This timeLabelsZoneLayout is for adding some spacing
  // to align timelabels, to agenda's grid
  QVBoxLayout *timeLabelsZoneLayout = new QVBoxLayout();

  agendaLayout->addLayout( timeLabelsZoneLayout );
  agendaLayout->addWidget( scrollArea );

  timeLabelsZoneLayout->addSpacing( scrollArea->frameWidth() );
  timeLabelsZoneLayout->addWidget( d->mTimeLabelsZone );
  timeLabelsZoneLayout->addSpacing( scrollArea->frameWidth() );

  // Scrolling
  connect( d->mAgenda, SIGNAL(zoomView(int,QPoint,Qt::Orientation)),
           SLOT(zoomView(int,QPoint,Qt::Orientation)) );

  // Event indicator updates
  connect( d->mAgenda, SIGNAL(lowerYChanged(int)),
           SLOT(updateEventIndicatorTop(int)) );
  connect( d->mAgenda, SIGNAL(upperYChanged(int)),
           SLOT(updateEventIndicatorBottom(int)) );

  if ( d->mIsSideBySide ) {
    d->mTimeLabelsZone->hide();
  }

  /* Create a frame at the bottom which may be used by decorations */
  d->mBottomDayLabelsFrame = new KHBox( d->mSplitterAgenda );
  d->mBottomDayLabelsFrame->setSpacing( SPACING );

  if ( !d->mIsSideBySide ) {
    /* Make the all-day and normal agendas line up with each other */
    int margin = style()->pixelMetric( QStyle::PM_ScrollBarExtent );
    if ( style()->styleHint( QStyle::SH_ScrollView_FrameOnlyAroundContents ) ) {
      // Needed for some styles. Oxygen needs it, Plastique does not.
      margin -= scrollArea->frameWidth();
    }
    d->mAllDayFrame->layout()->addItem( new QSpacerItem( margin, 0 ) );
  }

  updateTimeBarWidth();

  // Don't call it now, bottom agenda isn't fully up yet
  QMetaObject::invokeMethod( this, "alignAgendas", Qt::QueuedConnection );

  // Whoever changes this code, remember to leave createDayLabels()
  // inside the ctor, so it's always called before readSettings(), so
  // readSettings() works on the splitter that has the right amount of
  // widgets ( createDayLabels() via placeDecorationFrame() removes widgets).
  createDayLabels( true );

  /* Connect the agendas */

  connect( d->mAllDayAgenda,
           SIGNAL(newTimeSpanSignal(QPoint,QPoint)),
           SLOT(newTimeSpanSelectedAllDay(QPoint,QPoint)) );

  connect( d->mAgenda,
           SIGNAL(newTimeSpanSignal(QPoint,QPoint)),
           SLOT(newTimeSpanSelected(QPoint,QPoint)) );

  connectAgenda( d->mAgenda, d->mAllDayAgenda );
  connectAgenda( d->mAllDayAgenda, d->mAgenda );
}

AgendaView::~AgendaView()
{
  if ( calendar() ) {
    calendar()->unregisterObserver( d );
  }

  delete d;
}

void AgendaView::setCalendar( const Akonadi::ETMCalendar::Ptr &cal )
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

  connect( agenda, SIGNAL(newEventSignal()), SIGNAL(newEventSignal()) );

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

  // drag signals
  connect( agenda, SIGNAL(startDragSignal(Akonadi::Item)),
           SLOT(startDrag(Akonadi::Item)) );

  // synchronize selections
  connect( agenda, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           otherAgenda, SLOT(deselectItem()) );
  connect( agenda, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );

  // rescheduling of todos by d'n'd
  connect( agenda, SIGNAL(droppedIncidences(KCalCore::Incidence::List,QPoint,bool)),
           SLOT(slotIncidencesDropped(KCalCore::Incidence::List,QPoint,bool)) );
  connect( agenda, SIGNAL(droppedIncidences(QList<KUrl>,QPoint,bool)),
           SLOT(slotIncidencesDropped(QList<KUrl>,QPoint,bool)) );

}

void AgendaView::zoomInVertically( )
{
  if ( !d->mIsSideBySide ) {
    preferences()->setHourSize( preferences()->hourSize() + 1 );
  }
  d->mAgenda->updateConfig();
  d->mAgenda->checkScrollBoundaries();

  d->mTimeLabelsZone->updateAll();
  setChanges( changes() | ZoomChanged );
  updateView();

}

void AgendaView::zoomOutVertically( )
{

  if ( preferences()->hourSize() > 4 || d->mIsSideBySide ) {
    if ( !d->mIsSideBySide ) {
      preferences()->setHourSize( preferences()->hourSize() - 1 );
    }
    d->mAgenda->updateConfig();
    d->mAgenda->checkScrollBoundaries();

    d->mTimeLabelsZone->updateAll();
    setChanges( changes() | ZoomChanged );
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
    qDebug() << "change to the month view?";
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

#ifndef EVENTVIEWS_NODECOS

bool AgendaView::loadDecorations( const QStringList &decorations,
                                  DecorationList &decoList )
{
  foreach ( const QString &decoName, decorations ) {
    if ( preferences()->selectedPlugins().contains( decoName ) ) {
      decoList << d->loadCalendarDecoration( decoName );
    }
  }
  return decoList.count() > 0;
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

void AgendaView::placeDecorations( DecorationList &decoList, const QDate &date,
                                   KHBox *labelBox, bool forWeek )
{
  foreach ( CalendarDecoration::Decoration *deco, decoList ) {
    CalendarDecoration::Element::List elements;
    elements = forWeek ? deco->weekElements( date ) : deco->dayElements( date );
    if ( elements.count() > 0 ) {
      KHBox *decoHBox = new KHBox( labelBox );
      decoHBox->setFrameShape( QFrame::StyledPanel );
      decoHBox->setMinimumWidth( 1 );

      foreach ( CalendarDecoration::Element *it, elements ) {
        DecorationLabel *label = new DecorationLabel( it, decoHBox );
        label->setAlignment( Qt::AlignBottom );
        label->setMinimumWidth( 1 );
      }
    }
  }
}

#endif

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
  d->mDateDayLabels.clear();

  QFontMetrics fm = fontMetrics();

  d->mTopDayLabels = new QFrame ( d->mTopDayLabelsFrame );
  d->mTopDayLabelsFrame->setStretchFactor( d->mTopDayLabels, 1 );
  d->mLayoutTopDayLabels = new QHBoxLayout( d->mTopDayLabels );
  d->mLayoutTopDayLabels->setMargin( 0 );
  d->mLayoutTopDayLabels->setSpacing( 1 );

  // this spacer moves the day labels over to line up with the day columns
  QSpacerItem *spacer =
    new QSpacerItem( ( !d->mIsSideBySide ? d->mTimeLabelsZone->width() : 0 ) +
                     SPACING +
                     d->mAllDayAgenda->scrollArea()->frameWidth(),
                     1, QSizePolicy::Fixed );

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

#ifndef EVENTVIEWS_NODECOS
  QList<CalendarDecoration::Decoration *> topDecos;
  QStringList topStrDecos = preferences()->decorationsAtAgendaViewTop();
  placeDecorationsFrame( d->mTopDayLabelsFrame, loadDecorations( topStrDecos, topDecos ), true );

  QList<CalendarDecoration::Decoration *> botDecos;
  QStringList botStrDecos = preferences()->decorationsAtAgendaViewBottom();
  placeDecorationsFrame( d->mBottomDayLabelsFrame,
                         loadDecorations( botStrDecos, botDecos ), false );
#endif

  Q_FOREACH ( const QDate &date, d->mSelectedDates ) {
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
    dayLabel->useShortText(); // will be recalculated in updateDayLabelSizes() anyway
    dayLabel->setMinimumWidth( 1 );
    dayLabel->setAlignment( Qt::AlignHCenter );
    if ( date == QDate::currentDate() ) {
      QFont font = dayLabel->font();
      font.setBold( true );
      dayLabel->setFont( font );
    }
    d->mDateDayLabels.append( dayLabel );
    // if a holiday region is selected, show the holiday name
    const QStringList texts = CalendarSupport::holiday( date );
    Q_FOREACH ( const QString &text, texts ) {
      // Compute a small version of the holiday string for AlternateLabel
      const KWordWrap ww = KWordWrap::formatText( fm, topDayLabelBox->rect(), 0, text, -1 );
      AlternateLabel *label =
        new AlternateLabel( ww.truncatedString(), text, text, topDayLabelBox );
      label->setMinimumWidth( 1 );
      label->setAlignment( Qt::AlignCenter );
    }

#ifndef EVENTVIEWS_NODECOS
    // Day decoration labels
    placeDecorations( topDecos, date, topDayLabelBox, false );
    placeDecorations( botDecos, date, bottomDayLabelBox, false );
#endif
  }

  QSpacerItem *rightSpacer = new QSpacerItem( d->mAllDayAgenda->scrollArea()->frameWidth(),
                                              1, QSizePolicy::Fixed );
  d->mLayoutTopDayLabels->addSpacerItem( rightSpacer );

#ifndef EVENTVIEWS_NODECOS
  // Week decoration labels
  placeDecorations( topDecos, d->mSelectedDates.first(), topWeekLabelBox, true );
  placeDecorations( botDecos, d->mSelectedDates.first(), bottomWeekLabelBox, true );
#endif

  if ( !d->mIsSideBySide ) {
    d->mLayoutTopDayLabels->addSpacing( d->mAgenda->verticalScrollBar()->width() );
    d->mLayoutBottomDayLabels->addSpacing( d->mAgenda->verticalScrollBar()->width() );
  }
  d->mTopDayLabels->show();
  d->mBottomDayLabels->show();

  // Update the labels now and after a single event loop run. Now to avoid flicker, and
  // delayed so that the delayed layouting size is taken into account.
  updateDayLabelSizes();
}

void AgendaView::updateDayLabelSizes()
{
  // First, calculate the maximum text type that fits for all labels
  AlternateLabel::TextType overallType = AlternateLabel::Extensive;
  foreach ( AlternateLabel *label, d->mDateDayLabels ) {
    AlternateLabel::TextType type = label->largestFittingTextType();
    if ( type < overallType ) {
      overallType = type;
    }
  }

  // Then, set that maximum text type to all the labels
  foreach ( AlternateLabel *label, d->mDateDayLabels ) {
    label->setFixedType( overallType );
  }
}

void AgendaView::resizeEvent( QResizeEvent *resizeEvent )
{
  updateDayLabelSizes();
  EventView::resizeEvent( resizeEvent );
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

KCalCore::DateList AgendaView::selectedIncidenceDates() const
{
  KCalCore::DateList selected;
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
      QTime defaultDuration( CalendarSupport::KCalPrefs::instance()->defaultDuration().time() );
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
  // Agenda can be null if setPreferences() is called inside the ctor
  // We don't need to update anything in this case.
  if ( d->mAgenda && d->mAllDayAgenda ) {
    d->mAgenda->updateConfig();
    d->mAllDayAgenda->updateConfig();
    d->mTimeLabelsZone->setPreferences( preferences() );
    d->mTimeLabelsZone->updateAll();
    updateTimeBarWidth();
    setHolidayMasks();
    createDayLabels( true );
    setChanges( changes() | ConfigChanged );
    updateView();
  }
}

void AgendaView::createTimeBarHeaders()
{
  qDeleteAll( d->mTimeBarHeaders );
  d->mTimeBarHeaders.clear();

  const QFont oldFont( font() );
  QFont labelFont = d->mTimeLabelsZone->preferences()->agendaTimeLabelsFont();
  labelFont.setPointSize( labelFont.pointSize() - SHRINKDOWN );

  foreach ( QScrollArea *area, d->mTimeLabelsZone->timeLabels() ) {
    TimeLabels *timeLabel = static_cast<TimeLabels*>( area->widget() );
    QLabel *label = new QLabel( timeLabel->header().replace( QLatin1Char('/'), QLatin1String("/ ") ),
                                d->mTimeBarHeaderFrame );
    label->setFont( labelFont );
    label->setAlignment( Qt::AlignBottom | Qt::AlignRight );
    label->setMargin( 0 );
    label->setWordWrap( true );
    label->setToolTip( timeLabel->headerToolTip() );
    d->mTimeBarHeaders.append( label );
  }
  setFont( oldFont );
}

void AgendaView::updateTimeBarWidth()
{
  if ( d->mIsSideBySide ) {
    return;
  }

  createTimeBarHeaders();

  const QFont oldFont( font() );
  QFont labelFont = d->mTimeLabelsZone->preferences()->agendaTimeLabelsFont();
  labelFont.setPointSize( labelFont.pointSize() - SHRINKDOWN );
  QFontMetrics fm( labelFont );

  int width = d->mTimeLabelsZone->preferedTimeLabelsWidth();
  foreach ( QLabel *l, d->mTimeBarHeaders ) {
    foreach ( const QString &word, l->text().split( QLatin1Char(' ') ) ) {
      width = qMax( width, fm.width( word ) );
    }
  }
  setFont( oldFont );

  width = width + fm.width( QLatin1Char( '/' ) );

  const int timeBarWidth = width * d->mTimeBarHeaders.count();

  d->mTimeBarHeaderFrame->setFixedWidth( timeBarWidth - SPACING );
  d->mTimeLabelsZone->setFixedWidth( timeBarWidth );
  d->mDummyAllDayLeft->setFixedWidth( 0 );
}

void AgendaView::updateEventDates( AgendaItem *item, bool addIncidence,
                                   Akonadi::Collection::Id collectionId )
{
  qDebug() << item->text()
           << "; item->cellXLeft(): " << item->cellXLeft()
           << "; item->cellYTop(): " << item->cellYTop()
           << "; item->lastMultiItem(): " << item->lastMultiItem()
           << "; item->itemPos(): " << item->itemPos()
           << "; item->itemCount(): " << item->itemCount()
           << endl;

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
  int daysOffset = 0;

  // daysOffset should only be calculated if item->cellXLeft() is positive which doesn't happen
  // if the event's start isn't visible.
  if ( item->cellXLeft() >= 0 ) {
    daysOffset = item->occurrenceDate().daysTo( thisDate );
  }

  int daysLength = 0;
  //  startDt.setDate( startDate );

  const Akonadi::Item aitem = item->incidence();
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  if ( !incidence || !changer() ) {
    qWarning() << "changer is " << changer() << " and incidence is " << incidence.data();
    return;
  }

  KCalCore::Incidence::Ptr oldIncidence( incidence->clone() );

  QTime startTime( 0, 0, 0 ), endTime( 0, 0, 0 );
  if ( incidence->allDay() ) {
    daysLength = item->cellWidth() - 1;
  } else {
    startTime = d->mAgenda->gyToTime( item->cellYTop() );
    if ( item->lastMultiItem() ) {
      endTime = d->mAgenda->gyToTime( item->lastMultiItem()->cellYBottom() + 1 );
      daysLength = item->lastMultiItem()->cellXLeft() - item->cellXLeft();
    } else if ( item->itemPos() == item->itemCount() && item->itemCount() > 1 ) {
      /* multiitem handling in agenda assumes two things:
         - The start (first KOAgendaItem) is always visible.
         - The first KOAgendaItem of the incidence has a non-null item->lastMultiItem()
             pointing to the last KOagendaItem.

        But those aren't always met, for example when in day-view.
        kolab/issue4417
       */

      // Cornercase 1: - Resizing the end of the event but the start isn't visible
      endTime = d->mAgenda->gyToTime( item->cellYBottom() + 1 );
      daysLength = item->itemCount() - 1;
      startTime = incidence->dtStart().time();
    } else if ( item->itemPos() == 1 && item->itemCount() > 1 ) {
      // Cornercase 2: - Resizing the start of the event but the end isn't visible
      endTime = incidence->dateTime( KCalCore::Incidence::RoleEnd ).time();
      daysLength = item->itemCount() - 1;
    } else {
      endTime = d-> mAgenda->gyToTime( item->cellYBottom() + 1 );
    }
  }

  // FIXME: use a visitor here
  if ( const KCalCore::Event::Ptr ev = CalendarSupport::event( aitem ) ) {
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
  } else if ( const KCalCore::Todo::Ptr td = CalendarSupport::todo( aitem ) ) {
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

  // A commented code block which had 150 lines to adjust recurrence was here.
  // I deleted it in rev 1180272 to make this function readable.

  if ( const KCalCore::Event::Ptr ev = CalendarSupport::event( aitem ) ) {
    /* setDtEnd() must be called before setDtStart(), otherwise, when moving
     * events, CalendarLocal::incidenceUpdated() will not remove the old hash
     * and that causes the event to be shown in the old date also (bug #179157).
     *
     * TODO: We need a better hashing mechanism for CalendarLocal.
     */
    ev->setDtEnd(
      endDt.toTimeSpec( incidence->dateTime( KCalCore::Incidence::RoleEnd ).timeSpec() ) );
    incidence->setDtStart( startDt.toTimeSpec( incidence->dtStart().timeSpec() ) );
  } else if ( const KCalCore::Todo::Ptr td = CalendarSupport::todo( aitem ) ) {
    if ( td->hasStartDate() ) {
      td->setDtStart( startDt.toTimeSpec( incidence->dtStart().timeSpec() ) );
    }
    td->setDtDue( endDt.toTimeSpec( td->dtDue().timeSpec() ) );
  }
  item->setOccurrenceDateTime( startDt );

  bool result;
  if ( addIncidence ) {
    Akonadi::Collection collection = calendar()->collection( collectionId );
    qDebug() << "Collection isValid() = " << collection.isValid();
    result = changer()->createIncidence( incidence, collection, this ) != -1;
  } else {
    result = changer()->modifyIncidence( aitem, oldIncidence, this ) != -1;
  }

  // Update the view correctly if an agenda item move was aborted by
  // cancelling one of the subsequent dialogs.
  if ( !result ) {
    setChanges( changes() | IncidencesEdited );
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
  if ( incidence->recurs() || incidence->hasRecurrenceId() ) {
    d->mUpdateItem = aitem;
    QMetaObject::invokeMethod( this, "updateView", Qt::QueuedConnection );
  }

  enableAgendaUpdate( true );
}

QDate AgendaView::startDate() const
{
  if ( d->mSelectedDates.isEmpty() ) {
    return QDate();
  }
  return d->mSelectedDates.first();
}

QDate AgendaView::endDate() const
{
  if ( d->mSelectedDates.isEmpty() ) {
    return QDate();
  }
  return d->mSelectedDates.last();
}

void AgendaView::showDates( const QDate &start, const QDate &end, const QDate &preferredMonth )
{
  Q_UNUSED( preferredMonth );
  if ( !d->mSelectedDates.isEmpty() &&
       d->mSelectedDates.first() == start &&
       d->mSelectedDates.last() == end ) {
    return;
  }

  if ( !start.isValid() || !end.isValid() || start > end ||
       start.daysTo( end ) > MAX_DAY_COUNT ) {
    qWarning() << "got bizare parameters: " << start << end << " - aborting here";
    return;
  }

  d->mSelectedDates = d->generateDateList( start, end );

  // and update the view
  setChanges( changes() | DatesChanged );
  fillAgenda();
}

void AgendaView::showIncidences( const Akonadi::Item::List &incidences, const QDate &date )
{
  Q_UNUSED( date );

  if ( !calendar() ) {
    qCritical() << "No Calendar set";
    return;
  }

  // we must check if they are not filtered; if they are, remove the filter
  KCalCore::CalFilter *filter = calendar()->filter();
  bool wehaveall = true;
  if ( filter ) {
    Q_FOREACH ( const Akonadi::Item &aitem, incidences ) {
      if ( !( wehaveall = filter->filterIncidence( CalendarSupport::incidence( aitem ) ) ) ) {
        break;
      }
    }
  }

  if ( !wehaveall ) {
    calendar()->setFilter( 0 );
  }

  const KDateTime::Spec timeSpec = preferences()->timeSpec();
  KDateTime start =
    CalendarSupport::incidence( incidences.first() )->dtStart().toTimeSpec( timeSpec );
  KDateTime end =
    CalendarSupport::incidence( incidences.first() )->dateTime(
      KCalCore::Incidence::RoleEnd ).toTimeSpec( timeSpec );
  Akonadi::Item first = incidences.first();
  Q_FOREACH ( const Akonadi::Item &aitem, incidences ) {
    if ( CalendarSupport::incidence( aitem )->dtStart().toTimeSpec( timeSpec ) < start ) {
      first = aitem;
    }
    start = qMin( start,
                  CalendarSupport::incidence( aitem )->dtStart().toTimeSpec( timeSpec ) );
    end = qMax( start,
                CalendarSupport::incidence( aitem )->dateTime(
                  KCalCore::Incidence::RoleEnd ).toTimeSpec( timeSpec ) );
  }

  end.toTimeSpec( start );    // allow direct comparison of dates
  if ( start.date().daysTo( end.date() ) + 1 <= currentDateCount() ) {
    showDates( start.date(), end.date() );
  } else {
    showDates( start.date(), start.date().addDays( currentDateCount() - 1 ) );
  }

  d->mAgenda->selectItem( first );
}

void AgendaView::fillAgenda()
{
  if ( changes() == NothingChanged ) {
    return;
  }

  if ( !calendar() ) {
    qWarning() << "No calendar is set";
    return;
  }

  /*
  qDebug() << "changes = " << changes()
           << "; mUpdateAgenda = " << d->mUpdateAgenda
           << "; mUpdateAllDayAgenda = " << d->mUpdateAllDayAgenda; */

  /* Remember the item Ids of the selected items. In case one of the
   * items was deleted and re-added, we want to reselect it. */
  const Akonadi::Item::Id selectedAgendaId = d->mAgenda->lastSelectedItemId();
  const Akonadi::Item::Id selectedAllDayAgendaId = d->mAllDayAgenda->lastSelectedItemId();

  enableAgendaUpdate( true );
  d->clearView();

  if ( changes().testFlag( DatesChanged ) ) {
    d->mAllDayAgenda->changeColumns( d->mSelectedDates.count() );
    d->mAgenda->changeColumns( d->mSelectedDates.count() );
    d->changeColumns( d->mSelectedDates.count() );

    createDayLabels( false );
    setHolidayMasks();

    d->mAgenda->setDateList( d->mSelectedDates );
  }

  setChanges( NothingChanged );

  bool somethingReselected = false;
  const KCalCore::Incidence::List incidences = calendar()->incidences();

  foreach ( const KCalCore::Incidence::Ptr &incidence, incidences ) {
    Q_ASSERT( incidence );
    Akonadi::Item aitem = calendar()->item( incidence );
    const bool wasSelected = aitem.id() == selectedAgendaId  ||
                             aitem.id() == selectedAllDayAgendaId;

    if ( ( incidence->allDay() && d->mUpdateAllDayAgenda ) ||
          ( !incidence->allDay() && d->mUpdateAgenda ) ) {
      displayIncidence( aitem, wasSelected );
    }

    if ( wasSelected ) {
      somethingReselected = true;
    }
  }

  d->mAgenda->checkScrollBoundaries();
  updateEventIndicators();

  //  mAgenda->viewport()->update();
  //  mAllDayAgenda->viewport()->update();

  // make invalid
  deleteSelectedDateTime();

  d->mUpdateAgenda = false;
  d->mUpdateAllDayAgenda = false;

  if ( !somethingReselected ) {
    emit incidenceSelected( Akonadi::Item(), QDate() );
  }
}

bool AgendaView::displayIncidence( const Akonadi::Item &aitem, bool createSelected )
{
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  if ( !incidence || incidence->hasRecurrenceId() ) {
    return false;
  }

  KCalCore::Todo::Ptr todo = CalendarSupport::todo( aitem );
  if ( todo && ( !preferences()->showTodosAgendaView() || !todo->hasDueDate() ) ) {
    return false;
  }

  KCalCore::Event::Ptr event = CalendarSupport::event( aitem );
  const QDate today = QDate::currentDate();
  KCalCore::DateTimeList::iterator t;

  const KDateTime::Spec timeSpec = preferences()->timeSpec();

  KDateTime firstVisibleDateTime( d->mSelectedDates.first(), timeSpec );
  KDateTime lastVisibleDateTime( d->mSelectedDates.last(), timeSpec );

  // Optimization, very cheap operation that discards incidences that aren't in the timespan
  if ( !d->mightBeVisible( incidence ) ) {
    return false;
  }

  lastVisibleDateTime.setTime( QTime( 23, 59, 59, 59 ) );
  firstVisibleDateTime.setTime( QTime( 0, 0 ) );
  KCalCore::DateTimeList dateTimeList;

  const KDateTime incDtStart = incidence->dtStart().toTimeSpec( timeSpec );
  const KDateTime incDtEnd =
    incidence->dateTime( KCalCore::Incidence::RoleEnd ).toTimeSpec( timeSpec );

  bool alreadyAddedToday = false;

  if ( incidence->recurs() ) {
    // timed incidences occur in [dtStart(), dtEnd()[
    // all-day incidences occur in [dtStart(), dtEnd()]
    // so we subtract 1 second in the timed case
    const int secsToAdd = incidence->allDay() ? 0 : -1;
    const int eventDuration = event ? incDtStart.daysTo( incDtEnd.addSecs( secsToAdd ) ) : 0;

    // if there's a multiday event that starts before firstVisibleDateTime but ends after
    // lets include it. timesInInterval() ignores incidences that aren't totaly inside
    // the range
    const KDateTime startDateTimeWithOffset = firstVisibleDateTime.addDays( -eventDuration );

    KCalCore::OccurrenceIterator rIt( *calendar(), incidence,
                                      startDateTimeWithOffset, lastVisibleDateTime );
    while ( rIt.hasNext() ) {
      rIt.next();
      const KDateTime occurrenceDate( rIt.occurrenceStartDate().toTimeSpec( timeSpec ) );
      const bool makesDayBusy =
        preferences()->colorAgendaBusyDays() && makesWholeDayBusy( rIt.incidence() );
      if ( makesDayBusy ) {
        KCalCore::Event::List &busyEvents = d->mBusyDays[occurrenceDate.date()];
        busyEvents.append( event );
      }

      const Akonadi::Item item = calendar()->item( rIt.incidence() );
      if ( !item.isValid() ) {
        qWarning() << "Couldn't find item for "
                   << rIt.incidence()->uid() << rIt.incidence()->recurrenceId().toString();
        continue;
      }
      Q_ASSERT( item.hasPayload() );
      if ( occurrenceDate.date() == today ) {
        alreadyAddedToday = true;
      }
      d->insertIncidence( item, occurrenceDate, createSelected );
    }

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

  // ToDo items shall be displayed today if they are overdue
  const KDateTime dateTimeToday = KDateTime( today, timeSpec );
  if ( todo &&
       todo->isOverdue() &&
       dateTimeToday >= firstVisibleDateTime &&
       dateTimeToday <= lastVisibleDateTime ) {
    /* If there's a recurring instance showing up today don't add "today" again
     * we don't want the event to appear duplicated */
    if ( !alreadyAddedToday ) {
      dateTimeList += dateTimeToday;
    }
  }

  const bool makesDayBusy = preferences()->colorAgendaBusyDays() && makesWholeDayBusy( incidence );
  for ( t = dateTimeList.begin(); t != dateTimeList.end(); ++t ) {
    if ( makesDayBusy ) {
      KCalCore::Event::List &busyEvents = d->mBusyDays[(*t).date()];
      busyEvents.append( event );
    }

    d->insertIncidence( aitem, t->toTimeSpec( timeSpec ), createSelected );
  }

  // Can be multiday
  if ( event && makesDayBusy && event->isMultiDay() ) {
    const QDate lastVisibleDate = d->mSelectedDates.last();
    for ( QDate date = event->dtStart().date();
          date <= event->dtEnd().date() && date <= lastVisibleDate ;
          date = date.addDays( 1 ) ) {
      KCalCore::Event::List &busyEvents = d->mBusyDays[date];
      busyEvents.append( event );
    }
  }

  return !dateTimeList.isEmpty();
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

void AgendaView::slotIncidencesDropped( const QList<KUrl> &items, const QPoint &gpos, bool allDay )
{
  Q_UNUSED( items );
  Q_UNUSED( gpos );
  Q_UNUSED( allDay );

#ifdef AKONADI_PORT_DISABLED // one item -> multiple items, Incidence* -> akonadi item url (we might have to fetch the items here first!)
  if ( gpos.x() < 0 || gpos.y() < 0 ) {
    return;
  }

  const QDate day = d->mSelectedDates[gpos.x()];
  const QTime time = d->mAgenda->gyToTime( gpos.y() );
  KDateTime newTime( day, time, preferences()->timeSpec() );
  newTime.setDateOnly( allDay );

  Todo::Ptr todo = CalendarSupport::todo( todoItem );
  if ( todo &&  dynamic_cast<Akonadi::ETMCalendar*>( calendar() ) ) {
    const Akonadi::Item existingTodoItem =
      calendar()->itemForIncidence( calendar()->todo( todo->uid() ) );

    if ( Todo::Ptr existingTodo = CalendarSupport::todo( existingTodoItem ) ) {
      qDebug() << "Drop existing Todo";
      Todo::Ptr oldTodo( existingTodo->clone() );
      if ( changer() ) {
        existingTodo->setDtDue( newTime );
        existingTodo->setAllDay( allDay );
        changer()->modifyIncidence( existingTodoItem, oldTodo, this );
      } else {
        KMessageBox::sorry( this, i18n( "Unable to modify this to-do, "
                                        "because it cannot be locked." ) );
      }
    } else {
      qDebug() << "Drop new Todo";
      todo->setDtDue( newTime );
      todo->setAllDay( allDay );
      if ( !changer()->addIncidence( todo, this ) ) {
        KMessageBox::sorry( this,
                            i18n( "Unable to save %1 \"%2\".",
                            i18n( todo->type() ), todo->summary() ) );
      }
    }
  }
#else
  qDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
}

void AgendaView::slotIncidencesDropped( const KCalCore::Incidence::List &incidences,
                                        const QPoint &gpos, bool allDay )
{
  if ( gpos.x() < 0 || gpos.y() < 0 ) {
    return;
  }

  const QDate day = d->mSelectedDates[gpos.x()];
  const QTime time = d->mAgenda->gyToTime( gpos.y() );
  KDateTime newTime( day, time, preferences()->timeSpec() );
  newTime.setDateOnly( allDay );

  Q_FOREACH ( const KCalCore::Incidence::Ptr &incidence, incidences ) {
    const Akonadi::Item existingItem = calendar()->item( incidence );
    const bool existsInSameCollection = existingItem.isValid() &&
                                               ( existingItem.storageCollectionId() == collectionId() || collectionId() == -1 );

    if ( existingItem.isValid() && existsInSameCollection ) {
      KCalCore::Incidence::Ptr newIncidence = existingItem.payload<KCalCore::Incidence::Ptr>();
      KCalCore::Incidence::Ptr oldIncidence( newIncidence->clone() );

      if ( newIncidence->dtStart() == newTime && newIncidence->allDay() == allDay ) {
        // Nothing changed
        continue;
      }

      newIncidence->setAllDay( allDay );
      newIncidence->setDateTime( newTime, KCalCore::Incidence::RoleDnD );

      changer()->modifyIncidence( existingItem, oldIncidence, this );
    } else { // Create a new one
      // The drop came from another application create a new incidence
      incidence->setDateTime( newTime, KCalCore::Incidence::RoleDnD );
      incidence->setAllDay( allDay );
      incidence->setUid( KCalCore::CalFormat::createUniqueId() );
      Akonadi::Collection collection( collectionId() );
      const bool added = -1 != changer()->createIncidence( incidence, collection, this );

      if ( added ) {
        // TODO: make async
        if ( existingItem.isValid() ) { // Dragged from one agenda to another, delete origin
          changer()->deleteIncidence( existingItem );
        }
      }
    }
  }
}
void AgendaView::startDrag( const Akonadi::Item &incidence )
{
  if ( !calendar() ) {
    qCritical() << "No Calendar set";
    return;
  }
#ifndef KORG_NODND
  if ( QDrag *drag = CalendarSupport::createDrag( incidence, calendar()->timeSpec(), this ) ) {
    drag->exec();
  }
#else
  Q_UNUSED( incidence );
#endif
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
    updateConfig();
  }
}

void AgendaView::writeSettings( KConfig *config )
{
  KConfigGroup group = config->group( "Views" );

  QList<int> list = d->mSplitterAgenda->sizes();
  group.writeEntry( "Separator AgendaView", list );
}

QVector<bool> AgendaView::busyDayMask() const
{
  if ( d->mSelectedDates.isEmpty() || !d->mSelectedDates[0].isValid() ) {
    return QVector<bool>();
  }

  QVector<bool> busyDayMask;
  busyDayMask.resize( d->mSelectedDates.count() );

  for ( int i = 0; i < d->mSelectedDates.count(); ++i ) {
    busyDayMask[i] = !d->mBusyDays[d->mSelectedDates[i]].isEmpty();
  }

  return busyDayMask;
}

void AgendaView::setHolidayMasks()
{
  if ( d->mSelectedDates.isEmpty() || !d->mSelectedDates[0].isValid() ) {
    return;
  }

  d->mHolidayMask.resize( d->mSelectedDates.count() + 1 );

  const QList<QDate> workDays = CalendarSupport::workDays( d->mSelectedDates.first().addDays( -1 ),
                                                           d->mSelectedDates.last() );
  for ( int i = 0; i < d->mSelectedDates.count(); ++i ) {
    d->mHolidayMask[i] = !workDays.contains( d->mSelectedDates[ i ] );
  }

  // Store the information about the day before the visible area (needed for
  // overnight working hours) in the last bit of the mask:
  bool showDay = !workDays.contains( d->mSelectedDates[ 0 ].addDays( -1 ) );
  d->mHolidayMask[ d->mSelectedDates.count() ] = showDay;

  d->mAgenda->setHolidayMask( &d->mHolidayMask );
  d->mAllDayAgenda->setHolidayMask( &d->mHolidayMask );
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

void AgendaView::newTimeSpanSelected( const QPoint &start, const QPoint &end )
{
  if ( !d->mSelectedDates.count() ) {
    return;
  }

  d->mTimeSpanInAllDay = false;

  const QDate dayStart =
    d-> mSelectedDates[ qBound( 0, start.x(), (int)d->mSelectedDates.size() - 1 ) ];
  const QDate dayEnd =
    d->mSelectedDates[ qBound( 0, end.x(), (int)d->mSelectedDates.size() - 1 ) ];

  const QTime timeStart = d->mAgenda->gyToTime( start.y() );
  const QTime timeEnd = d->mAgenda->gyToTime( end.y() + 1 );

  d->mTimeSpanBegin = QDateTime( dayStart, timeStart );
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

void AgendaView::removeIncidence( const KCalCore::Incidence::Ptr &incidence )
{
  // Don't wrap this in a if ( incidence->isAllDay ) because whe all day
  // property might have changed
  d->mAllDayAgenda->removeIncidence( incidence );
  d->mAgenda->removeIncidence( incidence );

  if ( !incidence->hasRecurrenceId() ) {
    KCalCore::Incidence::List exceptions = calendar()->instances( incidence );
    foreach ( const KCalCore::Incidence::Ptr &exception, exceptions ) {
      if ( exception->allDay() ) {
        d->mAllDayAgenda->removeIncidence( exception );
      } else {
        d->mAgenda->removeIncidence( exception );
      }
    }
  }
}

void AgendaView::updateEventIndicators()
{
  d->mUpdateEventIndicatorsScheduled = false;
  d->mMinY = d->mAgenda->minContentsY();
  d->mMaxY = d->mAgenda->maxContentsY();

  d->mAgenda->checkScrollBoundaries();
  updateEventIndicatorTop( d->mAgenda->visibleContentsYMin() );
  updateEventIndicatorBottom( d->mAgenda->visibleContentsYMax() );
}

void AgendaView::setIncidenceChanger( Akonadi::IncidenceChanger *changer )
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

Agenda *AgendaView::agenda() const
{
  return d->mAgenda;
}

Agenda *AgendaView::allDayAgenda() const
{
  return d->mAllDayAgenda;
}

QSplitter *AgendaView::splitter() const
{
  return d->mSplitterAgenda;
}

bool AgendaView::filterByCollectionSelection( const Akonadi::Item &incidence )
{
  if ( customCollectionSelection() ) {
    return customCollectionSelection()->contains( incidence.parentCollection().id() );
  }

  if ( collectionId() < 0 ) {
    return true;
  } else {
    return collectionId() == incidence.storageCollectionId();
  }
}

void AgendaView::alignAgendas()
{
  // resize dummy widget so the allday agenda lines up with the hourly agenda.
  d->mDummyAllDayLeft->setFixedWidth( -SPACING + d->mTimeLabelsZone->width() -
                                      ( d->mIsSideBySide ? 0 : d->mTimeBarHeaderFrame->width() ) );

  // Must be async, so they are centered
  createDayLabels( true );
}

CalendarDecoration::Decoration *AgendaView::Private::loadCalendarDecoration( const QString &name )
{
  const QString type = CalendarSupport::Plugin::serviceType();
  const int version = CalendarSupport::Plugin::interfaceVersion();

  QString constraint;
  if ( version >= 0 ) {
    constraint =
      QString::fromLatin1( "[X-KDE-PluginInterfaceVersion] == %1" ).arg( QString::number( version ) );
  }

  KService::List list = KServiceTypeTrader::self()->query( type, constraint );

  KService::List::ConstIterator it;
  for ( it = list.constBegin(); it != list.constEnd(); ++it ) {
    if ( (*it)->desktopEntryName() == name ) {
      KService::Ptr service = *it;
      KPluginLoader loader( *service );
      KPluginFactory *factory = loader.factory();

      if ( !factory ) {
        qDebug() << "Factory creation failed";
        return 0;
      }

      CalendarDecoration::DecorationFactory *pluginFactory =
        static_cast<CalendarDecoration::DecorationFactory *>( factory );

      if ( !pluginFactory ) {
        qDebug() << "Cast failed";
        return 0;
      }

      return pluginFactory->createPluginFactory();
    }
  }

  return 0;
}

void AgendaView::setChanges( EventView::Changes changes )
{
  d->setChanges( changes );
}

void AgendaView::scheduleUpdateEventIndicators()
{
  if ( !d->mUpdateEventIndicatorsScheduled ) {
    d->mUpdateEventIndicatorsScheduled = true;
    QTimer::singleShot( 0, this, SLOT(updateEventIndicators()) );
  }
}


// kate: space-indent on; indent-width 2; replace-tabs on;

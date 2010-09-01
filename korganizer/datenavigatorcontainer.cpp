/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
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

#include <kdebug.h>
#include <klocale.h>

#include "koglobals.h"
#include "navigatorbar.h"
#include "kdatenavigator.h"
#include "kodaymatrix.h"

#include <kcalendarsystem.h>
#include <kdialog.h>

#include "datenavigatorcontainer.h"

#include <tqwhatsthis.h>
#include <tqtimer.h>

DateNavigatorContainer::DateNavigatorContainer( TQWidget *parent,
                                                const char *name )
  : TQFrame( parent, name ), mCalendar( 0 ),
    mHorizontalCount( 1 ), mVerticalCount( 1 )
{
  mExtraViews.setAutoDelete( true );
  setFrameStyle( TQFrame::Sunken | TQFrame::StyledPanel );

  mNavigatorView = new KDateNavigator( this, name );
  TQWhatsThis::add( mNavigatorView,
                   i18n( "<qt><p>Select the dates you want to "
                         "display in KOrganizer's main view here. Hold down the "
                         "mouse button to select more than one day.</p>"
                         "<p>Press the top buttons to browse to the next "
                         "/ previous months or years.</p>"
                         "<p>Each line shows a week. The number in the left "
                         "column is the number of the week in the year. "
                         "Press it to select the whole week.</p>"
                         "</qt>" ) );

  connectNavigatorView( mNavigatorView );
}

DateNavigatorContainer::~DateNavigatorContainer()
{
}

void DateNavigatorContainer::connectNavigatorView( KDateNavigator *v )
{
  connect( v, TQT_SIGNAL( datesSelected( const KCal::DateList & ) ),
           TQT_SIGNAL( datesSelected( const KCal::DateList & ) ) );
  connect( v, TQT_SIGNAL( incidenceDropped( Incidence *, const TQDate & ) ),
           TQT_SIGNAL( incidenceDropped( Incidence *, const TQDate & ) ) );
  connect( v, TQT_SIGNAL( incidenceDroppedMove( Incidence *, const TQDate & ) ),
           TQT_SIGNAL( incidenceDroppedMove( Incidence *, const TQDate & ) ) );
  connect( v, TQT_SIGNAL( weekClicked( const TQDate & ) ),
           TQT_SIGNAL( weekClicked( const TQDate & ) ) );

  connect( v, TQT_SIGNAL( goPrevious() ), TQT_SIGNAL( goPrevious() ) );
  connect( v, TQT_SIGNAL( goNext() ), TQT_SIGNAL( goNext() ) );

  connect( v, TQT_SIGNAL( nextYearClicked() ), TQT_SIGNAL( nextYearClicked() ) );
  connect( v, TQT_SIGNAL( prevYearClicked() ), TQT_SIGNAL( prevYearClicked() ) );

  connect( v, TQT_SIGNAL( prevMonthClicked() ), this, TQT_SLOT( goPrevMonth() ) );
  connect( v, TQT_SIGNAL( nextMonthClicked() ), this, TQT_SLOT( goNextMonth() ) );

  connect( v, TQT_SIGNAL( monthSelected( int ) ), TQT_SIGNAL( monthSelected( int ) ) );
  connect( v, TQT_SIGNAL( yearSelected( int ) ), TQT_SIGNAL( yearSelected( int ) ) );
}

void DateNavigatorContainer::setCalendar( Calendar *cal )
{
  mCalendar = cal;
  mNavigatorView->setCalendar( cal );
  KDateNavigator *n;
  for( n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    n->setCalendar( cal );
  }
}

// TODO_Recurrence: let the navigators update just once, and tell them that
// if data has changed or just the selection (because then the list of dayss
// with events doesn't have to be updated if the month stayed the same
void DateNavigatorContainer::updateDayMatrix()
{
  mNavigatorView->updateDayMatrix();
  KDateNavigator *n;
  for( n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    n->updateDayMatrix();
  }
}

void DateNavigatorContainer::updateToday()
{
  mNavigatorView->updateToday();
  KDateNavigator *n;
  for( n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    n->updateToday();
  }
}

void DateNavigatorContainer::setUpdateNeeded()
{
  mNavigatorView->setUpdateNeeded();
  KDateNavigator *n;
  for ( n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    n->setUpdateNeeded();
  }
}

void DateNavigatorContainer::updateView()
{
  mNavigatorView->updateView();
  KDateNavigator *n;
  for ( n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    n->setUpdateNeeded();
  }
}

void DateNavigatorContainer::updateConfig()
{
  mNavigatorView->updateConfig();
  KDateNavigator *n;
  for( n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    n->updateConfig();
  }
}

void DateNavigatorContainer::selectDates( const DateList &dateList, const TQDate &preferredMonth )
{
  if ( !dateList.isEmpty() ) {
    TQDate start( dateList.first() );
    TQDate end( dateList.last() );
    TQDate navfirst( mNavigatorView->startDate() );
    TQDate navsecond; // start of the second shown month if existant
    TQDate navlast;
    if ( !mExtraViews.isEmpty() ) {
      navlast = mExtraViews.last()->endDate();
      navsecond = mExtraViews.first()->startDate();
    } else {
      navlast = mNavigatorView->endDate();
      navsecond = navfirst;
    }

    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

    // If the datelist crosses months we won't know which month to show
    // so we read what's in preferredMonth
    const bool changingMonth = ( preferredMonth.isValid()  &&
                                 calSys->month( mNavigatorView->month() ) != calSys->month( preferredMonth ) );

    if ( start < navfirst // <- start should always be visible
         // end is not visible and we have a spare month at the beginning:
         || ( end > navlast && start >= navsecond )
         || changingMonth ) {

      if ( preferredMonth.isValid() ) {
        setBaseDates( preferredMonth );
      } else {
        setBaseDates( start );
      }
    }

    mNavigatorView->selectDates( dateList );
    KDateNavigator *n = mExtraViews.first();
    while ( n ) {
      n->selectDates( dateList );
      n = mExtraViews.next();
    }
  }
}

void DateNavigatorContainer::setBaseDates( const TQDate &start )
{
  TQDate baseDate = start;
  mNavigatorView->setBaseDate( baseDate );
  for( KDateNavigator *n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    baseDate = KOGlobals::self()->calendarSystem()->addMonths( baseDate, 1 );
    n->setBaseDate( baseDate );
  }
}

void DateNavigatorContainer::resizeEvent( TQResizeEvent * )
{
#if 0
  kdDebug(5850) << "DateNavigatorContainer::resizeEvent()" << endl;
  kdDebug(5850) << "  CURRENT SIZE: " << size() << endl;
  kdDebug(5850) << "  MINIMUM SIZEHINT: " << minimumSizeHint() << endl;
  kdDebug(5850) << "  SIZEHINT: " << sizeHint() << endl;
  kdDebug(5850) << "  MINIMUM SIZE: " << minimumSize() << endl;
#endif
  TQTimer::singleShot( 0, this, TQT_SLOT( resizeAllContents() ) );
}

void DateNavigatorContainer::resizeAllContents()
{
  TQSize minSize = mNavigatorView->minimumSizeHint();

//  kdDebug(5850) << "  NAVIGATORVIEW minimumSizeHint: " << minSize << endl;

  int margin = KDialog::spacingHint();
  int verticalCount = ( size().height() - margin*2 ) / minSize.height();
  int horizontalCount = ( size().width() - margin*2 ) / minSize.width();

  if ( horizontalCount != mHorizontalCount ||
       verticalCount != mVerticalCount ) {
    uint count = horizontalCount * verticalCount;
    if ( count == 0 ) {
      return;
    }

    while ( count > ( mExtraViews.count() + 1 ) ) {
      KDateNavigator *n = new KDateNavigator( this );
      mExtraViews.append( n );
      n->setCalendar( mCalendar );
      connectNavigatorView( n );
    }

    while ( count < ( mExtraViews.count() + 1 ) ) {
      mExtraViews.removeLast();
    }

    mHorizontalCount = horizontalCount;
    mVerticalCount = verticalCount;
    setBaseDates( mNavigatorView->selectedDates().first() );
    selectDates( mNavigatorView->selectedDates() );
    for( KDateNavigator *n = mExtraViews.first(); n; n = mExtraViews.next() ) {
      n->show();
    }
  }

  int height = (size().height() - margin*2) / verticalCount;
  int width = (size().width() - margin*2) / horizontalCount;

  NavigatorBar *bar = mNavigatorView->navigatorBar();
  if ( horizontalCount > 1 ) {
    bar->showButtons( true, false );
  } else {
    bar->showButtons( true, true );
  }

  mNavigatorView->setGeometry(
      ( ( (KOGlobals::self()->reverseLayout())?(horizontalCount-1):0) * width ) + margin,
        margin, width, height );

  for( uint i = 0; i < mExtraViews.count(); ++i ) {
    int x = ( i + 1 ) % horizontalCount;
    int y = ( i + 1 ) / horizontalCount;

    KDateNavigator *view = mExtraViews.at( i );
    bar = view->navigatorBar();
    if ( y > 0 ) {
      bar->showButtons( false, false );
    } else {
      if ( x + 1 == horizontalCount ) {
        bar->showButtons( false, true );
      } else {
        bar->showButtons( false, false );
      }
    }
    view->setGeometry(
        ( ( (KOGlobals::self()->reverseLayout())?(horizontalCount-1-x):x) * width ) + margin,
          ( y * height ) + margin, width, height );
  }
}

TQSize DateNavigatorContainer::minimumSizeHint() const
{
  int margin = KDialog::spacingHint() * 2;
  return mNavigatorView->minimumSizeHint() + TQSize( margin, margin );
}

TQSize DateNavigatorContainer::sizeHint() const
{
  int margin = KDialog::spacingHint() * 2;
  return mNavigatorView->sizeHint() + TQSize( margin, margin );
}

void DateNavigatorContainer::goNextMonth()
{
  const QPair<TQDate,TQDate> p = dateLimits( 1 );

  emit nextMonthClicked( mNavigatorView->month(),
                         p.first,
                         p.second);
}

void DateNavigatorContainer::goPrevMonth()
{
  const QPair<TQDate,TQDate> p = dateLimits( -1 );

  emit prevMonthClicked( mNavigatorView->month(),
                         p.first,
                         p.second );
}

QPair<TQDate,TQDate> DateNavigatorContainer::dateLimits( int offset )
{
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
  TQDate firstMonth, lastMonth;
  if ( mExtraViews.isEmpty() ) {
    lastMonth = mNavigatorView->month();
  } else {
    lastMonth = mExtraViews.last()->month();
  }

  firstMonth = calSys->addMonths( mNavigatorView->month(), offset );
  lastMonth = calSys->addMonths( lastMonth, offset );

  QPair<TQDate,TQDate> firstMonthBoundary = KODayMatrix::matrixLimits( firstMonth );
  QPair<TQDate,TQDate> lastMonthBoundary = KODayMatrix::matrixLimits( lastMonth );

  return qMakePair( firstMonthBoundary.first, lastMonthBoundary.second );
}

#include "datenavigatorcontainer.moc"

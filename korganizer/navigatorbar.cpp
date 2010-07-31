/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <tqstring.h>
#include <tqtooltip.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqframe.h>
#include <tqpopupmenu.h>
#include <tqlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "koglobals.h"
#include "koprefs.h"

#include <kcalendarsystem.h>

#include "navigatorbar.h"

ActiveLabel::ActiveLabel( TQWidget *parent, const char *name )
  : TQLabel( parent, name )
{
}

void ActiveLabel::mouseReleaseEvent( TQMouseEvent * )
{
  emit clicked();
}


NavigatorBar::NavigatorBar( TQWidget *parent, const char *name )
  : TQWidget( parent, name ), mHasMinWidth( false )
{
  TQFont tfont = font();
  tfont.setPointSize( 10 );
  tfont.setBold( false );

  bool isRTL = KOGlobals::self()->reverseLayout();

  TQPixmap pix;
  // Create backward navigation buttons
  mPrevYear = new TQPushButton( this );
  pix = KOGlobals::self()->smallIcon( isRTL ? "2rightarrow" : "2leftarrow" );
  mPrevYear->setPixmap( pix );
  mPrevYear->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  TQToolTip::add( mPrevYear, i18n("Previous year") );

  pix = KOGlobals::self()->smallIcon( isRTL ? "1rightarrow" : "1leftarrow");
  mPrevMonth = new TQPushButton( this );
  mPrevMonth->setPixmap( pix );
  mPrevMonth->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  TQToolTip::add( mPrevMonth, i18n("Previous month") );

  // Create forward navigation buttons
  pix = KOGlobals::self()->smallIcon( isRTL ? "1leftarrow" : "1rightarrow");
  mNextMonth = new TQPushButton( this );
  mNextMonth->setPixmap( pix );
  mNextMonth->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  TQToolTip::add( mNextMonth, i18n("Next month") );

  pix = KOGlobals::self()->smallIcon( isRTL ? "2leftarrow" : "2rightarrow");
  mNextYear = new TQPushButton( this );
  mNextYear->setPixmap( pix );
  mNextYear->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  TQToolTip::add( mNextYear, i18n("Next year") );

  // Create month name button
  mMonth = new ActiveLabel( this );
  mMonth->setFont( tfont );
  mMonth->setAlignment( AlignCenter );
  mMonth->setMinimumHeight( mPrevYear->sizeHint().height() );
  TQToolTip::add( mMonth, i18n("Select a month") );

  // set up control frame layout
  TQBoxLayout *ctrlLayout = new TQHBoxLayout( this, 0, 4 );
  ctrlLayout->addWidget( mPrevYear, 3 );
  ctrlLayout->addWidget( mPrevMonth, 3 );
  ctrlLayout->addWidget( mMonth, 3 );
  ctrlLayout->addWidget( mNextMonth, 3 );
  ctrlLayout->addWidget( mNextYear, 3 );

  connect( mPrevYear, TQT_SIGNAL( clicked() ), TQT_SIGNAL( goPrevYear() ) );
  connect( mPrevMonth, TQT_SIGNAL( clicked() ), TQT_SIGNAL( goPrevMonth() ) );
  connect( mNextMonth, TQT_SIGNAL( clicked() ), TQT_SIGNAL( goNextMonth() ) );
  connect( mNextYear, TQT_SIGNAL( clicked() ), TQT_SIGNAL( goNextYear() ) );
  connect( mMonth, TQT_SIGNAL( clicked() ), TQT_SLOT( selectMonth() ) );
}

NavigatorBar::~NavigatorBar()
{
}

void NavigatorBar::showButtons( bool left, bool right )
{
  if ( left ) {
    mPrevYear->show();
    mPrevMonth->show();
  } else {
    mPrevYear->hide();
    mPrevMonth->hide();
  }

  if ( right ) {
    mNextYear->show();
    mNextMonth->show();
  } else {
    mNextYear->hide();
    mNextMonth->hide();
  }

}

void NavigatorBar::selectDates( const KCal::DateList &dateList )
{
  if ( dateList.count() > 0 ) {
    mDate = dateList.first();

    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

    if ( !mHasMinWidth ) {
      // Set minimum width to width of widest month name label
      int i;
      int maxwidth = 0;

      for( i = 1; i <= calSys->monthsInYear( mDate ); ++i ) {
        int w = TQFontMetrics( mMonth->font() ).width( TQString("%1 8888")
            .arg( calSys->monthName( i, calSys->year( mDate ) ) ) );
        if ( w > maxwidth ) maxwidth = w;
      }
      mMonth->setMinimumWidth( maxwidth );

      mHasMinWidth = true;
    }

    // compute the label at the top of the navigator
    mMonth->setText( i18n( "monthname year", "%1 %2" )
                     .arg( calSys->monthName( mDate ) )
                     .arg( calSys->year( mDate ) ) );
  }
}

void NavigatorBar::selectMonth()
{
  // every year can have different month names (in some calendar systems)
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  int i, month, months = calSys->monthsInYear( mDate );

  TQPopupMenu *popup = new TQPopupMenu( mMonth );

  for ( i = 1; i <= months; i++ )
    popup->insertItem( calSys->monthName( i, calSys->year( mDate ) ), i );

  popup->setActiveItem( calSys->month( mDate ) - 1 );
  popup->setMinimumWidth( mMonth->width() );

  if ( ( month = popup->exec( mMonth->mapToGlobal( TQPoint( 0, 0 ) ),
                              calSys->month( mDate ) - 1 ) ) == -1 ) {
    delete popup;
    return;  // canceled
  }

  emit goMonth( month );

  delete popup;
}

#include "navigatorbar.moc"

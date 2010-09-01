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

  // Create a horizontal spacers
  TQSpacerItem *frontSpacer = new TQSpacerItem( 50, 1, TQSizePolicy::Expanding );
  TQSpacerItem *endSpacer = new TQSpacerItem( 50, 1, TQSizePolicy::Expanding );

  bool isRTL = KOGlobals::self()->reverseLayout();

  TQPixmap pix;
  // Create backward navigation buttons
  pix = KOGlobals::self()->smallIcon( isRTL ? "2rightarrow" : "2leftarrow" );
  mPrevYear = new TQPushButton( this );
  mPrevYear->setPixmap( pix );
  mPrevYear->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  TQToolTip::add( mPrevYear, i18n( "Previous year" ) );

  pix = KOGlobals::self()->smallIcon( isRTL ? "1rightarrow" : "1leftarrow");
  mPrevMonth = new TQPushButton( this );
  mPrevMonth->setPixmap( pix );
  mPrevMonth->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  TQToolTip::add( mPrevMonth, i18n( "Previous month" ) );

  // Create forward navigation buttons
  pix = KOGlobals::self()->smallIcon( isRTL ? "1leftarrow" : "1rightarrow");
  mNextMonth = new TQPushButton( this );
  mNextMonth->setPixmap( pix );
  mNextMonth->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  TQToolTip::add( mNextMonth, i18n( "Next month" ) );

  pix = KOGlobals::self()->smallIcon( isRTL ? "2leftarrow" : "2rightarrow");
  mNextYear = new TQPushButton( this );
  mNextYear->setPixmap( pix );
  mNextYear->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  TQToolTip::add( mNextYear, i18n( "Next year" ) );

  // Create month name button
  mMonth = new ActiveLabel( this );
  mMonth->setFont( tfont );
  mMonth->setAlignment( AlignCenter );
  mMonth->setMinimumHeight( mPrevYear->sizeHint().height() );
  TQToolTip::add( mMonth, i18n( "Select a month" ) );

  // Create year button
  mYear = new ActiveLabel( this );
  mYear->setFont( tfont );
  mYear->setAlignment( AlignCenter );
  mYear->setMinimumHeight( mPrevYear->sizeHint().height() );
  TQToolTip::add( mYear, i18n( "Select a year" ) );

  // set up control frame layout
  TQHBoxLayout *ctrlLayout = new TQHBoxLayout( this );
  ctrlLayout->addWidget( mPrevYear );
  ctrlLayout->addWidget( mPrevMonth );
  ctrlLayout->addItem( frontSpacer );
  ctrlLayout->addWidget( mMonth );
  ctrlLayout->addWidget( mYear );
  ctrlLayout->addItem( endSpacer );
  ctrlLayout->addWidget( mNextMonth );
  ctrlLayout->addWidget( mNextYear );

  connect( mPrevYear, TQT_SIGNAL( clicked() ), TQT_SIGNAL( prevYearClicked() ) );
  connect( mPrevMonth, TQT_SIGNAL( clicked() ), TQT_SIGNAL( prevMonthClicked() ) );
  connect( mNextMonth, TQT_SIGNAL( clicked() ), TQT_SIGNAL( nextMonthClicked() ) );
  connect( mNextYear, TQT_SIGNAL( clicked() ), TQT_SIGNAL( nextYearClicked() ) );
  connect( mMonth, TQT_SIGNAL( clicked() ), TQT_SLOT( selectMonthFromMenu() ) );
  connect( mYear, TQT_SIGNAL( clicked() ), TQT_SLOT( selectYearFromMenu() ) );
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

    // Set minimum width to width of widest month name label
    int i;
    int maxwidth = 0;

    for( i = 1; i <= calSys->monthsInYear( mDate ); ++i ) {
      int w = TQFontMetrics( mMonth->font() ).
              width( TQString( "%1" ).
                     arg( calSys->monthName( i, calSys->year( mDate ) ) ) );
      if ( w > maxwidth ) {
        maxwidth = w;
      }
    }
    mMonth->setMinimumWidth( maxwidth );

    mHasMinWidth = true;

    // set the label text at the top of the navigator
    mMonth->setText( i18n( "monthname", "%1" ).arg( calSys->monthName( mDate ) ) );
    mYear->setText( i18n( "4 digit year", "%1" ).arg( calSys->yearString( mDate, false ) ) );
  }
}

void NavigatorBar::selectMonthFromMenu()
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

  emit monthSelected( month );

  delete popup;
}

void NavigatorBar::selectYearFromMenu()
{
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  int year = calSys->year( mDate );
  int years = 11;  // odd number (show a few years ago -> a few years from now)
  int minYear = year - ( years / 3 );

  TQPopupMenu *popup = new TQPopupMenu( mYear );

  TQString yearStr;
  int y = minYear;
  for ( int i=0; i < years; i++ ) {
    popup->insertItem( yearStr.setNum( y ), i );
    y++;
  }
  popup->setActiveItem( year - minYear );

  if ( ( year = popup->exec( mYear->mapToGlobal( TQPoint( 0, 0 ) ),
                             year - minYear ) ) == -1 ) {
    delete popup;
    return;  // canceled
  }

  emit yearSelected( year + minYear );

  delete popup;
}

#include "navigatorbar.moc"

/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include <tqpainter.h>
#include <tqlayout.h>
#include <tqframe.h>
#include <tqlabel.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kcalendarsystem.h>
#include <kwordwrap.h>

#include "calprintpluginbase.h"
#include "cellitem.h"

#ifndef KORG_NOPRINTER

inline int round(const double x)
{
  return int(x > 0.0 ? x + 0.5 : x - 0.5);
}

static TQString cleanStr( const TQString &instr )
{
  TQString ret = instr;
  return ret.replace( '\n', ' ' );
}

/******************************************************************
 **              The Todo positioning structure                  **
 ******************************************************************/
class CalPrintPluginBase::TodoParentStart
{
  public:
    TodoParentStart( TQRect pt = TQRect(), bool page = true )
      : mRect( pt ), mSamePage( page ) {}

    TQRect mRect;
    bool mSamePage;
};


/******************************************************************
 **                     The Print item                           **
 ******************************************************************/


class PrintCellItem : public KOrg::CellItem
{
  public:
    PrintCellItem( Event *event, const TQDateTime &start, const TQDateTime &end )
      : mEvent( event ), mStart( start), mEnd( end )
    {
    }

    Event *event() const { return mEvent; }

    TQString label() const { return mEvent->summary(); }

    TQDateTime start() const { return mStart; }
    TQDateTime end() const { return mEnd; }

    /** Calculate the start and end date/time of the recurrence that
        happens on the given day */
    bool overlaps( KOrg::CellItem *o ) const
    {
      PrintCellItem *other = static_cast<PrintCellItem *>( o );

#if 0
      kdDebug(5850) << "PrintCellItem::overlaps() " << event()->summary()
                    << " <-> " << other->event()->summary() << endl;
      kdDebug(5850) << "  start     : " << start.toString() << endl;
      kdDebug(5850) << "  end       : " << end.toString() << endl;
      kdDebug(5850) << "  otherStart: " << otherStart.toString() << endl;
      kdDebug(5850) << "  otherEnd  : " << otherEnd.toString() << endl;
#endif

      return !( other->start() >= end() || other->end() <= start() );
    }

  private:
    Event *mEvent;
    TQDateTime mStart, mEnd;
};




/******************************************************************
 **                    The Print plugin                          **
 ******************************************************************/


CalPrintPluginBase::CalPrintPluginBase() : PrintPlugin(), mUseColors( true ),
    mHeaderHeight( -1 ), mSubHeaderHeight( SUBHEADER_HEIGHT ), mFooterHeight( -1 ),
    mMargin( MARGIN_SIZE ), mPadding( PADDING_SIZE), mCalSys( 0 )
{
}
CalPrintPluginBase::~CalPrintPluginBase()
{
}



TQWidget *CalPrintPluginBase::createConfigWidget( TQWidget *w )
{
  TQFrame *wdg = new TQFrame( w );
  TQVBoxLayout *layout = new TQVBoxLayout( wdg );

  TQLabel *title = new TQLabel( description(), wdg );
  TQFont titleFont( title->font() );
  titleFont.setPointSize( 20 );
  titleFont.setBold( true );
  title->setFont( titleFont );

  layout->addWidget( title );
  layout->addWidget( new TQLabel( info(), wdg ) );
  layout->addSpacing( 20 );
  layout->addWidget( new TQLabel( i18n("This printing style does not "
                                      "have any configuration options."),
                                 wdg ) );
  layout->addStretch();
  return wdg;
}

void CalPrintPluginBase::doPrint( KPrinter *printer )
{
  if ( !printer ) return;
  mPrinter = printer;
  TQPainter p;

  mPrinter->setColorMode( mUseColors?(KPrinter::Color):(KPrinter::GrayScale) );

  p.begin( mPrinter );
  // TODO: Fix the margins!!!
  // the painter initially begins at 72 dpi per the Qt docs.
  // we want half-inch margins.
  int margins = margin();
  p.setViewport( margins, margins,
                 p.viewport().width() - 2*margins,
                 p.viewport().height() - 2*margins );
//   TQRect vp( p.viewport() );
// vp.setRight( vp.right()*2 );
// vp.setBottom( vp.bottom()*2 );
//   p.setWindow( vp );
  int pageWidth = p.window().width();
  int pageHeight = p.window().height();
//   int pageWidth = p.viewport().width();
//   int pageHeight = p.viewport().height();

  print( p, pageWidth, pageHeight );

  p.end();
  mPrinter = 0;
}

void CalPrintPluginBase::doLoadConfig()
{
  if ( mConfig ) {
    KConfigGroupSaver saver( mConfig, description() );
    mConfig->sync();
    TQDateTime currDate( TQDate::currentDate() );
    mFromDate = mConfig->readDateTimeEntry( "FromDate", &currDate ).date();
    mToDate = mConfig->readDateTimeEntry( "ToDate" ).date();
    mUseColors = mConfig->readBoolEntry( "UseColors", true );
    setUseColors( mUseColors );
    loadConfig();
  } else {
    kdDebug(5850) << "No config available in loadConfig!!!!" << endl;
  }
}

void CalPrintPluginBase::doSaveConfig()
{
  if ( mConfig ) {
    KConfigGroupSaver saver( mConfig, description() );
    saveConfig();
    mConfig->writeEntry( "FromDate", TQDateTime( mFromDate ) );
    mConfig->writeEntry( "ToDate", TQDateTime( mToDate ) );
    mConfig->writeEntry( "UseColors", mUseColors );
    mConfig->sync();
  } else {
    kdDebug(5850) << "No config available in saveConfig!!!!" << endl;
  }
}




void CalPrintPluginBase::setKOrgCoreHelper( KOrg::CoreHelper*helper )
{
  PrintPlugin::setKOrgCoreHelper( helper );
  if ( helper )
    setCalendarSystem( helper->calendarSystem() );
}

bool CalPrintPluginBase::useColors() const
{
  return mUseColors;
}
void CalPrintPluginBase::setUseColors( bool useColors )
{
  mUseColors = useColors;
}

KPrinter::Orientation CalPrintPluginBase::orientation() const
{
  return (mPrinter)?(mPrinter->orientation()):(KPrinter::Portrait);
}



TQTime CalPrintPluginBase::dayStart()
{
  TQTime start( 8,0,0 );
  if ( mCoreHelper ) start = mCoreHelper->dayStart();
  return start;
}

void CalPrintPluginBase::setCategoryColors( TQPainter &p, Incidence *incidence )
{
  TQColor bgColor = categoryBgColor( incidence );
  if ( bgColor.isValid() )
    p.setBrush( bgColor );
  TQColor tColor( textColor( bgColor ) );
  if ( tColor.isValid() )
    p.setPen( tColor );
}

TQColor CalPrintPluginBase::categoryBgColor( Incidence *incidence )
{
  if (mCoreHelper && incidence)
    return mCoreHelper->categoryColor( incidence->categories() );
  else
    return TQColor();
}

TQColor CalPrintPluginBase::textColor( const TQColor &color )
{
  return (mCoreHelper)?(mCoreHelper->textColor( color )):TQColor();
}

bool CalPrintPluginBase::isWorkingDay( const TQDate &dt )
{
  return (mCoreHelper)?( mCoreHelper->isWorkingDay( dt ) ):true;
}

TQString CalPrintPluginBase::holidayString( const TQDate &dt )
{
  return (mCoreHelper)?(mCoreHelper->holidayString(dt)):(TQString::null);
}


Event *CalPrintPluginBase::holiday( const TQDate &dt )
{
  TQString hstring( holidayString( dt ) );
  if ( !hstring.isEmpty() ) {
    Event*holiday=new Event();
    holiday->setSummary( hstring );
    holiday->setDtStart( dt );
    holiday->setDtEnd( dt );
    holiday->setFloats( true );
    holiday->setCategories( i18n("Holiday") );
    return holiday;
  }
  return 0;
}

const KCalendarSystem *CalPrintPluginBase::calendarSystem() const
{
  return mCalSys;
}
void CalPrintPluginBase::setCalendarSystem( const KCalendarSystem *calsys )
{
  mCalSys = calsys;
}

int CalPrintPluginBase::headerHeight() const
{
  if ( mHeaderHeight >= 0 )
    return mHeaderHeight;
  else if ( orientation() == KPrinter::Portrait )
    return PORTRAIT_HEADER_HEIGHT;
  else
    return LANDSCAPE_HEADER_HEIGHT;
}
void CalPrintPluginBase::setHeaderHeight( const int height )
{
  mHeaderHeight = height;
}

int CalPrintPluginBase::subHeaderHeight() const
{
  return mSubHeaderHeight;
}
void CalPrintPluginBase::setSubHeaderHeight( const int height )
{
  mSubHeaderHeight = height;
}

int CalPrintPluginBase::footerHeight() const
{
  if ( mFooterHeight >= 0 )
    return mFooterHeight;
  else if ( orientation() == KPrinter::Portrait )
    return PORTRAIT_FOOTER_HEIGHT;
  else
    return LANDSCAPE_FOOTER_HEIGHT;
}
void CalPrintPluginBase::setFooterHeight( const int height )
{
  mFooterHeight = height;
}

int CalPrintPluginBase::margin() const
{
  return mMargin;
}
void CalPrintPluginBase::setMargin( const int margin )
{
  mMargin = margin;
}

int CalPrintPluginBase::padding() const
{
  return mPadding;
}
void CalPrintPluginBase::setPadding( const int padding )
{
  mPadding = padding;
}

int CalPrintPluginBase::borderWidth() const
{
  return mBorder;
}
void CalPrintPluginBase::setBorderWidth( const int borderwidth )
{
  mBorder = borderwidth;
}




void CalPrintPluginBase::drawBox( TQPainter &p, int linewidth, const TQRect &rect )
{
  TQPen pen( p.pen() );
  TQPen oldpen( pen );
  pen.setWidth( linewidth );
  p.setPen( pen );
  p.drawRect( rect );
  p.setPen( oldpen );
}

void CalPrintPluginBase::drawShadedBox( TQPainter &p, int linewidth, const TQBrush &brush, const TQRect &rect )
{
  TQBrush oldbrush( p.brush() );
  p.setBrush( brush );
  drawBox( p, linewidth, rect );
  p.setBrush( oldbrush );
}

void CalPrintPluginBase::printEventString( TQPainter &p, const TQRect &box, const TQString &str, int flags )
{
  TQRect newbox( box );
  newbox.addCoords( 3, 1, -1, -1 );
  p.drawText( newbox, (flags==-1)?(Qt::AlignTop | Qt::AlignJustify | Qt::BreakAnywhere):flags, str );
}


void CalPrintPluginBase::showEventBox( TQPainter &p, int linewidth, const TQRect &box,
                                       Incidence *incidence, const TQString &str, int flags )
{
  TQPen oldpen( p.pen() );
  TQBrush oldbrush( p.brush() );
  TQColor bgColor( categoryBgColor( incidence ) );
  if ( mUseColors & bgColor.isValid() ) {
    p.setBrush( bgColor );
  } else {
    p.setBrush( TQColor( 232, 232, 232 ) );
  }
  drawBox( p, ( linewidth > 0 ) ? linewidth : EVENT_BORDER_WIDTH, box );

  if ( mUseColors && bgColor.isValid() ) {
    p.setPen( textColor( bgColor ) );
  }
  printEventString( p, box, str, flags );
  p.setPen( oldpen );
  p.setBrush( oldbrush );
}


void CalPrintPluginBase::drawSubHeaderBox(TQPainter &p, const TQString &str, const TQRect &box )
{
  drawShadedBox( p, BOX_BORDER_WIDTH, TQColor( 232, 232, 232 ), box );
  TQFont oldfont( p.font() );
  p.setFont( TQFont( "sans-serif", 10, TQFont::Bold ) );
  p.drawText( box, Qt::AlignCenter | Qt::AlignVCenter, str );
  p.setFont( oldfont );
}

void CalPrintPluginBase::drawVerticalBox( TQPainter &p, int linewidth, const TQRect &box,
                                          const TQString &str, int flags )
{
  p.save();
  p.rotate( -90 );
  TQRect rotatedBox( -box.top()-box.height(), box.left(), box.height(), box.width() );
  showEventBox( p, linewidth, rotatedBox, 0, str,
                ( flags == -1 ) ? Qt::AlignLeft | Qt::AlignVCenter | Qt::SingleLine : flags );

  p.restore();
}



///////////////////////////////////////////////////////////////////////////////
// Return value: If expand, bottom of the printed box, otherwise vertical end
// of the printed contents inside the box.

int CalPrintPluginBase::drawBoxWithCaption( TQPainter &p, const TQRect &allbox,
        const TQString &caption, const TQString &contents, bool sameLine, bool expand, const TQFont &captionFont, const TQFont &textFont )
{
  TQFont oldFont( p.font() );
//   TQFont captionFont( "sans-serif", 11, TQFont::Bold );
//   TQFont textFont( "sans-serif", 11, TQFont::Normal );
//   TQFont captionFont( "Tahoma", 11, TQFont::Bold );
//   TQFont textFont( "Tahoma", 11, TQFont::Normal );


  TQRect box( allbox );

  // Bounding rectangle for caption, single-line, clip on the right
  TQRect captionBox( box.left() + padding(), box.top() + padding(), 0, 0 );
  p.setFont( captionFont );
  captionBox = p.boundingRect( captionBox, Qt::AlignLeft | Qt::AlignTop | Qt::SingleLine, caption );
  p.setFont( oldFont );
  if ( captionBox.right() > box.right() )
    captionBox.setRight( box.right() );
  if ( expand && captionBox.bottom() + padding() > box.bottom() )
    box.setBottom( captionBox.bottom() + padding() );

  // Bounding rectangle for the contents (if any), word break, clip on the bottom
  TQRect textBox( captionBox );
  if ( !contents.isEmpty() ) {
    if ( sameLine ) {
      textBox.setLeft( captionBox.right() + padding() );
    } else {
      textBox.setTop( captionBox.bottom() + padding() );
    }
    textBox.setRight( box.right() );
    textBox.setHeight( 0 );
    p.setFont( textFont );
    textBox = p.boundingRect( textBox, Qt::WordBreak | Qt::AlignTop | Qt::AlignLeft, contents );
    p.setFont( oldFont );
    if ( textBox.bottom() + padding() > box.bottom() ) {
      if ( expand ) {
        box.setBottom( textBox.bottom() + padding() );
      } else {
        textBox.setBottom( box.bottom() );
      }
    }
  }

  drawBox( p, BOX_BORDER_WIDTH, box );
  p.setFont( captionFont );
  p.drawText( captionBox, Qt::AlignLeft | Qt::AlignTop | Qt::SingleLine, caption );
  if ( !contents.isEmpty() ) {
    p.setFont( textFont );
    p.drawText( textBox, Qt::WordBreak | Qt::AlignTop | Qt::AlignLeft, contents );
  }
  p.setFont( oldFont );

  if ( expand ) {
    return box.bottom();
  } else {
    return textBox.bottom();
  }
}


///////////////////////////////////////////////////////////////////////////////

int CalPrintPluginBase::drawHeader( TQPainter &p, TQString title,
    const TQDate &month1, const TQDate &month2, const TQRect &allbox, bool expand )
{
  // print previous month for month view, print current for to-do, day and week
  int smallMonthWidth = (allbox.width()/4) - 10;
  if (smallMonthWidth>100) smallMonthWidth=100;

  int right = allbox.right();
  if ( month1.isValid() ) right -= (20+smallMonthWidth);
  if ( month2.isValid() ) right -= (20+smallMonthWidth);
  TQRect box( allbox );
  TQRect textRect( allbox );
  textRect.addCoords( 5, 0, 0, 0 );
  textRect.setRight( right );


  TQFont oldFont( p.font() );
  TQFont newFont("sans-serif", (textRect.height()<60)?16:18, TQFont::Bold);
  if ( expand ) {
    p.setFont( newFont );
    TQRect boundingR = p.boundingRect( textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::WordBreak, title );
    p.setFont( oldFont );
    int h = boundingR.height();
    if ( h > allbox.height() ) {
      box.setHeight( h );
      textRect.setHeight( h );
    }
  }

  drawShadedBox( p, BOX_BORDER_WIDTH, TQColor( 232, 232, 232 ), box );

  TQRect monthbox( box.right()-10-smallMonthWidth, box.top(), smallMonthWidth, box.height() );
  if (month2.isValid()) {
    drawSmallMonth( p, TQDate(month2.year(), month2.month(), 1), monthbox );
    monthbox.moveBy( -20 - smallMonthWidth, 0 );
  }
  if (month1.isValid()) {
    drawSmallMonth( p, TQDate(month1.year(), month1.month(), 1), monthbox );
    monthbox.moveBy( -20 - smallMonthWidth, 0 );
  }

  // Set the margins
  p.setFont( newFont );
  p.drawText( textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::WordBreak, title );
  p.setFont( oldFont );

  return textRect.bottom();
}


int CalPrintPluginBase::drawFooter( TQPainter &p, TQRect &footbox )
{
  TQFont oldfont( p.font() );
  p.setFont( TQFont( "sans-serif", 6 ) );
  TQFontMetrics fm( p.font() );
  TQString dateStr = KGlobal::locale()->formatDateTime( TQDateTime::currentDateTime(), false );
  p.drawText( footbox, TQt::AlignCenter | TQt::AlignVCenter | TQt::SingleLine,
              i18n( "print date: formatted-datetime", "printed: %1" ).arg( dateStr ) );
  p.setFont( oldfont );

  return footbox.bottom();
}

void CalPrintPluginBase::drawSmallMonth(TQPainter &p, const TQDate &qd,
    const TQRect &box )
{

  int weekdayCol = weekdayColumn( qd.dayOfWeek() );
  int month = qd.month();
  TQDate monthDate(TQDate(qd.year(), qd.month(), 1));
  // correct begin of week
  TQDate monthDate2( monthDate.addDays( -weekdayCol ) );

  double cellWidth = double(box.width())/double(7);
  int rownr = 3 + ( qd.daysInMonth() + weekdayCol - 1 ) / 7;
  // 3 Pixel after month name, 2 after day names, 1 after the calendar
  double cellHeight = (box.height() - 5) / rownr;
  TQFont oldFont( p.font() );
  p.setFont(TQFont("sans-serif", int(cellHeight-1), TQFont::Normal));

  // draw the title
  if ( mCalSys ) {
    TQRect titleBox( box );
    titleBox.setHeight( int(cellHeight+1) );
    p.drawText( titleBox, Qt::AlignTop | Qt::AlignHCenter, mCalSys->monthName( qd ) );
  }

  // draw days of week
  TQRect wdayBox( box );
  wdayBox.setTop( int( box.top() + 3 + cellHeight ) );
  wdayBox.setHeight( int(2*cellHeight)-int(cellHeight) );

  if ( mCalSys ) {
    for (int col = 0; col < 7; ++col) {
      TQString tmpStr = mCalSys->weekDayName( monthDate2 )[0].upper();
      wdayBox.setLeft( int(box.left() + col*cellWidth) );
      wdayBox.setRight( int(box.left() + (col+1)*cellWidth) );
      p.drawText( wdayBox, Qt::AlignCenter, tmpStr );
      monthDate2 = monthDate2.addDays( 1 );
    }
  }

  // draw separator line
  int calStartY = wdayBox.bottom() + 2;
  p.drawLine( box.left(), calStartY, box.right(), calStartY );
  monthDate = monthDate.addDays( -weekdayCol );

  for ( int row = 0; row < (rownr-2); row++ ) {
    for ( int col = 0; col < 7; col++ ) {
      if ( monthDate.month() == month ) {
        TQRect dayRect( int( box.left() + col*cellWidth ), int( calStartY + row*cellHeight ), 0, 0 );
        dayRect.setRight( int( box.left() + (col+1)*cellWidth ) );
        dayRect.setBottom( int( calStartY + (row+1)*cellHeight ) );
        p.drawText( dayRect, Qt::AlignCenter, TQString::number( monthDate.day() ) );
      }
      monthDate = monthDate.addDays(1);
    }
  }
  p.setFont( oldFont );
}





///////////////////////////////////////////////////////////////////////////////

/*
 * This routine draws a header box over the main part of the calendar
 * containing the days of the week.
 */
void CalPrintPluginBase::drawDaysOfWeek(TQPainter &p,
    const TQDate &fromDate, const TQDate &toDate, const TQRect &box )
{
  double cellWidth = double(box.width()) / double(fromDate.daysTo( toDate )+1);
  TQDate cellDate( fromDate );
  TQRect dateBox( box );
  int i = 0;

  while ( cellDate <= toDate ) {
    dateBox.setLeft( box.left() + int(i*cellWidth) );
    dateBox.setRight( box.left() + int((i+1)*cellWidth) );
    drawDaysOfWeekBox(p, cellDate, dateBox );
    cellDate = cellDate.addDays(1);
    i++;
  }
}


void CalPrintPluginBase::drawDaysOfWeekBox(TQPainter &p, const TQDate &qd,
    const TQRect &box )
{
  drawSubHeaderBox( p, (mCalSys)?(mCalSys->weekDayName( qd )):(TQString::null), box );
}


void CalPrintPluginBase::drawTimeLine( TQPainter &p, const TQTime &fromTime,
                                       const TQTime &toTime, const TQRect &box )
{
  drawBox( p, BOX_BORDER_WIDTH, box );

  int totalsecs = fromTime.secsTo( toTime );
  float minlen = (float)box.height() * 60. / (float)totalsecs;
  float cellHeight = ( 60. * (float)minlen );
  float currY = box.top();
  // TODO: Don't use half of the width, but less, for the minutes!
  int xcenter = box.left() + box.width() / 2;

  TQTime curTime( fromTime );
  TQTime endTime( toTime );
  if ( fromTime.minute() > 30 ) {
    curTime = TQTime( fromTime.hour()+1, 0, 0 );
  } else if ( fromTime.minute() > 0 ) {
    curTime = TQTime( fromTime.hour(), 30, 0 );
    float yy = currY + minlen * (float)fromTime.secsTo( curTime ) / 60.;
    p.drawLine( xcenter, (int)yy, box.right(), (int)yy );
    curTime = TQTime( fromTime.hour() + 1, 0, 0 );
  }
  currY += ( float( fromTime.secsTo( curTime ) * minlen ) / 60. );

  while ( curTime < endTime ) {
    p.drawLine( box.left(), (int)currY, box.right(), (int)currY );
    int newY = (int)( currY + cellHeight / 2. );
    TQString numStr;
    if ( newY < box.bottom() ) {
      TQFont oldFont( p.font() );
      // draw the time:
      if ( !KGlobal::locale()->use12Clock() ) {
        p.drawLine( xcenter, (int)newY, box.right(), (int)newY );
        numStr.setNum( curTime.hour() );
        if  ( cellHeight > 30 ) {
          p.setFont( TQFont( "sans-serif", 14, TQFont::Bold ) );
        } else {
          p.setFont( TQFont( "sans-serif", 12, TQFont::Bold ) );
        }
        p.drawText( box.left() + 4, (int)currY + 2, box.width() / 2 - 2, (int)cellHeight,
                    Qt::AlignTop | Qt::AlignRight, numStr );
        p.setFont( TQFont ( "helvetica", 10, TQFont::Normal ) );
        p.drawText( xcenter + 4, (int)currY + 2, box.width() / 2 + 2, (int)(cellHeight / 2 ) - 3,
                    Qt::AlignTop | Qt::AlignLeft, "00" );
      } else {
        p.drawLine( box.left(), (int)newY, box.right(), (int)newY );
        TQTime time( curTime.hour(), 0 );
        numStr = KGlobal::locale()->formatTime( time );
        if ( box.width() < 60 ) {
          p.setFont( TQFont( "sans-serif", 7, TQFont::Bold ) ); // for weekprint
        } else {
          p.setFont( TQFont( "sans-serif", 12, TQFont::Bold ) ); // for dayprint
        }
        p.drawText( box.left() + 2, (int)currY + 2, box.width() - 4, (int)cellHeight / 2 - 3,
                    Qt::AlignTop|Qt::AlignLeft, numStr );
      }
      currY += cellHeight;
      p.setFont( oldFont );
    } // enough space for half-hour line and time
    if ( curTime.secsTo( endTime ) > 3600 ) {
      curTime = curTime.addSecs( 3600 );
    } else {
      curTime = endTime;
    }
  } // currTime<endTime
}

/**
  prints the all-day box for the agenda print view. if expandable is set,
  height is the cell height of a single cell, and the returned height will
  be the total height used for the all-day events. If !expandable, only one
  cell will be used, and multiple events are concatenated using ", ".
*/
int CalPrintPluginBase::drawAllDayBox(TQPainter &p, Event::List &eventList,
    const TQDate &qd, bool expandable, const TQRect &box )
{
  Event::List::Iterator it, itold;

  int offset=box.top();

  TQString multiDayStr;

  Event*hd = holiday( qd );
  if ( hd ) eventList.prepend( hd );

  it = eventList.begin();
  Event *currEvent = 0;
  // First, print all floating events
  while( it!=eventList.end() ) {
    currEvent=*it;
    itold=it;
    ++it;
    if ( currEvent && currEvent->doesFloat() ) {
      // set the colors according to the categories
      if ( expandable ) {
        TQRect eventBox( box );
        eventBox.setTop( offset );
        showEventBox( p, EVENT_BORDER_WIDTH, eventBox, currEvent, currEvent->summary() );
        offset += box.height();
      } else {
        if ( !multiDayStr.isEmpty() ) multiDayStr += ", ";
        multiDayStr += currEvent->summary();
      }
      eventList.remove( itold );
    }
  }
  if ( hd ) delete hd;

  int ret = box.height();
  TQRect eventBox( box );
  if (!expandable) {
    if (!multiDayStr.isEmpty()) {
      drawShadedBox( p, BOX_BORDER_WIDTH, TQColor( 128, 128, 128 ), eventBox );
      printEventString( p, eventBox, multiDayStr );
    } else {
      drawBox( p, BOX_BORDER_WIDTH, eventBox );
    }
  } else {
    ret = offset - box.top();
    eventBox.setBottom( ret );
    drawBox( p, BOX_BORDER_WIDTH, eventBox );
  }
  return ret;
}


void CalPrintPluginBase::drawAgendaDayBox( TQPainter &p, Event::List &events,
                                     const TQDate &qd, bool expandable,
                                     TQTime &fromTime, TQTime &toTime,
                                     const TQRect &oldbox )
{
  if ( !isWorkingDay( qd ) ) {
    drawShadedBox( p, BOX_BORDER_WIDTH, TQColor( 232, 232, 232 ), oldbox );
  } else {
    drawBox( p, BOX_BORDER_WIDTH, oldbox );
  }
  TQRect box( oldbox );
  // Account for the border with and cut away that margin from the interior
//   box.setRight( box.right()-BOX_BORDER_WIDTH );

  Event *event;

  if ( expandable ) {
    // Adapt start/end times to include complete events
    Event::List::ConstIterator it;
    for ( it = events.begin(); it != events.end(); ++it ) {
      event = *it;
      if ( event->dtStart().time() < fromTime )
        fromTime = event->dtStart().time();
      if ( event->dtEnd().time() > toTime )
        toTime = event->dtEnd().time();
    }
  }

  // Show at least one hour
//   if ( fromTime.secsTo( toTime ) < 3600 ) {
//     fromTime = TQTime( fromTime.hour(), 0, 0 );
//     toTime = fromTime.addSecs( 3600 );
//   }

  // calculate the height of a cell and of a minute
  int totalsecs = fromTime.secsTo( toTime );
  float minlen = box.height() * 60. / totalsecs;
  float cellHeight = 60. * minlen;
  float currY = box.top();

  // print grid:
  TQTime curTime( TQTime( fromTime.hour(), 0, 0 ) );
  currY += fromTime.secsTo( curTime ) * minlen / 60;

  while ( curTime < toTime && curTime.isValid() ) {
    if ( currY > box.top() )
      p.drawLine( box.left(), int( currY ), box.right(), int( currY ) );
    currY += cellHeight / 2;
    if ( ( currY > box.top() ) && ( currY < box.bottom() ) ) {
      // enough space for half-hour line
      TQPen oldPen( p.pen() );
      p.setPen( TQColor( 192, 192, 192 ) );
      p.drawLine( box.left(), int( currY ), box.right(), int( currY ) );
      p.setPen( oldPen );
    }
    if ( curTime.secsTo( toTime ) > 3600 )
      curTime = curTime.addSecs( 3600 );
    else curTime = toTime;
    currY += cellHeight / 2;
  }

  TQDateTime startPrintDate = TQDateTime( qd, fromTime );
  TQDateTime endPrintDate = TQDateTime( qd, toTime );

  // Calculate horizontal positions and widths of events taking into account
  // overlapping events

  TQPtrList<KOrg::CellItem> cells;
  cells.setAutoDelete( true );

  Event::List::ConstIterator itEvents;
  for( itEvents = events.begin(); itEvents != events.end(); ++itEvents ) {
    TQValueList<TQDateTime> times = (*itEvents)->startDateTimesForDate( qd );
    for ( TQValueList<TQDateTime>::ConstIterator it = times.begin();
          it != times.end(); ++it ) {
      cells.append( new PrintCellItem( *itEvents, (*it), (*itEvents)->endDateForStart( *it ) ) );
    }
  }

  TQPtrListIterator<KOrg::CellItem> it1( cells );
  for( it1.toFirst(); it1.current(); ++it1 ) {
    KOrg::CellItem *placeItem = it1.current();
    KOrg::CellItem::placeItem( cells, placeItem );
  }

//   p.setFont( TQFont( "sans-serif", 10 ) );

  for( it1.toFirst(); it1.current(); ++it1 ) {
    PrintCellItem *placeItem = static_cast<PrintCellItem *>( it1.current() );
    drawAgendaItem( placeItem, p, startPrintDate, endPrintDate, minlen, box );
  }
//   p.setFont( oldFont );
}



void CalPrintPluginBase::drawAgendaItem( PrintCellItem *item, TQPainter &p,
                                   const TQDateTime &startPrintDate,
                                   const TQDateTime &endPrintDate,
                                   float minlen, const TQRect &box )
{
  Event *event = item->event();

  // start/end of print area for event
  TQDateTime startTime = item->start();
  TQDateTime endTime = item->end();
  if ( ( startTime < endPrintDate && endTime > startPrintDate ) ||
       ( endTime > startPrintDate && startTime < endPrintDate ) ) {
    if ( startTime < startPrintDate ) startTime = startPrintDate;
    if ( endTime > endPrintDate ) endTime = endPrintDate;
    int currentWidth = box.width() / item->subCells();
    int currentX = box.left() + item->subCell() * currentWidth;
    int currentYPos = int( box.top() + startPrintDate.secsTo( startTime ) *
                           minlen / 60. );
    int currentHeight = int( box.top() + startPrintDate.secsTo( endTime ) * minlen / 60. ) - currentYPos;

    TQRect eventBox( currentX, currentYPos, currentWidth, currentHeight );
    TQString str;
    if ( event->location().isEmpty() ) {
      str = i18n( "starttime - endtime summary",
                  "%1-%2 %3" ).
            arg( KGlobal::locale()->formatTime( startTime.time() ) ).
            arg( KGlobal::locale()->formatTime( endTime.time() ) ).
            arg( cleanStr( event->summary() ) );
    } else {
      str = i18n( "starttime - endtime summary, location",
                  "%1-%2 %3, %4" ).
            arg( KGlobal::locale()->formatTime( startTime.time() ) ).
            arg( KGlobal::locale()->formatTime( endTime.time() ) ).
            arg( cleanStr( event->summary() ) ).
            arg( cleanStr( event->location() ) );
    }
    showEventBox( p, EVENT_BORDER_WIDTH, eventBox, event, str );
  }
}

//TODO TODO TODO
void CalPrintPluginBase::drawDayBox( TQPainter &p, const TQDate &qd,
    const TQRect &box,
    bool fullDate, bool printRecurDaily, bool printRecurWeekly )
{
  TQString dayNumStr;
  const KLocale*local = KGlobal::locale();

  // This has to be localized
  if ( fullDate && mCalSys ) {

    dayNumStr = i18n("weekday month date", "%1 %2 %3")
        .arg( mCalSys->weekDayName( qd ) )
        .arg( mCalSys->monthName( qd ) )
        .arg( qd.day() );
//    dayNumStr = local->formatDate(qd);
  } else {
    dayNumStr = TQString::number( qd.day() );
  }

  TQRect subHeaderBox( box );
  subHeaderBox.setHeight( mSubHeaderHeight );
  drawShadedBox( p, BOX_BORDER_WIDTH, p.backgroundColor(), box );
  drawShadedBox( p, 0, TQColor( 232, 232, 232 ), subHeaderBox );
  drawBox( p, BOX_BORDER_WIDTH, box );
  TQString hstring( holidayString( qd ) );
  TQFont oldFont( p.font() );

  TQRect headerTextBox( subHeaderBox );
  headerTextBox.setLeft( subHeaderBox.left()+5 );
  headerTextBox.setRight( subHeaderBox.right()-5 );
  if (!hstring.isEmpty()) {
    p.setFont( TQFont( "sans-serif", 8, TQFont::Bold, true ) );

    p.drawText( headerTextBox, Qt::AlignLeft | Qt::AlignVCenter, hstring );
  }
  p.setFont(TQFont("sans-serif", 10, TQFont::Bold));
  p.drawText( headerTextBox, Qt::AlignRight | Qt::AlignVCenter, dayNumStr);

  Event::List eventList = mCalendar->events( qd,
                                             EventSortStartDate,
                                             SortDirectionAscending );
  TQString timeText;
  p.setFont( TQFont( "sans-serif", 8 ) );

  int textY=mSubHeaderHeight+3; // gives the relative y-coord of the next printed entry
  Event::List::ConstIterator it;

  for( it = eventList.begin(); it != eventList.end() && textY<box.height(); ++it ) {
    Event *currEvent = *it;
    if ( ( !printRecurDaily  && currEvent->recurrenceType() == Recurrence::rDaily  ) ||
         ( !printRecurWeekly && currEvent->recurrenceType() == Recurrence::rWeekly ) ) {
      continue;
    }
    if ( currEvent->doesFloat() || currEvent->isMultiDay() ) {
      timeText = "";
    } else {
      timeText = local->formatTime( currEvent->dtStart().time() );
    }

    TQString str;
    if ( !currEvent->location().isEmpty() ) {
      str = i18n( "summary, location", "%1, %2" ).
            arg( currEvent->summary() ).arg( currEvent->location() );
    } else {
      str = currEvent->summary();
    }
    drawIncidence( p, box, timeText, str, textY );
  }

  if ( textY < box.height() ) {
    Todo::List todos = mCalendar->todos( qd );
    Todo::List::ConstIterator it2;
    for ( it2 = todos.begin(); it2 != todos.end() && textY <box.height(); ++it2 ) {
      Todo *todo = *it2;
      if ( ( !printRecurDaily  && todo->recurrenceType() == Recurrence::rDaily  ) ||
           ( !printRecurWeekly && todo->recurrenceType() == Recurrence::rWeekly ) ) {
        continue;
      }
      if ( todo->hasStartDate() && !todo->doesFloat() ) {
        timeText = KGlobal::locale()->formatTime( todo->dtStart().time() ) + " ";
      } else {
        timeText = "";
      }
      TQString summaryStr;
      if ( !todo->location().isEmpty() ) {
        summaryStr = i18n( "summary, location", "%1, %2" ).
                     arg( todo->summary() ).arg( todo->location() );
      } else {
        summaryStr = todo->summary();
      }
      TQString str;
      if ( todo->hasDueDate() ) {
        if ( !todo->doesFloat() ) {
          str = i18n( "%1 (Due: %2)" ).
                arg( summaryStr ).
                arg( KGlobal::locale()->formatDateTime( todo->dtDue() ) );
        } else {
          str = i18n( "%1 (Due: %2)" ).
                arg( summaryStr ).
                arg( KGlobal::locale()->formatDate( todo->dtDue().date(), true ) );
        }
      } else {
        str = summaryStr;
      }
      drawIncidence( p, box, timeText, i18n("To-do: %1").arg( str ), textY );
    }
  }

  p.setFont( oldFont );
}

// TODO TODO TODO
void CalPrintPluginBase::drawIncidence( TQPainter &p, const TQRect &dayBox, const TQString &time, const TQString &summary, int &textY )
{
  kdDebug(5850) << "summary = " << summary << endl;

  int flags = Qt::AlignLeft;
  TQFontMetrics fm = p.fontMetrics();
  TQRect timeBound = p.boundingRect( dayBox.x() + 5, dayBox.y() + textY,
                                    dayBox.width() - 10, fm.lineSpacing(),
                                    flags, time );
  p.drawText( timeBound, flags, time );

  int summaryWidth = time.isEmpty() ? 0 : timeBound.width() + 4;
  TQRect summaryBound = TQRect( dayBox.x() + 5 + summaryWidth, dayBox.y() + textY,
                              dayBox.width() - summaryWidth -5, dayBox.height() );

  KWordWrap *ww = KWordWrap::formatText( fm, summaryBound, flags, summary );
  ww->drawText( &p, dayBox.x() + 5 + summaryWidth, dayBox.y() + textY, flags );

  textY += ww->boundingRect().height();

  delete ww;
}


///////////////////////////////////////////////////////////////////////////////

void CalPrintPluginBase::drawWeek(TQPainter &p, const TQDate &qd, const TQRect &box )
{
  TQDate weekDate = qd;
  bool portrait = ( box.height() > box.width() );
  int cellWidth, cellHeight;
  int vcells;
  if (portrait) {
    cellWidth = box.width()/2;
    vcells=3;
  } else {
    cellWidth = box.width()/6;
    vcells=1;
  }
  cellHeight = box.height()/vcells;

  // correct begin of week
  int weekdayCol = weekdayColumn( qd.dayOfWeek() );
  weekDate = qd.addDays( -weekdayCol );

  for (int i = 0; i < 7; i++, weekDate = weekDate.addDays(1)) {
    // Saturday and sunday share a cell, so we have to special-case sunday
    int hpos = ((i<6)?i:(i-1)) / vcells;
    int vpos = ((i<6)?i:(i-1)) % vcells;
    TQRect dayBox( box.left()+cellWidth*hpos, box.top()+cellHeight*vpos + ((i==6)?(cellHeight/2):0),
        cellWidth, (i<5)?(cellHeight):(cellHeight/2) );
    drawDayBox(p, weekDate, dayBox, true);
  } // for i through all weekdays
}


void CalPrintPluginBase::drawTimeTable(TQPainter &p,
    const TQDate &fromDate, const TQDate &toDate,
    TQTime &fromTime, TQTime &toTime,
    const TQRect &box)
{
  // timeline is 1 hour:
  int alldayHeight = (int)( 3600.*box.height()/(fromTime.secsTo(toTime)+3600.) );
  int timelineWidth = TIMELINE_WIDTH;

  TQRect dowBox( box );
  dowBox.setLeft( box.left() + timelineWidth );
  dowBox.setHeight( mSubHeaderHeight );
  drawDaysOfWeek( p, fromDate, toDate, dowBox );

  TQRect tlBox( box );
  tlBox.setWidth( timelineWidth );
  tlBox.setTop( dowBox.bottom() + BOX_BORDER_WIDTH + alldayHeight );
  drawTimeLine( p, fromTime, toTime, tlBox );

  // draw each day
  TQDate curDate(fromDate);
  int i=0;
  double cellWidth = double(dowBox.width()) / double(fromDate.daysTo(toDate)+1);
  while (curDate<=toDate) {
    TQRect allDayBox( dowBox.left()+int(i*cellWidth), dowBox.bottom() + BOX_BORDER_WIDTH,
                     int((i+1)*cellWidth)-int(i*cellWidth), alldayHeight );
    TQRect dayBox( allDayBox );
    dayBox.setTop( tlBox.top() );
    dayBox.setBottom( box.bottom() );
    Event::List eventList = mCalendar->events(curDate,
                                              EventSortStartDate,
                                              SortDirectionAscending);
    alldayHeight = drawAllDayBox( p, eventList, curDate, false, allDayBox );
    drawAgendaDayBox( p, eventList, curDate, false, fromTime, toTime, dayBox );
    i++;
    curDate=curDate.addDays(1);
  }

}


///////////////////////////////////////////////////////////////////////////////

class MonthEventStruct
{
  public:
    MonthEventStruct() : event(0) {}
    MonthEventStruct( const TQDateTime &s, const TQDateTime &e, Event *ev)
    {
      event = ev;
      start = s;
      end = e;
      if ( event->doesFloat() ) {
        start = TQDateTime( start.date(), TQTime(0,0,0) );
        end = TQDateTime( end.date().addDays(1), TQTime(0,0,0) ).addSecs(-1);
      }
    }
    bool operator<(const MonthEventStruct &mes) { return start < mes.start; }
    TQDateTime start;
    TQDateTime end;
    Event *event;
};

void CalPrintPluginBase::drawMonth( TQPainter &p, const TQDate &dt, const TQRect &box, int maxdays, int subDailyFlags, int holidaysFlags )
{
  const KCalendarSystem *calsys = calendarSystem();
  TQRect subheaderBox( box );
  subheaderBox.setHeight( subHeaderHeight() );
  TQRect borderBox( box );
  borderBox.setTop( subheaderBox.bottom()+1 );
  drawSubHeaderBox( p, calsys->monthName(dt), subheaderBox );
  // correct for half the border width
  int correction = (BOX_BORDER_WIDTH/*-1*/)/2;
  TQRect daysBox( borderBox );
  daysBox.addCoords( correction, correction, -correction, -correction );

  int daysinmonth = calsys->daysInMonth( dt );
  if ( maxdays <= 0 ) maxdays = daysinmonth;

  int d;
  float dayheight = float(daysBox.height()) / float( maxdays );

  TQColor holidayColor( 240, 240, 240 );
  TQColor workdayColor( 255, 255, 255 );
  int dayNrWidth = p.fontMetrics().width( "99" );

  // Fill the remaining space (if a month has less days than others) with a crossed-out pattern
  if ( daysinmonth<maxdays ) {
    TQRect dayBox( box.left(), daysBox.top() + round(dayheight*daysinmonth), box.width(), 0 );
    dayBox.setBottom( daysBox.bottom() );
    p.fillRect( dayBox, Qt::DiagCrossPattern );
  }
  // Backgrounded boxes for each day, plus day numbers
  TQBrush oldbrush( p.brush() );
  for ( d = 0; d < daysinmonth; ++d ) {
    TQDate day;
    calsys->setYMD( day, dt.year(), dt.month(), d+1 );
    TQRect dayBox( daysBox.left()/*+rand()%50*/, daysBox.top() + round(dayheight*d), daysBox.width()/*-rand()%50*/, 0 );
    // FIXME: When using a border width of 0 for event boxes, don't let the rectangles overlap, i.e. subtract 1 from the top or bottom!
    dayBox.setBottom( daysBox.top()+round(dayheight*(d+1)) - 1 );

    p.setBrush( isWorkingDay( day )?workdayColor:holidayColor );
    p.drawRect( dayBox );
    TQRect dateBox( dayBox );
    dateBox.setWidth( dayNrWidth+3 );
    p.drawText( dateBox, Qt::AlignRight | Qt::AlignVCenter | Qt::SingleLine,
                TQString::number(d+1) );
  }
  p.setBrush( oldbrush );
  int xstartcont = box.left() + dayNrWidth + 5;

  TQDate start, end;
  calsys->setYMD( start, dt.year(), dt.month(), 1 );
  end = calsys->addMonths( start, 1 );
  end = calsys->addDays( end, -1 );

  Event::List events = mCalendar->events( start, end );
  TQMap<int, TQStringList> textEvents;
  TQPtrList<KOrg::CellItem> timeboxItems;
  timeboxItems.setAutoDelete( true );


  // 1) For multi-day events, show boxes spanning several cells, use CellItem
  //    print the summary vertically
  // 2) For sub-day events, print the concated summaries into the remaining
  //    space of the box (optional, depending on the given flags)
  // 3) Draw some kind of timeline showing free and busy times

  // Holidays
  Event::List holidays;
  holidays.setAutoDelete( true );
  for ( TQDate d(start); d <= end; d = d.addDays(1) ) {
    Event *e = holiday( d );
    if ( e ) {
      holidays.append( e );
      if ( holidaysFlags & TimeBoxes ) {
        timeboxItems.append( new PrintCellItem( e, TQDateTime(d, TQTime(0,0,0) ),
            TQDateTime( d.addDays(1), TQTime(0,0,0) ) ) );
      }
      if ( holidaysFlags & Text ) {
        textEvents[ d.day() ] << e->summary();
      }
    }
  }

  TQValueList<MonthEventStruct> monthentries;

  for ( Event::List::ConstIterator evit = events.begin();
        evit != events.end(); ++evit ) {
    Event *e = (*evit);
    if (!e) continue;
    if ( e->doesRecur() ) {
      if ( e->recursOn( start ) ) {
        // This occurrence has possibly started before the beginning of the
        // month, so obtain the start date before the beginning of the month
        TQValueList<TQDateTime> starttimes = e->startDateTimesForDate( start );
        TQValueList<TQDateTime>::ConstIterator it = starttimes.begin();
        for ( ; it != starttimes.end(); ++it ) {
          monthentries.append( MonthEventStruct( *it, e->endDateForStart( *it ), e ) );
        }
      }
      // Loop through all remaining days of the month and check if the event
      // begins on that day (don't use Event::recursOn, as that will
      // also return events that have started earlier. These start dates
      // however, have already been treated!
      Recurrence *recur = e->recurrence();
      TQDate d1( start.addDays(1) );
      while ( d1 <= end ) {
        if ( recur->recursOn(d1) ) {
          TimeList times( recur->recurTimesOn( d1 ) );
          for ( TimeList::ConstIterator it = times.begin();
                it != times.end(); ++it ) {
            TQDateTime d1start( d1, *it );
            monthentries.append( MonthEventStruct( d1start, e->endDateForStart( d1start ), e ) );
          }
        }
        d1 = d1.addDays(1);
      }
    } else {
      monthentries.append( MonthEventStruct( e->dtStart(), e->dtEnd(), e ) );
    }
  }
  qHeapSort( monthentries );

  TQValueList<MonthEventStruct>::ConstIterator mit = monthentries.begin();
  TQDateTime endofmonth( end, TQTime(0,0,0) );
  endofmonth = endofmonth.addDays(1);
  for ( ; mit != monthentries.end(); ++mit ) {
    if ( (*mit).start.date() == (*mit).end.date() ) {
      // Show also single-day events as time line boxes
      if ( subDailyFlags & TimeBoxes ) {
        timeboxItems.append( new PrintCellItem( (*mit).event, (*mit).start, (*mit).end ) );
      }
      // Show as text in the box
      if ( subDailyFlags & Text ) {
        textEvents[ (*mit).start.date().day() ] << (*mit).event->summary();
      }
    } else {
      // Multi-day events are always shown as time line boxes
      TQDateTime thisstart( (*mit).start );
      TQDateTime thisend( (*mit).end );
      if ( thisstart.date()<start ) thisstart = start;
      if ( thisend>endofmonth ) thisend = endofmonth;
      timeboxItems.append( new PrintCellItem( (*mit).event, thisstart, thisend ) );
    }
  }

  // For Multi-day events, line them up nicely so that the boxes don't overlap
  TQPtrListIterator<KOrg::CellItem> it1( timeboxItems );
  for( it1.toFirst(); it1.current(); ++it1 ) {
    KOrg::CellItem *placeItem = it1.current();
    KOrg::CellItem::placeItem( timeboxItems, placeItem );
  }
  TQDateTime starttime( start, TQTime( 0, 0, 0 ) );
  int newxstartcont = xstartcont;

  TQFont oldfont( p.font() );
  p.setFont( TQFont( "sans-serif", 7 ) );
  for( it1.toFirst(); it1.current(); ++it1 ) {
    PrintCellItem *placeItem = static_cast<PrintCellItem *>( it1.current() );
    int minsToStart = starttime.secsTo( placeItem->start() )/60;
    int minsToEnd = starttime.secsTo( placeItem->end() )/60;

    TQRect eventBox( xstartcont + placeItem->subCell()*17,
           daysBox.top() + round( double( minsToStart*daysBox.height()) / double(maxdays*24*60) ),
           14, 0 );
    eventBox.setBottom( daysBox.top() + round( double( minsToEnd*daysBox.height()) / double(maxdays*24*60) ) );
    drawVerticalBox( p, 0, eventBox, placeItem->event()->summary() );
    newxstartcont = QMAX( newxstartcont, eventBox.right() );
  }
  xstartcont = newxstartcont;

  // For Single-day events, simply print their summaries into the remaining
  // space of the day's cell
  for ( int d=0; d<daysinmonth; ++d ) {
    TQStringList dayEvents( textEvents[d+1] );
    TQString txt = dayEvents.join(", ");
    TQRect dayBox( xstartcont, daysBox.top()+round(dayheight*d), 0, 0 );
    dayBox.setRight( box.right() );
    dayBox.setBottom( daysBox.top()+round(dayheight*(d+1)) );
    printEventString(p, dayBox, txt, Qt::AlignTop | Qt::AlignLeft | Qt::BreakAnywhere );
  }
  p.setFont( oldfont );
//   p.setBrush( Qt::NoBrush );
  drawBox( p, BOX_BORDER_WIDTH, borderBox );
  p.restore();
}

///////////////////////////////////////////////////////////////////////////////

void CalPrintPluginBase::drawMonthTable(TQPainter &p, const TQDate &qd, bool weeknumbers,
                               bool recurDaily, bool recurWeekly,
                               const TQRect &box)
{
  int yoffset = mSubHeaderHeight;
  int xoffset = 0;
  TQDate monthDate(TQDate(qd.year(), qd.month(), 1));
  TQDate monthFirst(monthDate);
  TQDate monthLast(monthDate.addMonths(1).addDays(-1));


  int weekdayCol = weekdayColumn( monthDate.dayOfWeek() );
  monthDate = monthDate.addDays(-weekdayCol);

  if (weeknumbers) {
    xoffset += 14;
  }

  int rows=(weekdayCol + qd.daysInMonth() - 1)/7 +1;
  double cellHeight = ( box.height() - yoffset ) / (1.*rows);
  double cellWidth = ( box.width() - xoffset ) / 7.;

  // Precalculate the grid...
  // rows is at most 6, so using 8 entries in the array is fine, too!
  int coledges[8], rowedges[8];
  for ( int i = 0; i <= 7; i++ ) {
    rowedges[i] = int( box.top() + yoffset + i*cellHeight );
    coledges[i] = int( box.left() + xoffset + i*cellWidth );
  }

  if (weeknumbers) {
    TQFont oldFont(p.font());
    TQFont newFont(p.font());
    newFont.setPointSize(6);
    p.setFont(newFont);
    TQDate weekDate(monthDate);
    for (int row = 0; row<rows; ++row ) {
      int calWeek = weekDate.weekNumber();
      TQRect rc( box.left(), rowedges[row], coledges[0] - 3 - box.left(), rowedges[row+1]-rowedges[row] );
      p.drawText( rc, Qt::AlignRight | Qt::AlignVCenter, TQString::number( calWeek ) );
      weekDate = weekDate.addDays( 7 );
    }
    p.setFont( oldFont );
  }

  TQRect daysOfWeekBox( box );
  daysOfWeekBox.setHeight( mSubHeaderHeight );
  daysOfWeekBox.setLeft( box.left()+xoffset );
  drawDaysOfWeek( p, monthDate, monthDate.addDays( 6 ), daysOfWeekBox );

  TQColor back = p.backgroundColor();
  bool darkbg = false;
  for ( int row = 0; row < rows; ++row ) {
    for ( int col = 0; col < 7; ++col ) {
      // show days from previous/next month with a grayed background
      if ( (monthDate < monthFirst) || (monthDate > monthLast) ) {
        p.setBackgroundColor( back.dark( 120 ) );
        darkbg = true;
      }
      TQRect dayBox( coledges[col], rowedges[row], coledges[col+1]-coledges[col], rowedges[row+1]-rowedges[row] );
      drawDayBox(p, monthDate, dayBox, false, recurDaily, recurWeekly );
      if ( darkbg ) {
        p.setBackgroundColor( back );
        darkbg = false;
      }
      monthDate = monthDate.addDays(1);
    }
  }
}


///////////////////////////////////////////////////////////////////////////////

void CalPrintPluginBase::drawTodo( int &count, Todo *todo, TQPainter &p,
                               TodoSortField sortField, SortDirection sortDir,
                               bool connectSubTodos, bool strikeoutCompleted,
                               bool desc, int posPriority, int posSummary,
                               int posDueDt, int posPercentComplete,
                               int level, int x, int &y, int width,
                               int pageHeight, const Todo::List &todoList,
                               TodoParentStart *r )
{
  TQString outStr;
  const KLocale *local = KGlobal::locale();
  TQRect rect;
  TodoParentStart startpt;

  // This list keeps all starting points of the parent to-dos so the connection
  // lines of the tree can easily be drawn (needed if a new page is started)
  static TQPtrList<TodoParentStart> startPoints;
  if ( level < 1 ) {
    startPoints.clear();
  }

  // Compute the right hand side of the to-do box
  int rhs = posPercentComplete;
  if ( rhs < 0 ) rhs = posDueDt; //not printing percent completed
  if ( rhs < 0 ) rhs = x+width;  //not printing due dates either

  // size of to-do
  outStr=todo->summary();
  int left = posSummary + ( level*10 );
  rect = p.boundingRect( left, y, ( rhs-left-5 ), -1, Qt::WordBreak, outStr );
  if ( !todo->description().isEmpty() && desc ) {
    outStr = todo->description();
    rect = p.boundingRect( left+20, rect.bottom()+5, width-(left+10-x), -1,
                           Qt::WordBreak, outStr );
  }
  // if too big make new page
  if ( rect.bottom() > pageHeight ) {
    // first draw the connection lines from parent to-dos:
    if ( level > 0 && connectSubTodos ) {
      TodoParentStart *rct;
      for ( rct = startPoints.first(); rct; rct = startPoints.next() ) {
        int start;
        int center = rct->mRect.left() + (rct->mRect.width()/2);
        int to = p.viewport().bottom();

        // draw either from start point of parent or from top of the page
        if ( rct->mSamePage )
          start = rct->mRect.bottom() + 1;
        else
          start = p.viewport().top();
        p.moveTo( center, start );
        p.lineTo( center, to );
        rct->mSamePage = false;
      }
    }
    y=0;
    mPrinter->newPage();
  }

  // If this is a sub-to-do, r will not be 0, and we want the LH side
  // of the priority line up to the RH side of the parent to-do's priority
  bool showPriority = posPriority>=0;
  int lhs = posPriority;
  if ( r ) {
    lhs = r->mRect.right() + 1;
  }

  outStr.setNum( todo->priority() );
  rect = p.boundingRect( lhs, y + 10, 5, -1, Qt::AlignCenter, outStr );
  // Make it a more reasonable size
  rect.setWidth(18);
  rect.setHeight(18);

  // Draw a checkbox
  p.setBrush( TQBrush( Qt::NoBrush ) );
  p.drawRect( rect );
  if ( todo->isCompleted() ) {
    // cross out the rectangle for completed to-dos
    p.drawLine( rect.topLeft(), rect.bottomRight() );
    p.drawLine( rect.topRight(), rect.bottomLeft() );
  }
  lhs = rect.right() + 3;

  // Priority
  if ( todo->priority() > 0 && showPriority ) {
    p.drawText( rect, Qt::AlignCenter, outStr );
  }
  startpt.mRect = rect; //save for later

  // Connect the dots
  if ( level > 0 && connectSubTodos ) {
    int bottom;
    int center( r->mRect.left() + (r->mRect.width()/2) );
    if ( r->mSamePage )
      bottom = r->mRect.bottom() + 1;
    else
      bottom = 0;
    int to( rect.top() + (rect.height()/2) );
    int endx( rect.left() );
    p.moveTo( center, bottom );
    p.lineTo( center, to );
    p.lineTo( endx, to );
  }

  // summary
  outStr=todo->summary();
  rect = p.boundingRect( lhs, rect.top(), (rhs-(left + rect.width() + 5)),
                         -1, Qt::WordBreak, outStr );

  TQRect newrect;
  //FIXME: the following code prints underline rather than strikeout text
#if 0
  TQFont f( p.font() );
  if ( todo->isCompleted() && strikeoutCompleted ) {
    f.setStrikeOut( true );
    p.setFont( f );
  }
  p.drawText( rect, Qt::WordBreak, outStr, -1, &newrect );
  f.setStrikeOut( false );
  p.setFont( f );
#endif
  //TODO: Remove this section when the code above is fixed
  p.drawText( rect, Qt::WordBreak, outStr, -1, &newrect );
  if ( todo->isCompleted() && strikeoutCompleted ) {
    // strike out the summary text if to-do is complete
    // Note: we tried to use a strike-out font and for unknown reasons the
    // result was underline instead of strike-out, so draw the lines ourselves.
    int delta = p.fontMetrics().lineSpacing();
    int lines = ( rect.height() / delta ) + 1;
    for ( int i=0; i<lines; i++ ) {
      p.moveTo( rect.left(),  rect.top() + ( delta/2 ) + ( i*delta ) );
      p.lineTo( rect.right(), rect.top() + ( delta/2 ) + ( i*delta ) );
    }
  }

  // due date
  if ( todo->hasDueDate() && posDueDt>=0 ) {
    outStr = local->formatDate( todo->dtDue().date(), true );
    rect = p.boundingRect( posDueDt, y, x + width, -1,
                           Qt::AlignTop | Qt::AlignLeft, outStr );
    p.drawText( rect, Qt::AlignTop | Qt::AlignLeft, outStr );
  }

  // percentage completed
  bool showPercentComplete = posPercentComplete>=0;
  if ( showPercentComplete ) {
    int lwidth = 24;
    int lheight = 12;
    //first, draw the progress bar
    int progress = (int)(( lwidth*todo->percentComplete())/100.0 + 0.5);

    p.setBrush( TQBrush( Qt::NoBrush ) );
    p.drawRect( posPercentComplete, y+3, lwidth, lheight );
    if ( progress > 0 ) {
      p.setBrush( TQColor( 128, 128, 128 ) );
      p.drawRect( posPercentComplete, y+3, progress, lheight );
    }

    //now, write the percentage
    outStr = i18n( "%1%" ).arg( todo->percentComplete() );
    rect = p.boundingRect( posPercentComplete+lwidth+3, y, x + width, -1,
                           Qt::AlignTop | Qt::AlignLeft, outStr );
    p.drawText( rect, Qt::AlignTop | Qt::AlignLeft, outStr );
  }

  // description
  if ( !todo->description().isEmpty() && desc ) {
    y = newrect.bottom() + 5;
    outStr = todo->description();
    rect = p.boundingRect( left+20, y, x+width-(left+10), -1,
                           Qt::WordBreak, outStr );
    p.drawText( rect, Qt::WordBreak, outStr, -1, &newrect );
  }

  // Set the new line position
  y = newrect.bottom() + 10; //set the line position

  // If the to-do has sub-to-dos, we need to call ourselves recursively
#if 0
  Incidence::List l = todo->relations();
  Incidence::List::ConstIterator it;
  startPoints.append( &startpt );
  for( it = l.begin(); it != l.end(); ++it ) {
    count++;
    // In the future, to-dos might also be related to events
    // Manually check if the sub-to-do is in the list of to-dos to print
    // The problem is that relations() does not apply filters, so
    // we need to compare manually with the complete filtered list!
    Todo* subtodo = dynamic_cast<Todo *>( *it );
    if (subtodo && todoList.contains( subtodo ) ) {
      drawTodo( count, subtodo, p, connectSubTodos, strikeoutCompleted,
                desc, posPriority, posSummary, posDueDt, posPercentComplete,
                level+1, x, y, width, pageHeight, todoList, &startpt );
    }
  }
#endif
  // Make a list of all the sub-to-dos related to this to-do.
  Todo::List t;
  Incidence::List l = todo->relations();
  Incidence::List::ConstIterator it;
  for( it=l.begin(); it!=l.end(); ++it ) {
    // In the future, to-dos might also be related to events
    // Manually check if the sub-to-do is in the list of to-dos to print
    // The problem is that relations() does not apply filters, so
    // we need to compare manually with the complete filtered list!
    Todo* subtodo = dynamic_cast<Todo *>( *it );
    if ( subtodo && todoList.contains( subtodo ) ) {
      t.append( subtodo );
    }
  }

  // Sort the sub-to-dos and then print them
  Todo::List sl = mCalendar->sortTodos( &t, sortField, sortDir );
  Todo::List::ConstIterator isl;
  startPoints.append( &startpt );
  for( isl = sl.begin(); isl != sl.end(); ++isl ) {
    count++;
    drawTodo( count, ( *isl ), p, sortField, sortDir,
              connectSubTodos, strikeoutCompleted,
              desc, posPriority, posSummary, posDueDt, posPercentComplete,
              level+1, x, y, width, pageHeight, todoList, &startpt );
  }
  startPoints.remove( &startpt );
}

int CalPrintPluginBase::weekdayColumn( int weekday )
{
  return ( weekday + 7 - KGlobal::locale()->weekStartDay() ) % 7;
}

void CalPrintPluginBase::drawJournalField( TQPainter &p, TQString field, TQString text,
                                       int x, int &y, int width, int pageHeight )
{
  if ( text.isEmpty() ) return;

  TQString entry( field.arg( text ) );

  TQRect rect( p.boundingRect( x, y, width, -1, Qt::WordBreak, entry) );
  if ( rect.bottom() > pageHeight) {
    // Start new page...
    // FIXME: If it's a multi-line text, draw a few lines on this page, and the
    // remaining lines on the next page.
    y=0;
    mPrinter->newPage();
    rect = p.boundingRect( x, y, width, -1, Qt::WordBreak, entry);
  }
  TQRect newrect;
  p.drawText( rect, Qt::WordBreak, entry, -1, &newrect );
  y = newrect.bottom() + 7;
}

void CalPrintPluginBase::drawJournal( Journal * journal, TQPainter &p, int x, int &y,
                                  int width, int pageHeight )
{
  TQFont oldFont( p.font() );
  p.setFont( TQFont( "sans-serif", 15 ) );
  TQString headerText;
  TQString dateText( KGlobal::locale()->
        formatDate( journal->dtStart().date(), false ) );

  if ( journal->summary().isEmpty() ) {
    headerText = dateText;
  } else {
    headerText = i18n("Description - date", "%1 - %2")
                     .arg( journal->summary() )
                     .arg( dateText );
  }

  TQRect rect( p.boundingRect( x, y, width, -1, Qt::WordBreak, headerText) );
  if ( rect.bottom() > pageHeight) {
    // Start new page...
    y=0;
    mPrinter->newPage();
    rect = p.boundingRect( x, y, width, -1, Qt::WordBreak, headerText );
  }
  TQRect newrect;
  p.drawText( rect, Qt::WordBreak, headerText, -1, &newrect );
  p.setFont( oldFont );

  y = newrect.bottom() + 4;

  p.drawLine( x + 3, y, x + width - 6, y );
  y += 5;

  drawJournalField( p, i18n("Person: %1"), journal->organizer().fullName(), x, y, width, pageHeight );
  drawJournalField( p, i18n("%1"), journal->description(), x, y, width, pageHeight );
  y += 10;
}


void CalPrintPluginBase::drawSplitHeaderRight( TQPainter &p, const TQDate &fd,
                                           const TQDate &td,
                                           const TQDate &,
                                           int width, int )
{
  TQFont oldFont( p.font() );

  TQPen oldPen( p.pen() );
  TQPen pen( Qt::black, 4 );

  TQString title;
  if ( mCalSys ) {
    if ( fd.month() == td.month() ) {
      title = i18n("Date range: Month dayStart - dayEnd", "%1 %2 - %3")
        .arg( mCalSys->monthName( fd.month(), false ) )
        .arg( mCalSys->dayString( fd, false ) )
        .arg( mCalSys->dayString( td, false ) );
    } else {
      title = i18n("Date range: monthStart dayStart - monthEnd dayEnd", "%1 %2 - %3 %4")
        .arg( mCalSys->monthName( fd.month(), false ) )
        .arg( mCalSys->dayString( fd, false ) )
        .arg( mCalSys->monthName( td.month(), false ) )
        .arg( mCalSys->dayString( td, false ) );
    }
  }

  TQFont serifFont("Times", 30);
  p.setFont(serifFont);

  int lineSpacing = p.fontMetrics().lineSpacing();
  p.drawText( 0, lineSpacing * 0, width, lineSpacing,
              Qt::AlignRight | Qt::AlignTop, title );

  title.truncate(0);

  p.setPen( pen );
  p.drawLine(300, lineSpacing * 1, width, lineSpacing * 1);
  p.setPen( oldPen );

  p.setFont(TQFont("Times", 20, TQFont::Bold, TRUE));
  int newlineSpacing = p.fontMetrics().lineSpacing();
  title += TQString::number(fd.year());
  p.drawText( 0, lineSpacing * 1 + 4, width, newlineSpacing,
              Qt::AlignRight | Qt::AlignTop, title );

  p.setFont( oldFont );
}

#endif

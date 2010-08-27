/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

#include <tqhbox.h>
#include <tqvbox.h>
#include <tqlabel.h>
#include <tqframe.h>
#include <tqlayout.h>
#ifndef KORG_NOSPLITTER
#include <tqsplitter.h>
#endif
#include <tqfont.h>
#include <tqfontmetrics.h>
#include <tqpopupmenu.h>
#include <tqtooltip.h>
#include <tqpainter.h>
#include <tqpushbutton.h>
#include <tqcursor.h>
#include <tqbitarray.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kholidays.h>

#include <libkcal/calendar.h>
#include <libkcal/icaldrag.h>
#include <libkcal/dndfactory.h>
#include <libkcal/calfilter.h>

#include <kcalendarsystem.h>

#include "koglobals.h"
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif
#include "koprefs.h"
#include "koagenda.h"
#include "koagendaitem.h"
#include "timelabels.h"

#include "koincidencetooltip.h"
#include "kogroupware.h"
#include "kodialogmanager.h"
#include "koeventpopupmenu.h"

#include "koagendaview.h"
#include "koagendaview.moc"

using namespace KOrg;


EventIndicator::EventIndicator(Location loc,TQWidget *parent,const char *name)
  : TQFrame(parent,name)
{
  mColumns = 1;
  mEnabled.resize( mColumns );
  mLocation = loc;

  if (mLocation == Top) mPixmap = KOGlobals::self()->smallIcon("upindicator");
  else mPixmap = KOGlobals::self()->smallIcon("downindicator");

  setMinimumHeight(mPixmap.height());
}

EventIndicator::~EventIndicator()
{
}

void EventIndicator::drawContents(TQPainter *p)
{
//  kdDebug(5850) << "======== top: " << contentsRect().top() << "  bottom "
//         << contentsRect().bottom() << "  left " << contentsRect().left()
//         << "  right " << contentsRect().right() << endl;

  int i;
  for(i=0;i<mColumns;++i) {
    if (mEnabled[i]) {
      int cellWidth = contentsRect().right()/mColumns;
      int xOffset = KOGlobals::self()->reverseLayout() ?
               (mColumns - 1 - i)*cellWidth + cellWidth/2 -mPixmap.width()/2 :
               i*cellWidth + cellWidth/2 -mPixmap.width()/2;
      p->drawPixmap(TQPoint(xOffset,0),mPixmap);
    }
  }
}

void EventIndicator::changeColumns(int columns)
{
  mColumns = columns;
  mEnabled.resize(mColumns);

  update();
}

void EventIndicator::enableColumn(int column, bool enable)
{
  mEnabled[column] = enable;
}


#include <libkcal/incidence.h>

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


KOAlternateLabel::KOAlternateLabel(const TQString &shortlabel, const TQString &longlabel,
    const TQString &extensivelabel, TQWidget *parent, const char *name )
  : TQLabel(parent, name), mTextTypeFixed(false), mShortText(shortlabel),
    mLongText(longlabel), mExtensiveText(extensivelabel)
{
  setSizePolicy(TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed ));
  if (mExtensiveText.isEmpty()) mExtensiveText = mLongText;
  squeezeTextToLabel();
}

KOAlternateLabel::~KOAlternateLabel()
{
}

void KOAlternateLabel::useShortText()
{
  mTextTypeFixed = true;
  TQLabel::setText( mShortText );
  TQToolTip::remove( this );
  TQToolTip::add( this, mExtensiveText );
}

void KOAlternateLabel::useLongText()
{
  mTextTypeFixed = true;
  TQLabel::setText( mLongText );
  TQToolTip::remove( this );
  TQToolTip::add( this, mExtensiveText );
}

void KOAlternateLabel::useExtensiveText()
{
  mTextTypeFixed = true;
  TQLabel::setText( mExtensiveText );
  TQToolTip::remove( this );
  TQToolTip::hide();
}

void KOAlternateLabel::useDefaultText()
{
  mTextTypeFixed = false;
  squeezeTextToLabel();
}

void KOAlternateLabel::squeezeTextToLabel()
{
  if (mTextTypeFixed) return;

  TQFontMetrics fm(fontMetrics());
  int labelWidth = size().width();
  int textWidth = fm.width(mLongText);
  int longTextWidth = fm.width(mExtensiveText);
  if (longTextWidth <= labelWidth) {
    TQLabel::setText( mExtensiveText );
    TQToolTip::remove( this );
    TQToolTip::hide();
  } else if (textWidth <= labelWidth) {
    TQLabel::setText( mLongText );
    TQToolTip::remove( this );
    TQToolTip::add( this, mExtensiveText );
  } else {
    TQLabel::setText( mShortText );
    TQToolTip::remove( this );
    TQToolTip::add( this, mExtensiveText );
  }
}

void KOAlternateLabel::resizeEvent( TQResizeEvent * )
{
  squeezeTextToLabel();
}

TQSize KOAlternateLabel::minimumSizeHint() const
{
  TQSize sh = TQLabel::minimumSizeHint();
  sh.setWidth(-1);
  return sh;
}

void KOAlternateLabel::setText( const TQString &text ) {
  mLongText = text;
  squeezeTextToLabel();
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

KOAgendaView::KOAgendaView(Calendar *cal,TQWidget *parent,const char *name, bool isSideBySide ) :
  KOrg::AgendaView (cal,parent,name), mExpandButton( 0 ), mAllowAgendaUpdate( true ),
  mUpdateItem( 0 ),
  mResource( 0 ),
  mIsSideBySide( isSideBySide ),
  mPendingChanges( true )
{
  mSelectedDates.append(TQDate::currentDate());

  mLayoutDayLabels = 0;
  mDayLabelsFrame = 0;
  mDayLabels = 0;

  bool isRTL = KOGlobals::self()->reverseLayout();

  if ( KOPrefs::instance()->compactDialogs() ) {
    if ( KOPrefs::instance()->mVerticalScreen ) {
      mExpandedPixmap = KOGlobals::self()->smallIcon( "1downarrow" );
      mNotExpandedPixmap = KOGlobals::self()->smallIcon( "1uparrow" );
    } else {
      mExpandedPixmap = KOGlobals::self()->smallIcon( isRTL ? "1leftarrow" : "1rightarrow" );
      mNotExpandedPixmap = KOGlobals::self()->smallIcon( isRTL ? "1rightarrow" : "1leftarrow" );
    }
  }

  TQBoxLayout *topLayout = new TQVBoxLayout(this);

  // Create day name labels for agenda columns
  mDayLabelsFrame = new TQHBox(this);
  topLayout->addWidget(mDayLabelsFrame);

  // Create agenda splitter
#ifndef KORG_NOSPLITTER
  mSplitterAgenda = new TQSplitter(Vertical,this);
  topLayout->addWidget(mSplitterAgenda);

#if KDE_IS_VERSION( 3, 1, 93 )
  mSplitterAgenda->setOpaqueResize( KGlobalSettings::opaqueResize() );
#else
  mSplitterAgenda->setOpaqueResize();
#endif

  mAllDayFrame = new TQHBox(mSplitterAgenda);

  TQWidget *agendaFrame = new TQWidget(mSplitterAgenda);
#else
  TQVBox *mainBox = new TQVBox( this );
  topLayout->addWidget( mainBox );

  mAllDayFrame = new TQHBox(mainBox);

  TQWidget *agendaFrame = new TQWidget(mainBox);
#endif

  // Create all-day agenda widget
  mDummyAllDayLeft = new TQVBox( mAllDayFrame );
  if ( isSideBySide )
    mDummyAllDayLeft->hide();

  if ( KOPrefs::instance()->compactDialogs() ) {
    mExpandButton = new TQPushButton(mDummyAllDayLeft);
    mExpandButton->setPixmap( mNotExpandedPixmap );
    mExpandButton->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed,
                                  TQSizePolicy::Fixed ) );
    connect( mExpandButton, TQT_SIGNAL( clicked() ), TQT_SIGNAL( toggleExpand() ) );
  } else {
    TQLabel *label = new TQLabel( i18n("All Day"), mDummyAllDayLeft );
    label->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::WordBreak );
  }

  mAllDayAgenda = new KOAgenda(1,mAllDayFrame);
  TQWidget *dummyAllDayRight = new TQWidget(mAllDayFrame);

  // Create agenda frame
  TQGridLayout *agendaLayout = new TQGridLayout(agendaFrame,3,3);
//  TQHBox *agendaFrame = new TQHBox(splitterAgenda);

  // create event indicator bars
  mEventIndicatorTop = new EventIndicator(EventIndicator::Top,agendaFrame);
  agendaLayout->addWidget(mEventIndicatorTop,0,1);
  mEventIndicatorBottom = new EventIndicator(EventIndicator::Bottom,
                                             agendaFrame);
  agendaLayout->addWidget(mEventIndicatorBottom,2,1);
  TQWidget *dummyAgendaRight = new TQWidget(agendaFrame);
  agendaLayout->addWidget(dummyAgendaRight,0,2);

  // Create time labels
  mTimeLabels = new TimeLabels(24,agendaFrame);
  agendaLayout->addWidget(mTimeLabels,1,0);

  // Create agenda
  mAgenda = new KOAgenda(1,96,KOPrefs::instance()->mHourSize,agendaFrame);
  agendaLayout->addMultiCellWidget(mAgenda,1,1,1,2);
  agendaLayout->setColStretch(1,1);

  // Create event context menu for agenda
  mAgendaPopup = eventPopup();

  // Create event context menu for all day agenda
  mAllDayAgendaPopup = eventPopup();

  // make connections between dependent widgets
  mTimeLabels->setAgenda(mAgenda);
  if ( isSideBySide )
    mTimeLabels->hide();

  // Update widgets to reflect user preferences
//  updateConfig();

  createDayLabels();

  if ( !isSideBySide ) {
    // these blank widgets make the All Day Event box line up with the agenda
    dummyAllDayRight->setFixedWidth(mAgenda->verticalScrollBar()->width());
    dummyAgendaRight->setFixedWidth(mAgenda->verticalScrollBar()->width());
  }

  updateTimeBarWidth();

  // Scrolling
  connect(mAgenda->verticalScrollBar(),TQT_SIGNAL(valueChanged(int)),
          mTimeLabels, TQT_SLOT(positionChanged()));

  connect( mAgenda,
    TQT_SIGNAL( zoomView( const int, const TQPoint & ,const Qt::Orientation ) ),
    TQT_SLOT( zoomView( const int, const TQPoint &, const Qt::Orientation ) ) );

  connect(mTimeLabels->verticalScrollBar(),TQT_SIGNAL(valueChanged(int)),
          TQT_SLOT(setContentsPos(int)));

  // Create Events, depends on type of agenda
  connect( mAgenda, TQT_SIGNAL(newTimeSpanSignal(const TQPoint &, const TQPoint &)),
                    TQT_SLOT(newTimeSpanSelected(const TQPoint &, const TQPoint &)));
  connect( mAllDayAgenda, TQT_SIGNAL(newTimeSpanSignal(const TQPoint &, const TQPoint &)),
                          TQT_SLOT(newTimeSpanSelectedAllDay(const TQPoint &, const TQPoint &)));

  // event indicator update
  connect( mAgenda, TQT_SIGNAL(lowerYChanged(int)),
                    TQT_SLOT(updateEventIndicatorTop(int)));
  connect( mAgenda, TQT_SIGNAL(upperYChanged(int)),
                    TQT_SLOT(updateEventIndicatorBottom(int)));

  connectAgenda( mAgenda, mAgendaPopup, mAllDayAgenda );
  connectAgenda( mAllDayAgenda, mAllDayAgendaPopup, mAgenda);

  if ( cal )
    cal->registerObserver( this );

  CalendarResources *calres = dynamic_cast<CalendarResources*>( cal );
  if ( calres ) {
    connect( calres, TQT_SIGNAL(signalResourceAdded(ResourceCalendar *)), TQT_SLOT(resourcesChanged()) );
    connect( calres, TQT_SIGNAL(signalResourceModified( ResourceCalendar *)), TQT_SLOT(resourcesChanged()) );
    connect( calres, TQT_SIGNAL(signalResourceDeleted(ResourceCalendar *)), TQT_SLOT(resourcesChanged()) );
  }
}


KOAgendaView::~KOAgendaView()
{
  if ( calendar() )
    calendar()->unregisterObserver( this );
  delete mAgendaPopup;
  delete mAllDayAgendaPopup;
}

void KOAgendaView::connectAgenda( KOAgenda *agenda, TQPopupMenu *popup,
                                  KOAgenda *otherAgenda )
{
  connect( agenda, TQT_SIGNAL( showIncidencePopupSignal( Incidence *, const TQDate & ) ),
           popup, TQT_SLOT( showIncidencePopup( Incidence *, const TQDate & ) ) );

  connect( agenda, TQT_SIGNAL( showNewEventPopupSignal() ),
           TQT_SLOT( showNewEventPopup() ) );

  agenda->setCalendar( calendar() );

  // Create/Show/Edit/Delete Event
  connect( agenda, TQT_SIGNAL( newEventSignal() ), TQT_SIGNAL( newEventSignal() ) );

  connect( agenda, TQT_SIGNAL( newStartSelectSignal() ),
           otherAgenda, TQT_SLOT( clearSelection() ) );
  connect( agenda, TQT_SIGNAL( newStartSelectSignal() ),
           TQT_SIGNAL( timeSpanSelectionChanged()) );

  connect( agenda, TQT_SIGNAL( editIncidenceSignal( Incidence * ) ),
                   TQT_SIGNAL( editIncidenceSignal( Incidence * ) ) );
  connect( agenda, TQT_SIGNAL( showIncidenceSignal( Incidence * ) ),
                   TQT_SIGNAL( showIncidenceSignal( Incidence * ) ) );
  connect( agenda, TQT_SIGNAL( deleteIncidenceSignal( Incidence * ) ),
                   TQT_SIGNAL( deleteIncidenceSignal( Incidence * ) ) );

  connect( agenda, TQT_SIGNAL( startMultiModify( const TQString & ) ),
                   TQT_SIGNAL( startMultiModify( const TQString & ) ) );
  connect( agenda, TQT_SIGNAL( endMultiModify() ),
                   TQT_SIGNAL( endMultiModify() ) );

  connect( agenda, TQT_SIGNAL( itemModified( KOAgendaItem * ) ),
                   TQT_SLOT( updateEventDates( KOAgendaItem * ) ) );
  connect( agenda, TQT_SIGNAL( enableAgendaUpdate( bool ) ),
                   TQT_SLOT( enableAgendaUpdate( bool ) ) );

  // drag signals
  connect( agenda, TQT_SIGNAL( startDragSignal( Incidence * ) ),
           TQT_SLOT( startDrag( Incidence * ) ) );

  // synchronize selections
  connect( agenda, TQT_SIGNAL( incidenceSelected( Incidence * ) ),
           otherAgenda, TQT_SLOT( deselectItem() ) );
  connect( agenda, TQT_SIGNAL( incidenceSelected( Incidence * ) ),
           TQT_SIGNAL( incidenceSelected( Incidence * ) ) );

  // rescheduling of todos by d'n'd
  connect( agenda, TQT_SIGNAL( droppedToDo( Todo *, const TQPoint &, bool ) ),
           TQT_SLOT( slotTodoDropped( Todo *, const TQPoint &, bool ) ) );

}

void KOAgendaView::zoomInVertically( )
{
  if ( !mIsSideBySide )
    KOPrefs::instance()->mHourSize++;
  mAgenda->updateConfig();
  mAgenda->checkScrollBoundaries();

  mTimeLabels->updateConfig();
  mTimeLabels->positionChanged();
  mTimeLabels->repaint();

  updateView();
}

void KOAgendaView::zoomOutVertically( )
{

  if ( KOPrefs::instance()->mHourSize > 4 || mIsSideBySide ) {

    if ( !mIsSideBySide )
      KOPrefs::instance()->mHourSize--;
    mAgenda->updateConfig();
    mAgenda->checkScrollBoundaries();

    mTimeLabels->updateConfig();
    mTimeLabels->positionChanged();
    mTimeLabels->repaint();

    updateView();
  }
}

void KOAgendaView::zoomInHorizontally( const TQDate &date)
{
  TQDate begin;
  TQDate newBegin;
  TQDate dateToZoom = date;
  int ndays,count;

  begin = mSelectedDates.first();
  ndays = begin.daysTo( mSelectedDates.last() );

  // zoom with Action and are there a selected Incidence?, Yes, I zoom in to it.
  if ( ! dateToZoom.isValid () )
    dateToZoom=mAgenda->selectedIncidenceDate();

  if( !dateToZoom.isValid() ) {
    if ( ndays > 1 ) {
      newBegin=begin.addDays(1);
      count = ndays-1;
      emit zoomViewHorizontally ( newBegin , count );
    }
  } else {
    if ( ndays <= 2 ) {
      newBegin = dateToZoom;
      count = 1;
    } else  {
      newBegin = dateToZoom.addDays( -ndays/2 +1  );
      count = ndays -1 ;
    }
    emit zoomViewHorizontally ( newBegin , count );
  }
}

void KOAgendaView::zoomOutHorizontally( const TQDate &date )
{
  TQDate begin;
  TQDate newBegin;
  TQDate dateToZoom = date;
  int ndays,count;

  begin = mSelectedDates.first();
  ndays = begin.daysTo( mSelectedDates.last() );

  // zoom with Action and are there a selected Incidence?, Yes, I zoom out to it.
  if ( ! dateToZoom.isValid () )
    dateToZoom=mAgenda->selectedIncidenceDate();

  if ( !dateToZoom.isValid() ) {
    newBegin = begin.addDays(-1);
    count = ndays+3 ;
  } else {
    newBegin = dateToZoom.addDays( -ndays/2-1 );
    count = ndays+3;
  }

  if ( abs( count ) >= 31 )
    kdDebug(5850) << "change to the mounth view?"<<endl;
  else
    //We want to center the date
    emit zoomViewHorizontally( newBegin, count );
}

void KOAgendaView::zoomView( const int delta, const TQPoint &pos,
  const Qt::Orientation orient )
{
  static TQDate zoomDate;
  static TQTimer *t = new TQTimer( this );


  //Zoom to the selected incidence, on the other way
  // zoom to the date on screen after the first mousewheel move.
  if ( orient == Qt::Horizontal ) {
    TQDate date=mAgenda->selectedIncidenceDate();
    if ( date.isValid() )
      zoomDate=date;
    else{
      if ( !t->isActive() ) {
        zoomDate= mSelectedDates[pos.x()];
      }
      t->start ( 1000,true );
    }
    if ( delta > 0 )
      zoomOutHorizontally( zoomDate );
    else
      zoomInHorizontally( zoomDate );
  } else {
    // Vertical zoom
    TQPoint posConstentsOld = mAgenda->gridToContents(pos);
    if ( delta > 0 ) {
      zoomOutVertically();
    } else {
      zoomInVertically();
    }
    TQPoint posConstentsNew = mAgenda->gridToContents(pos);
    mAgenda->scrollBy( 0, posConstentsNew.y() - posConstentsOld.y() );
  }
}

void KOAgendaView::createDayLabels()
{
//  kdDebug(5850) << "KOAgendaView::createDayLabels()" << endl;

  // ### Before deleting and recreating we could check if mSelectedDates changed...
  // It would remove some flickering and gain speed (since this is called by
  // each updateView() call)
  delete mDayLabels;

  mDayLabels = new TQFrame (mDayLabelsFrame);
  mLayoutDayLabels = new TQHBoxLayout(mDayLabels);
  if ( !mIsSideBySide )
    mLayoutDayLabels->addSpacing(mTimeLabels->width());

  const KCalendarSystem*calsys=KOGlobals::self()->calendarSystem();

  DateList::ConstIterator dit;
  for( dit = mSelectedDates.begin(); dit != mSelectedDates.end(); ++dit ) {
    TQDate date = *dit;
    TQBoxLayout *dayLayout = new TQVBoxLayout(mLayoutDayLabels);
    mLayoutDayLabels->setStretchFactor(dayLayout, 1);
//    dayLayout->setMinimumWidth(1);

    int dW = calsys->dayOfWeek(date);
    TQString veryLongStr = KGlobal::locale()->formatDate( date );
    TQString longstr = i18n( "short_weekday date (e.g. Mon 13)","%1 %2" )
        .arg( calsys->weekDayName( dW, true ) )
        .arg( calsys->day(date) );
    TQString shortstr = TQString::number(calsys->day(date));

    KOAlternateLabel *dayLabel = new KOAlternateLabel(shortstr,
      longstr, veryLongStr, mDayLabels);
    dayLabel->setMinimumWidth(1);
    dayLabel->setAlignment(TQLabel::AlignHCenter);
    if (date == TQDate::currentDate()) {
      TQFont font = dayLabel->font();
      font.setBold(true);
      dayLabel->setFont(font);
    }
    dayLayout->addWidget(dayLabel);

    // if a holiday region is selected, show the holiday name
    TQStringList texts = KOGlobals::self()->holiday( date );
    TQStringList::ConstIterator textit = texts.begin();
    for ( ; textit != texts.end(); ++textit ) {
      // use a KOAlternateLabel so when the text doesn't fit any more a tooltip is used
      KOAlternateLabel*label = new KOAlternateLabel( (*textit), (*textit), TQString::null, mDayLabels );
      label->setMinimumWidth(1);
      label->setAlignment(AlignCenter);
      dayLayout->addWidget(label);
    }

#ifndef KORG_NOPLUGINS
    CalendarDecoration::List cds = KOCore::self()->calendarDecorations();
    CalendarDecoration *it;
    for(it = cds.first(); it; it = cds.next()) {
      TQString text = it->shortText( date );
      if ( !text.isEmpty() ) {
        // use a KOAlternateLabel so when the text doesn't fit any more a tooltip is used
        KOAlternateLabel*label = new KOAlternateLabel( text, text, TQString::null, mDayLabels );
        label->setMinimumWidth(1);
        label->setAlignment(AlignCenter);
        dayLayout->addWidget(label);
      }
    }

    for(it = cds.first(); it; it = cds.next()) {
      TQWidget *wid = it->smallWidget(mDayLabels,date);
      if ( wid ) {
//      wid->setHeight(20);
        dayLayout->addWidget(wid);
      }
    }
#endif
  }

  if ( !mIsSideBySide )
    mLayoutDayLabels->addSpacing(mAgenda->verticalScrollBar()->width());
  mDayLabels->show();
}

void KOAgendaView::enableAgendaUpdate( bool enable )
{
  mAllowAgendaUpdate = enable;
}

int KOAgendaView::maxDatesHint()
{
  // Not sure about the max number of events, so return 0 for now.
  return 0;
}

int KOAgendaView::currentDateCount()
{
  return mSelectedDates.count();
}

Incidence::List KOAgendaView::selectedIncidences()
{
  Incidence::List selected;
  Incidence *incidence;

  incidence = mAgenda->selectedIncidence();
  if (incidence) selected.append(incidence);

  incidence = mAllDayAgenda->selectedIncidence();
  if (incidence) selected.append(incidence);

  return selected;
}

DateList KOAgendaView::selectedDates()
{
  DateList selected;
  TQDate qd;

  qd = mAgenda->selectedIncidenceDate();
  if (qd.isValid()) selected.append(qd);

  qd = mAllDayAgenda->selectedIncidenceDate();
  if (qd.isValid()) selected.append(qd);

  return selected;
}

bool KOAgendaView::eventDurationHint( TQDateTime &startDt, TQDateTime &endDt,
                                      bool &allDay )
{
  if ( selectionStart().isValid() ) {
    TQDateTime start = selectionStart();
    TQDateTime end = selectionEnd();

    if ( start.secsTo( end ) == 15*60 ) {
      // One cell in the agenda view selected, e.g.
      // because of a double-click, => Use the default duration
      TQTime defaultDuration( KOPrefs::instance()->mDefaultDuration.time() );
      int addSecs = ( defaultDuration.hour()*3600 ) +
                    ( defaultDuration.minute()*60 );
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
bool KOAgendaView::selectedIsSingleCell()
{
  if ( !selectionStart().isValid() || !selectionEnd().isValid() ) return false;

  if (selectedIsAllDay()) {
    int days = selectionStart().daysTo(selectionEnd());
    return ( days < 1 );
  } else {
    int secs = selectionStart().secsTo(selectionEnd());
    return ( secs <= 24*60*60/mAgenda->rows() );
  }
}


void KOAgendaView::updateView()
{
//  kdDebug(5850) << "KOAgendaView::updateView()" << endl;
  fillAgenda();
}


/*
  Update configuration settings for the agenda view. This method is not
  complete.
*/
void KOAgendaView::updateConfig()
{
//  kdDebug(5850) << "KOAgendaView::updateConfig()" << endl;

  // update config for children
  mTimeLabels->updateConfig();
  mAgenda->updateConfig();
  mAllDayAgenda->updateConfig();

  // widget synchronization
  // FIXME: find a better way, maybe signal/slot
  mTimeLabels->positionChanged();

  // for some reason, this needs to be called explicitly
  mTimeLabels->repaint();

  updateTimeBarWidth();

  // ToolTips displaying summary of events
  KOAgendaItem::toolTipGroup()->setEnabled(KOPrefs::instance()
                                           ->mEnableToolTips);

  setHolidayMasks();

  createDayLabels();

  updateView();
}

void KOAgendaView::updateTimeBarWidth()
{
  int width;

  width = mDummyAllDayLeft->fontMetrics().width( i18n("All Day") );
  width = QMAX( width, mTimeLabels->width() );

  mDummyAllDayLeft->setFixedWidth( width );
  mTimeLabels->setFixedWidth( width );
}


void KOAgendaView::updateEventDates( KOAgendaItem *item )
{
  kdDebug(5850) << "KOAgendaView::updateEventDates(): " << item->text() << endl;

  TQDateTime startDt,endDt;

  // Start date of this incidence, calculate the offset from it (so recurring and
  // non-recurring items can be treated exactly the same, we never need to check
  // for doesRecur(), because we only move the start day by the number of days the
  // agenda item was really moved. Smart, isn't it?)
  TQDate thisDate;
  if ( item->cellXLeft() < 0 ) {
    thisDate = ( mSelectedDates.first() ).addDays( item->cellXLeft() );
  } else {
    thisDate = mSelectedDates[ item->cellXLeft() ];
  }
  TQDate oldThisDate( item->itemDate() );
  int daysOffset = oldThisDate.daysTo( thisDate );
  int daysLength = 0;

//  startDt.setDate( startDate );

  Incidence *incidence = item->incidence();
  if ( !incidence ) return;
  if ( !mChanger || !mChanger->beginChange(incidence) ) return;
  Incidence *oldIncidence = incidence->clone();

  TQTime startTime(0,0,0), endTime(0,0,0);
  if ( incidence->doesFloat() ) {
    daysLength = item->cellWidth() - 1;
  } else {
    startTime = mAgenda->gyToTime( item->cellYTop() );
    if ( item->lastMultiItem() ) {
      endTime = mAgenda->gyToTime( item->lastMultiItem()->cellYBottom() + 1 );
      daysLength = item->lastMultiItem()->cellXLeft() - item->cellXLeft();
    } else {
      endTime = mAgenda->gyToTime( item->cellYBottom() + 1 );
    }
  }

//  kdDebug(5850) << "KOAgendaView::updateEventDates(): now setting dates" << endl;
  // FIXME: use a visitor here
  if ( incidence->type() == "Event" ) {
    startDt = incidence->dtStart();
    startDt = startDt.addDays( daysOffset );
    startDt.setTime( startTime );
    endDt = startDt.addDays( daysLength );
    endDt.setTime( endTime );
    Event*ev = static_cast<Event*>(incidence);
    if( incidence->dtStart() == startDt && ev->dtEnd() == endDt ) {
      // No change
      delete oldIncidence;
      return;
    }
    incidence->setDtStart( startDt );
    ev->setDtEnd( endDt );
  } else if ( incidence->type() == "Todo" ) {
    Todo *td = static_cast<Todo*>(incidence);
    startDt = td->hasStartDate() ? td->dtStart() : td->dtDue();
    startDt = thisDate.addDays( td->dtDue().daysTo( startDt ) );
    startDt.setTime( startTime );
    endDt.setDate( thisDate );
    endDt.setTime( endTime );

    if( td->dtDue() == endDt ) {
      // No change
      delete oldIncidence;
      return;
    }
  }
  // FIXME: Adjusting the recurrence should really go to CalendarView so this
  // functionality will also be available in other views!
  // TODO_Recurrence: This does not belong here, and I'm not really sure
  // how it's supposed to work anyway.
  Recurrence *recur = incidence->recurrence();
/*  if ( recur->doesRecur() && daysOffset != 0 ) {
    switch ( recur->recurrenceType() ) {
      case Recurrence::rYearlyPos: {
        int freq = recur->frequency();
        int duration = recur->duration();
        TQDate endDt( recur->endDate() );
        bool negative = false;

        TQPtrList<Recurrence::rMonthPos> monthPos( recur->yearMonthPositions() );
        if ( monthPos.first() ) {
          negative = monthPos.first()->negative;
        }
        TQBitArray days( 7 );
        int pos = 0;
        days.fill( false );
        days.setBit( thisDate.dayOfWeek() - 1 );
        if ( negative ) {
          pos =  - ( thisDate.daysInMonth() - thisDate.day() - 1 ) / 7 - 1;
        } else {
          pos =  ( thisDate.day()-1 ) / 7 + 1;
        }
        // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
        recur->unsetRecurs();
        if ( duration != 0 ) {
          recur->setYearly( Recurrence::rYearlyPos, freq, duration );
        } else {
          recur->setYearly( Recurrence::rYearlyPos, freq, endDt );
        }
        recur->addYearlyMonthPos( pos, days );
        recur->addYearlyNum( thisDate.month() );

        break; }
        case Recurrence::rYearlyDay: {
          int freq = recur->frequency();
          int duration = recur->duration();
          TQDate endDt( recur->endDate() );
        // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
          recur->unsetRecurs();
          if ( duration == 0 ) { // end by date
            recur->setYearly( Recurrence::rYearlyDay, freq, endDt );
          } else {
            recur->setYearly( Recurrence::rYearlyDay, freq, duration );
          }
          recur->addYearlyNum( thisDate.dayOfYear() );
          break; }
          case Recurrence::rYearlyMonth: {
            int freq = recur->frequency();
            int duration = recur->duration();
            TQDate endDt( recur->endDate() );
        // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
            recur->unsetRecurs();
            if ( duration != 0 ) {
              recur->setYearlyByDate( thisDate.day(), recur->feb29YearlyType(), freq, duration );
            } else {
              recur->setYearlyByDate( thisDate.day(), recur->feb29YearlyType(), freq, endDt );
            }
            recur->addYearlyNum( thisDate.month() );
            break; }
            case Recurrence::rMonthlyPos: {
              int freq = recur->frequency();
              int duration = recur->duration();
              TQDate endDt( recur->endDate() );
              TQPtrList<Recurrence::rMonthPos> monthPos( recur->monthPositions() );
              if ( !monthPos.isEmpty() ) {
          // FIXME: How shall I adapt the day x of week Y if we move the date across month borders???
          // for now, just use the date of the moved item and assume the recurrence only occurs on that day.
          // That's fine for korganizer, but might mess up other organizers.
                TQBitArray rDays( 7 );
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

          // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
                recur->unsetRecurs();
                if ( duration == 0 ) { // end by date
                  recur->setMonthly( Recurrence::rMonthlyPos, freq, endDt );
                } else {
                  recur->setMonthly( Recurrence::rMonthlyPos, freq, duration );
                }
                recur->addMonthlyPos( newPos, rDays );
              }
              break;}
              case Recurrence::rMonthlyDay: {
                int freq = recur->frequency();
                int duration = recur->duration();
                TQDate endDt( recur->endDate() );
                TQPtrList<int> monthDays( recur->monthDays() );
        // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
                recur->unsetRecurs();
                if ( duration == 0 ) { // end by date
                  recur->setMonthly( Recurrence::rMonthlyDay, freq, endDt );
                } else {
                  recur->setMonthly( Recurrence::rMonthlyDay, freq, duration );
                }
        // FIXME: How shall I adapt the n-th day if we move the date across month borders???
        // for now, just use the date of the moved item and assume the recurrence only occurs on that day.
        // That's fine for korganizer, but might mess up other organizers.
                recur->addMonthlyDay( thisDate.day() );

                break;}
                case Recurrence::rWeekly: {
                  TQBitArray days(7), oldDays( recur->days() );
                  int offset = daysOffset % 7;
                  if ( offset<0 ) offset = (offset+7) % 7;
        // rotate the days
                  for (int d=0; d<7; d++ ) {
                    days.setBit( (d+offset) % 7, oldDays.at(d) );
                  }
                  if ( recur->duration() == 0 ) { // end by date
                    recur->setWeekly( recur->frequency(), days, recur->endDate(), recur->weekStart() );
                  } else { // duration or no end
                    recur->setWeekly( recur->frequency(), days, recur->duration(), recur->weekStart() );
                  }
                  break;}
      // nothing to be done for the following:
      case Recurrence::rDaily:
      case Recurrence::rHourly:
      case Recurrence::rMinutely:
      case Recurrence::rNone:
      default:
        break;
    }
    if ( recur->duration()==0 ) { // end by date
      recur->setEndDate( recur->endDate().addDays( daysOffset ) );
    }
    KMessageBox::information( this, i18n("A recurring calendar item was moved "
                              "to a different day. The recurrence settings "
                              "have been updated with that move. Please check "
                              "them in the editor."),
                              i18n("Recurrence Moved"),
                              "RecurrenceMoveInAgendaWarning" );
  }*/

  // FIXME: use a visitor here
  if ( incidence->type() == "Event" ) {
    incidence->setDtStart( startDt );
    (static_cast<Event*>( incidence ) )->setDtEnd( endDt );
  } else if ( incidence->type() == "Todo" ) {
    Todo *td = static_cast<Todo*>( incidence );
    if ( td->hasStartDate() )
      td->setDtStart( startDt );
    td->setDtDue( endDt );
  }

  item->setItemDate( startDt.date() );

  KOIncidenceToolTip::remove( item );
  KOIncidenceToolTip::add( item, incidence, KOAgendaItem::toolTipGroup() );

  mChanger->changeIncidence( oldIncidence, incidence );
  mChanger->endChange(incidence);
  delete oldIncidence;

  // don't update the agenda as the item already has the correct coordinates.
  // an update would delete the current item and recreate it, but we are still
  // using a pointer to that item! => CRASH
  enableAgendaUpdate( false );
  // We need to do this in a timer to make sure we are not deleting the item
  // we are currently working on, which would lead to crashes
  // Only the actually moved agenda item is already at the correct position and mustn't be
  // recreated. All others have to!!!
  if ( incidence->doesRecur() ) {
    mUpdateItem = incidence;
    TQTimer::singleShot( 0, this, TQT_SLOT( doUpdateItem() ) );
  }

    enableAgendaUpdate( true );

//  kdDebug(5850) << "KOAgendaView::updateEventDates() done " << endl;
}

void KOAgendaView::doUpdateItem()
{
  if ( mUpdateItem ) {
    changeIncidenceDisplay( mUpdateItem, KOGlobals::INCIDENCEEDITED );
    mUpdateItem = 0;
  }
}



void KOAgendaView::showDates( const TQDate &start, const TQDate &end )
{
//  kdDebug(5850) << "KOAgendaView::selectDates" << endl;
  if ( !mSelectedDates.isEmpty() && mSelectedDates.first() == start
        && mSelectedDates.last() == end && !mPendingChanges )
    return;

  mSelectedDates.clear();

  TQDate d = start;
  while (d <= end) {
    mSelectedDates.append(d);
    d = d.addDays( 1 );
  }

  // and update the view
  fillAgenda();
}


void KOAgendaView::showIncidences( const Incidence::List & )
{
  kdDebug(5850) << "KOAgendaView::showIncidences( const Incidence::List & ) is not yet implemented" << endl;
}

void KOAgendaView::insertIncidence( Incidence *incidence, const TQDate &curDate,
                                    int curCol )
{
  if ( !filterByResource( incidence ) )
    return;

  // FIXME: Use a visitor here, or some other method to get rid of the dynamic_cast's
  Event *event = dynamic_cast<Event *>( incidence );
  Todo  *todo  = dynamic_cast<Todo  *>( incidence );

  if ( curCol < 0 ) {
    curCol = mSelectedDates.findIndex( curDate );
  }
  // The date for the event is not displayed, just ignore it
  if ( curCol < 0 || curCol > int( mSelectedDates.size() ) )
    return;

  int beginX;
  int endX;
  if ( event ) {
    beginX = curDate.daysTo( incidence->dtStart().date() ) + curCol;
    endX = curDate.daysTo( event->dateEnd() ) + curCol;
  } else if ( todo ) {
    if ( ! todo->hasDueDate() ) return;  // todo shall not be displayed if it has no date
    beginX = curDate.daysTo( todo->dtDue().date() ) + curCol;
    endX = beginX;
  } else {
    return;
  }

  if ( todo && todo->isOverdue() ) {
    mAllDayAgenda->insertAllDayItem( incidence, curDate, curCol, curCol );
  } else if ( incidence->doesFloat() ) {
// FIXME: This breaks with recurring multi-day events!
    if ( incidence->recurrence()->doesRecur() ) {
      mAllDayAgenda->insertAllDayItem( incidence, curDate, curCol, curCol );
    } else {
      // Insert multi-day events only on the first day, otherwise it will
      // appear multiple times
      if ( ( beginX <= 0 && curCol == 0 ) || beginX == curCol ) {
        mAllDayAgenda->insertAllDayItem( incidence, curDate, beginX, endX );
      }
    }
  } else if ( event && event->isMultiDay() ) {
    int startY = mAgenda->timeToY( event->dtStart().time() );
    TQTime endtime( event->dtEnd().time() );
    if ( endtime == TQTime( 0, 0, 0 ) ) endtime = TQTime( 23, 59, 59 );
    int endY = mAgenda->timeToY( endtime ) - 1;
    if ( (beginX <= 0 && curCol == 0) || beginX == curCol ) {
      mAgenda->insertMultiItem( event, curDate, beginX, endX, startY, endY );
    }
    if ( beginX == curCol ) {
      mMaxY[curCol] = mAgenda->timeToY( TQTime(23,59) );
      if ( startY < mMinY[curCol] ) mMinY[curCol] = startY;
    } else if ( endX == curCol ) {
      mMinY[curCol] = mAgenda->timeToY( TQTime(0,0) );
      if ( endY > mMaxY[curCol] ) mMaxY[curCol] = endY;
    } else {
      mMinY[curCol] = mAgenda->timeToY( TQTime(0,0) );
      mMaxY[curCol] = mAgenda->timeToY( TQTime(23,59) );
    }
  } else {
    int startY = 0, endY = 0;
    if ( event ) {
      startY = mAgenda->timeToY( incidence->dtStart().time() );
      TQTime endtime( event->dtEnd().time() );
      if ( endtime == TQTime( 0, 0, 0 ) ) endtime = TQTime( 23, 59, 59 );
      endY = mAgenda->timeToY( endtime ) - 1;
    }
    if ( todo ) {
      TQTime t = todo->dtDue().time();
      endY = mAgenda->timeToY( t ) - 1;
      startY = mAgenda->timeToY( t.addSecs( -1800 ) );
    }
    if ( endY < startY ) endY = startY;
    mAgenda->insertItem( incidence, curDate, curCol, startY, endY );
    if ( startY < mMinY[curCol] ) mMinY[curCol] = startY;
    if ( endY > mMaxY[curCol] ) mMaxY[curCol] = endY;
  }
}

void KOAgendaView::changeIncidenceDisplayAdded( Incidence *incidence )
{
  Todo *todo = dynamic_cast<Todo *>(incidence);
  CalFilter *filter = calendar()->filter();
  if ( filter && !filter->filterIncidence( incidence ) ||
     ( todo && !KOPrefs::instance()->showAllDayTodo() ) )
    return;

  TQDate f = mSelectedDates.first();
  TQDate l = mSelectedDates.last();
  TQDate startDt = incidence->dtStart().date();

  if ( incidence->doesRecur() ) {
    DateList::ConstIterator dit;
    TQDate curDate;
    for( dit = mSelectedDates.begin(); dit != mSelectedDates.end(); ++dit ) {
      curDate = *dit;
// FIXME: This breaks with recurring multi-day events!
      if ( incidence->recursOn( curDate, calendar() ) ) {
        insertIncidence( incidence, curDate );
      }
    }
    return;
  }

  TQDate endDt;
  if ( incidence->type() == "Event" )
    endDt = (static_cast<Event *>(incidence))->dateEnd();
  if ( todo ) {
    endDt = todo->isOverdue() ? TQDate::currentDate()
                              : todo->dtDue().date();

    if ( endDt >= f && endDt <= l ) {
      insertIncidence( incidence, endDt );
      return;
    }
  }

  if ( startDt >= f && startDt <= l ) {
    insertIncidence( incidence, startDt );
  }
}

void KOAgendaView::changeIncidenceDisplay( Incidence *incidence, int mode )
{
  switch ( mode ) {
    case KOGlobals::INCIDENCEADDED: {
        //  Add an event. No need to recreate the whole view!
        // recreating everything even causes troubles: dropping to the day matrix
        // recreates the agenda items, but the evaluation is still in an agendaItems' code,
        // which was deleted in the mean time. Thus KOrg crashes...
      if ( mAllowAgendaUpdate )
        changeIncidenceDisplayAdded( incidence );
      break;
    }
    case KOGlobals::INCIDENCEEDITED: {
      if ( !mAllowAgendaUpdate ) {
        updateEventIndicators();
      } else {
        removeIncidence( incidence );
        updateEventIndicators();
        changeIncidenceDisplayAdded( incidence );
      }
      break;
    }
    case KOGlobals::INCIDENCEDELETED: {
      mAgenda->removeIncidence( incidence );
      mAllDayAgenda->removeIncidence( incidence );
      updateEventIndicators();
      break;
    }
    default:
      updateView();
  }
}

void KOAgendaView::fillAgenda( const TQDate & )
{
  fillAgenda();
}

void KOAgendaView::fillAgenda()
{
  mPendingChanges = false;

  /* Remember the uids of the selected items. In case one of the
   * items was deleted and re-added, we want to reselect it. */
  const TQString &selectedAgendaUid = mAgenda->lastSelectedUid();
  const TQString &selectedAllDayAgendaUid = mAllDayAgenda->lastSelectedUid();

  enableAgendaUpdate( true );
  clearView();

  mAllDayAgenda->changeColumns(mSelectedDates.count());
  mAgenda->changeColumns(mSelectedDates.count());
  mEventIndicatorTop->changeColumns(mSelectedDates.count());
  mEventIndicatorBottom->changeColumns(mSelectedDates.count());

  createDayLabels();
  setHolidayMasks();

  mMinY.resize(mSelectedDates.count());
  mMaxY.resize(mSelectedDates.count());

  Event::List dayEvents;

  // ToDo items shall be displayed for the day they are due, but only shown today if they are already overdue.
  // Therefore, get all of them.
  Todo::List todos  = calendar()->todos();

  mAgenda->setDateList(mSelectedDates);

  TQDate today = TQDate::currentDate();

  bool somethingReselected = false;
  DateList::ConstIterator dit;
  int curCol = 0;
  for( dit = mSelectedDates.begin(); dit != mSelectedDates.end(); ++dit ) {
    TQDate currentDate = *dit;
//    kdDebug(5850) << "KOAgendaView::fillAgenda(): " << currentDate.toString()
//              << endl;

    dayEvents = calendar()->events(currentDate,
                                   EventSortStartDate,
                                   SortDirectionAscending);

    // Default values, which can never be reached
    mMinY[curCol] = mAgenda->timeToY(TQTime(23,59)) + 1;
    mMaxY[curCol] = mAgenda->timeToY(TQTime(0,0)) - 1;

    unsigned int numEvent;
    for(numEvent=0;numEvent<dayEvents.count();++numEvent) {
      Event *event = *dayEvents.at(numEvent);
//      kdDebug(5850) << " Event: " << event->summary() << endl;
      insertIncidence( event, currentDate, curCol );
      if( event->uid() == selectedAgendaUid && !selectedAgendaUid.isNull() ) {
        mAgenda->selectItemByUID( event->uid() );
        somethingReselected = true;
      }
      if( event->uid() == selectedAllDayAgendaUid && !selectedAllDayAgendaUid.isNull() ) {
        mAllDayAgenda->selectItemByUID( event->uid() );
        somethingReselected = true;
      }

    }
//    if (numEvent == 0) kdDebug(5850) << " No events" << endl;


    // ---------- [display Todos --------------
    if ( KOPrefs::instance()->showAllDayTodo() ) {
      unsigned int numTodo;
      for (numTodo = 0; numTodo < todos.count(); ++numTodo) {
        Todo *todo = *todos.at(numTodo);

        if ( ! todo->hasDueDate() ) continue;  // todo shall not be displayed if it has no date

        if ( !filterByResource( todo ) ) continue;

        // ToDo items shall be displayed for the day they are due, but only showed today if they are already overdue.
        // Already completed items can be displayed on their original due date
        bool overdue = todo->isOverdue();

        if ( (( todo->dtDue().date() == currentDate) && !overdue) ||
             (( currentDate == today) && overdue) ||
             ( todo->recursOn( currentDate ) ) ) {
          if ( todo->doesFloat() || overdue ) {  // Todo has no due-time set or is already overdue
            //kdDebug(5850) << "todo without time:" << todo->dtDueDateStr() << ";" << todo->summary() << endl;

            mAllDayAgenda->insertAllDayItem(todo, currentDate, curCol, curCol);
          } else {
            //kdDebug(5850) << "todo with time:" << todo->dtDueStr() << ";" << todo->summary() << endl;

            int endY = mAgenda->timeToY(todo->dtDue().time()) - 1;
            int startY = endY - 1;

            mAgenda->insertItem(todo,currentDate,curCol,startY,endY);

            if (startY < mMinY[curCol]) mMinY[curCol] = startY;
            if (endY > mMaxY[curCol]) mMaxY[curCol] = endY;
          }
        }
      }
    }
    // ---------- display Todos] --------------

    ++curCol;
  }

  mAgenda->checkScrollBoundaries();
  updateEventIndicators();

//  mAgenda->viewport()->update();
//  mAllDayAgenda->viewport()->update();

// make invalid
  deleteSelectedDateTime();

  if( !somethingReselected ) {
    emit incidenceSelected( 0 );
  }

//  kdDebug(5850) << "Fill Agenda done" << endl;
}

void KOAgendaView::clearView()
{
//  kdDebug(5850) << "ClearView" << endl;
  mAllDayAgenda->clear();
  mAgenda->clear();
}

CalPrinterBase::PrintType KOAgendaView::printType()
{
  if ( currentDateCount() == 1 ) return CalPrinterBase::Day;
  else return CalPrinterBase::Week;
}

void KOAgendaView::updateEventIndicatorTop( int newY )
{
  uint i;
  for( i = 0; i < mMinY.size(); ++i ) {
    mEventIndicatorTop->enableColumn( i, newY >= mMinY[i] );
  }
  mEventIndicatorTop->update();
}

void KOAgendaView::updateEventIndicatorBottom( int newY )
{
  uint i;
  for( i = 0; i < mMaxY.size(); ++i ) {
    mEventIndicatorBottom->enableColumn( i, newY <= mMaxY[i] );
  }
  mEventIndicatorBottom->update();
}

void KOAgendaView::slotTodoDropped( Todo *todo, const TQPoint &gpos, bool allDay )
{
  if ( gpos.x()<0 || gpos.y()<0 ) return;
  TQDate day = mSelectedDates[gpos.x()];
  TQTime time = mAgenda->gyToTime( gpos.y() );
  TQDateTime newTime( day, time );

  if ( todo ) {
    Todo *existingTodo = calendar()->todo( todo->uid() );
    if ( existingTodo ) {
      kdDebug(5850) << "Drop existing Todo" << endl;
      Todo *oldTodo = existingTodo->clone();
      if ( mChanger && mChanger->beginChange( existingTodo ) ) {
        existingTodo->setDtDue( newTime );
        existingTodo->setFloats( allDay );
        existingTodo->setHasDueDate( true );
        mChanger->changeIncidence( oldTodo, existingTodo );
        mChanger->endChange( existingTodo );
      } else {
        KMessageBox::sorry( this, i18n("Unable to modify this to-do, "
                            "because it cannot be locked.") );
      }
      delete oldTodo;
    } else {
      kdDebug(5850) << "Drop new Todo" << endl;
      todo->setDtDue( newTime );
      todo->setFloats( allDay );
      todo->setHasDueDate( true );
      if ( !mChanger->addIncidence( todo, this ) ) {
        KODialogManager::errorSaveIncidence( this, todo );
      }
    }
  }
}

void KOAgendaView::startDrag( Incidence *incidence )
{
#ifndef KORG_NODND
  DndFactory factory( calendar() );
  ICalDrag *vd = factory.createDrag( incidence, this );
  if ( vd->drag() ) {
    kdDebug(5850) << "KOAgendaView::startDrag(): Delete drag source" << endl;
  }
#endif
}

void KOAgendaView::readSettings()
{
  readSettings(KOGlobals::self()->config());
}

void KOAgendaView::readSettings(KConfig *config)
{
//  kdDebug(5850) << "KOAgendaView::readSettings()" << endl;

  config->setGroup("Views");

#ifndef KORG_NOSPLITTER
  TQValueList<int> sizes = config->readIntListEntry("Separator AgendaView");
  if (sizes.count() == 2) {
    mSplitterAgenda->setSizes(sizes);
  }
#endif

  updateConfig();
}

void KOAgendaView::writeSettings(KConfig *config)
{
//  kdDebug(5850) << "KOAgendaView::writeSettings()" << endl;

  config->setGroup("Views");

#ifndef KORG_NOSPLITTER
  TQValueList<int> list = mSplitterAgenda->sizes();
  config->writeEntry("Separator AgendaView",list);
#endif
}

void KOAgendaView::setHolidayMasks()
{
  mHolidayMask.resize( mSelectedDates.count() + 1 );

  for( uint i = 0; i < mSelectedDates.count(); ++i ) {
    mHolidayMask[i] = !KOGlobals::self()->isWorkDay( mSelectedDates[ i ] );
  }

  // Store the information about the day before the visible area (needed for
  // overnight working hours) in the last bit of the mask:
  bool showDay = !KOGlobals::self()->isWorkDay( mSelectedDates[ 0 ].addDays( -1 ) );
  mHolidayMask[ mSelectedDates.count() ] = showDay;

  mAgenda->setHolidayMask( &mHolidayMask );
  mAllDayAgenda->setHolidayMask( &mHolidayMask );
}

void KOAgendaView::setContentsPos( int y )
{
  mAgenda->setContentsPos( 0, y );
}

void KOAgendaView::setExpandedButton( bool expanded )
{
  if ( !mExpandButton ) return;

  if ( expanded ) {
    mExpandButton->setPixmap( mExpandedPixmap );
  } else {
    mExpandButton->setPixmap( mNotExpandedPixmap );
  }
}

void KOAgendaView::clearSelection()
{
  mAgenda->deselectItem();
  mAllDayAgenda->deselectItem();
}

void KOAgendaView::newTimeSpanSelectedAllDay( const TQPoint &start, const TQPoint &end )
{
  newTimeSpanSelected( start, end );
  mTimeSpanInAllDay = true;
}

void KOAgendaView::newTimeSpanSelected( const TQPoint &start, const TQPoint &end )
{
  if (!mSelectedDates.count()) return;

  mTimeSpanInAllDay = false;

  TQDate dayStart = mSelectedDates[ kClamp( start.x(), 0, (int)mSelectedDates.size() - 1 ) ];
  TQDate dayEnd = mSelectedDates[ kClamp( end.x(), 0, (int)mSelectedDates.size() - 1 ) ];

  TQTime timeStart = mAgenda->gyToTime(start.y());
  TQTime timeEnd = mAgenda->gyToTime( end.y() + 1 );

  TQDateTime dtStart(dayStart,timeStart);
  TQDateTime dtEnd(dayEnd,timeEnd);

  mTimeSpanBegin = dtStart;
  mTimeSpanEnd = dtEnd;
}

void KOAgendaView::deleteSelectedDateTime()
{
  mTimeSpanBegin.setDate(TQDate());
  mTimeSpanEnd.setDate(TQDate());
  mTimeSpanInAllDay = false;
}

void KOAgendaView::setTypeAheadReceiver( TQObject *o )
{
  mAgenda->setTypeAheadReceiver( o );
  mAllDayAgenda->setTypeAheadReceiver( o );
}

void KOAgendaView::finishTypeAhead()
{
  mAgenda->finishTypeAhead();
  mAllDayAgenda->finishTypeAhead();
}

void KOAgendaView::removeIncidence( Incidence *incidence )
{
  mAgenda->removeIncidence( incidence );
  mAllDayAgenda->removeIncidence( incidence );
}

void KOAgendaView::updateEventIndicators()
{
  mMinY = mAgenda->minContentsY();
  mMaxY = mAgenda->maxContentsY();

  mAgenda->checkScrollBoundaries();
  updateEventIndicatorTop( mAgenda->visibleContentsYMin() );
  updateEventIndicatorBottom( mAgenda->visibleContentsYMax() );
}

void KOAgendaView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
  mAgenda->setIncidenceChanger( changer );
  mAllDayAgenda->setIncidenceChanger( changer );
}

void KOAgendaView::clearTimeSpanSelection()
{
  mAgenda->clearSelection();
  mAllDayAgenda->clearSelection();
  deleteSelectedDateTime();
}

void KOAgendaView::setResource(KCal::ResourceCalendar * res, const TQString & subResource)
{
  mResource = res;
  mSubResource = subResource;
}

bool KOAgendaView::filterByResource(Incidence * incidence)
{
  if ( !mResource )
    return true;
  CalendarResources *calRes = dynamic_cast<CalendarResources*>( calendar() );
  if ( !calRes )
    return true;
  if ( calRes->resource( incidence ) != mResource )
    return false;
  if ( !mSubResource.isEmpty() ) {
    if ( mResource->subresourceIdentifier( incidence ) != mSubResource )
      return false;
  }
  return true;
}

void KOAgendaView::resourcesChanged()
{
  mPendingChanges = true;
}

void KOAgendaView::calendarIncidenceAdded(Incidence * incidence)
{
  Q_UNUSED( incidence );
  mPendingChanges = true;
}

void KOAgendaView::calendarIncidenceChanged(Incidence * incidence)
{
  Q_UNUSED( incidence );
  mPendingChanges = true;
}

void KOAgendaView::calendarIncidenceRemoved(Incidence * incidence)
{
  Q_UNUSED( incidence );
  mPendingChanges = true;
}

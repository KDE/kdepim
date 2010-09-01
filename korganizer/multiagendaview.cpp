/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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
*/

#include "multiagendaview.h"

#include "koagendaview.h"
#include "koagenda.h"
#include "koprefs.h"
#include "timelabels.h"

#include <libkcal/calendarresources.h>

#include <kglobalsettings.h>

#include <tqlayout.h>
#include <tqvbox.h>
#include <tqobjectlist.h>
#include <tqheader.h>

#define FOREACH_VIEW(av) \
for(TQValueList<KOAgendaView*>::ConstIterator it = mAgendaViews.constBegin(); \
  it != mAgendaViews.constEnd();) \
  for(KOAgendaView* av = (it != mAgendaViews.constEnd() ? (*it) : 0); \
      it != mAgendaViews.constEnd(); ++it, av = (*it)  )

using namespace KOrg;

MultiAgendaView::MultiAgendaView( Calendar * cal, CalendarView *calendarView,
                                 TQWidget * parent, const char *name ) :
    AgendaView( cal, parent, name ),
    mSelectedAgendaView( 0 ),
    mLastMovedSplitter( 0 ),
    mUpdateOnShow( false ),
    mPendingChanges( true ),
    mCalendarView( calendarView )
{
  TQBoxLayout *topLevelLayout = new TQHBoxLayout( this );

  TQFontMetrics fm( font() );
  int topLabelHeight = 2 * fm.height() + fm.lineSpacing();

  TQVBox *topSideBox = new TQVBox( this );
  mLeftTopSpacer = new TQWidget( topSideBox );
  mLeftTopSpacer->setFixedHeight( topLabelHeight );
  mLeftSplitter = new TQSplitter( Qt::Vertical, topSideBox );
  mLeftSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );
  TQLabel *label = new TQLabel( i18n("All Day"), mLeftSplitter );
  label->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::WordBreak );
  TQVBox *sideBox = new TQVBox( mLeftSplitter );
  EventIndicator *eiSpacer = new EventIndicator( EventIndicator::Top, sideBox );
  eiSpacer->changeColumns( 0 );
  mTimeLabels = new TimeLabels( 24, sideBox );
  eiSpacer = new EventIndicator( EventIndicator::Bottom, sideBox );
  eiSpacer->changeColumns( 0 );
  mLeftBottomSpacer = new TQWidget( topSideBox );
  topLevelLayout->addWidget( topSideBox );

  mScrollView = new TQScrollView( this );
  mScrollView->setResizePolicy( TQScrollView::Manual );
  mScrollView->setVScrollBarMode( TQScrollView::AlwaysOff );
  mScrollView->setFrameShape( TQFrame::NoFrame );
  topLevelLayout->addWidget( mScrollView, 100 );
  mTopBox = new TQHBox( mScrollView->viewport() );
  mScrollView->addChild( mTopBox );

  topSideBox = new TQVBox( this );
  mRightTopSpacer = new TQWidget( topSideBox );
  mRightTopSpacer->setFixedHeight( topLabelHeight );
  mRightSplitter = new TQSplitter( Qt::Vertical, topSideBox );
  mRightSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );
  new TQWidget( mRightSplitter );
  sideBox = new TQVBox( mRightSplitter );
  eiSpacer = new EventIndicator( EventIndicator::Top, sideBox );
  eiSpacer->setFixedHeight( eiSpacer->minimumHeight() );
  eiSpacer->changeColumns( 0 );
  mScrollBar = new TQScrollBar( Qt::Vertical, sideBox );
  eiSpacer = new EventIndicator( EventIndicator::Bottom, sideBox );
  eiSpacer->setFixedHeight( eiSpacer->minimumHeight() );
  eiSpacer->changeColumns( 0 );
  mRightBottomSpacer = new TQWidget( topSideBox );
  topLevelLayout->addWidget( topSideBox );

  recreateViews();
}

void MultiAgendaView::recreateViews()
{
  if ( !mPendingChanges ) {
    return;
  }

  mPendingChanges = false;

  deleteViews();

  CalendarResources *calres = dynamic_cast<CalendarResources*>( calendar() );
  if ( !calres ) {
    // fallback to single-agenda
    KOAgendaView* av = new KOAgendaView( calendar(), mCalendarView, mTopBox );
    mAgendaViews.append( av );
    mAgendaWidgets.append( av );
    mSelectedAgendaView = av;
    av->show();
  } else {
    CalendarResourceManager *manager = calres->resourceManager();
    for ( CalendarResourceManager::ActiveIterator it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
      if ( (*it)->canHaveSubresources() ) {
        TQStringList subResources = (*it)->subresources();
        for ( TQStringList::ConstIterator subit = subResources.constBegin(); subit != subResources.constEnd(); ++subit ) {
          TQString type = (*it)->subresourceType( *subit );

          if ( !(*it)->subresourceActive( *subit ) || (!type.isEmpty() && type != "event") ) {
            continue;
          }

          addView( (*it)->labelForSubresource( *subit ), *it, *subit );
        }
      } else {
        addView( (*it)->resourceName(), *it );
      }
    }
  }

  // no resources activated, so stop here to avoid crashing somewhere down the line, TODO: show a nice message instead
  if ( mAgendaViews.isEmpty() ) {
    return;
  }

  setupViews();
  TQTimer::singleShot( 0, this, TQT_SLOT(slotResizeScrollView()) );
  mTimeLabels->updateConfig();

  connect( mTimeLabels->verticalScrollBar(), TQT_SIGNAL(valueChanged(int)),
           mScrollBar, TQT_SLOT(setValue(int)) );
  connect( mScrollBar, TQT_SIGNAL(valueChanged(int)),
           mTimeLabels, TQT_SLOT(positionChanged(int)) );

  installSplitterEventFilter( mLeftSplitter );
  installSplitterEventFilter( mRightSplitter );

  TQValueList<int> sizes = KOGlobals::self()->config()->readIntListEntry( "Separator AgendaView" );
  if ( sizes.count() != 2 ) {
    sizes = mLeftSplitter->sizes();
  }
  FOREACH_VIEW( agenda ) {
    agenda->splitter()->setSizes( sizes );
  }
  mLeftSplitter->setSizes( sizes );
  mRightSplitter->setSizes( sizes );

  TQTimer::singleShot( 0, this, TQT_SLOT(setupScrollBar()) );

  mTimeLabels->positionChanged();
}

void MultiAgendaView::deleteViews()
{
  for ( TQValueList<TQWidget*>::ConstIterator it = mAgendaWidgets.constBegin();
        it != mAgendaWidgets.constEnd(); ++it ) {
    delete *it;
  }
  mAgendaViews.clear();
  mAgendaWidgets.clear();
  mLastMovedSplitter = 0;
  mSelectedAgendaView = 0;
}

void MultiAgendaView::setupViews()
{
  FOREACH_VIEW( agenda ) {
    if ( !agenda->readOnly() ) {
      connect( agenda,
               TQT_SIGNAL(newEventSignal(ResourceCalendar *,const TQString &)),
               TQT_SIGNAL(newEventSignal(ResourceCalendar *,const TQString &)) );
      connect( agenda,
               TQT_SIGNAL(newEventSignal(ResourceCalendar *,const TQString &,const TQDate &)),
               TQT_SIGNAL(newEventSignal(ResourceCalendar *,const TQString &,const TQDate &)) );
      connect( agenda,
               TQT_SIGNAL(newEventSignal(ResourceCalendar *,const TQString &,const TQDateTime &)),
               TQT_SIGNAL(newEventSignal(ResourceCalendar *,const TQString &,const TQDateTime &)) );
      connect( agenda,
               TQT_SIGNAL(newEventSignal(ResourceCalendar *,const TQString &,const TQDateTime &,const TQDateTime &)),
               TQT_SIGNAL(newEventSignal(ResourceCalendar *,const TQString &,const TQDateTime &,const TQDateTime&)) );

      connect( agenda,
               TQT_SIGNAL(newTodoSignal(ResourceCalendar *,const TQString &,const TQDate &)),
               TQT_SIGNAL(newTodoSignal(ResourceCalendar *,const TQString &,const TQDate &)) );

      connect( agenda,
               TQT_SIGNAL(editIncidenceSignal(Incidence *,const TQDate &)),
               TQT_SIGNAL(editIncidenceSignal(Incidence *,const TQDate &)) );
      connect( agenda,
               TQT_SIGNAL(deleteIncidenceSignal(Incidence *)),
               TQT_SIGNAL(deleteIncidenceSignal(Incidence *)) );
      connect( agenda,
               TQT_SIGNAL(startMultiModify(const TQString &)),
               TQT_SIGNAL(startMultiModify(const TQString &)) );
      connect( agenda,
               TQT_SIGNAL(endMultiModify()),
               TQT_SIGNAL(endMultiModify()) );

      connect( agenda,
               TQT_SIGNAL(cutIncidenceSignal(Incidence*)),
               TQT_SIGNAL(cutIncidenceSignal(Incidence*)) );
      connect( agenda,
               TQT_SIGNAL(pasteIncidenceSignal()),
               TQT_SIGNAL(pasteIncidenceSignal()) );
      connect( agenda,
               TQT_SIGNAL(toggleAlarmSignal(Incidence*)),
               TQT_SIGNAL(toggleAlarmSignal(Incidence*)) );
      connect( agenda,
               TQT_SIGNAL(dissociateOccurrenceSignal(Incidence*, const TQDate&)),
               TQT_SIGNAL(dissociateOccurrenceSignal(Incidence*, const TQDate&)) );
      connect( agenda,
               TQT_SIGNAL(dissociateFutureOccurrenceSignal(Incidence*, const TQDate&)),
               TQT_SIGNAL(dissociateFutureOccurrenceSignal(Incidence*, const TQDate&)) );
    }

    connect( agenda,
             TQT_SIGNAL(copyIncidenceSignal(Incidence*)),
             TQT_SIGNAL(copyIncidenceSignal(Incidence*)) );
    connect( agenda,
             TQT_SIGNAL(showIncidenceSignal(Incidence *,const TQDate &)),
             TQT_SIGNAL(showIncidenceSignal(Incidence *,const TQDate &)) );
    connect( agenda,
             TQT_SIGNAL(incidenceSelected(Incidence *,const TQDate &)),
             TQT_SIGNAL(incidenceSelected(Incidence *,const TQDate &)) );
    connect( agenda,
             TQT_SIGNAL(incidenceSelected(Incidence*,const TQDate &)),
             TQT_SLOT(slotSelectionChanged()) );

    connect( agenda,
             TQT_SIGNAL(timeSpanSelectionChanged()),
             TQT_SLOT(slotClearTimeSpanSelection()) );

    disconnect( agenda->agenda(),
                TQT_SIGNAL(zoomView(const int,const TQPoint&,const Qt::Orientation)),
                agenda, 0 );
    connect( agenda->agenda(),
             TQT_SIGNAL(zoomView(const int,const TQPoint&,const Qt::Orientation)),
             TQT_SLOT(zoomView(const int,const TQPoint&,const Qt::Orientation)) );
  }

  KOAgenda *anAgenda = mAgendaViews.first()->agenda();
  connect( anAgenda, TQT_SIGNAL(lowerYChanged(int) ), TQT_SLOT(resizeSpacers(int)) );

  FOREACH_VIEW( agenda ) {
    agenda->readSettings();
  }

  int minWidth = 0;
  for ( TQValueList<TQWidget*>::ConstIterator it = mAgendaWidgets.constBegin(); it != mAgendaWidgets.constEnd(); ++it )
    minWidth = QMAX( minWidth, (*it)->minimumSizeHint().width() );
  for ( TQValueList<TQWidget*>::ConstIterator it = mAgendaWidgets.constBegin(); it != mAgendaWidgets.constEnd(); ++it )
    (*it)->setMinimumWidth( minWidth );
}

MultiAgendaView::~ MultiAgendaView()
{
}

Incidence::List MultiAgendaView::selectedIncidences()
{
  Incidence::List list;
  FOREACH_VIEW(agendaView) {
    list += agendaView->selectedIncidences();
  }
  return list;
}

DateList MultiAgendaView::selectedIncidenceDates()
{
  DateList list;
  FOREACH_VIEW(agendaView) {
    list += agendaView->selectedIncidenceDates();
  }
  return list;
}

int MultiAgendaView::currentDateCount()
{
  FOREACH_VIEW( agendaView )
    return agendaView->currentDateCount();
  return 0;
}

void MultiAgendaView::showDates(const TQDate & start, const TQDate & end)
{
  mStartDate = start;
  mEndDate = end;
  recreateViews();
  FOREACH_VIEW( agendaView )
    agendaView->showDates( start, end );
}

void MultiAgendaView::showIncidences(const Incidence::List & incidenceList, const TQDate &date)
{
  FOREACH_VIEW( agendaView )
    agendaView->showIncidences( incidenceList, date );
}

void MultiAgendaView::updateView()
{
  recreateViews();
  FOREACH_VIEW( agendaView )
    agendaView->updateView();
}

void MultiAgendaView::changeIncidenceDisplay(Incidence * incidence, int mode)
{
  FOREACH_VIEW( agendaView )
    agendaView->changeIncidenceDisplay( incidence, mode );
}

int MultiAgendaView::maxDatesHint()
{
  FOREACH_VIEW( agendaView )
    return agendaView->maxDatesHint();
  return 0;
}

void MultiAgendaView::slotSelectionChanged()
{
  FOREACH_VIEW( agenda ) {
    if ( agenda != sender() )
      agenda->clearSelection();
  }
}

bool MultiAgendaView::eventDurationHint(TQDateTime & startDt, TQDateTime & endDt, bool & allDay)
{
  FOREACH_VIEW( agenda ) {
    bool valid = agenda->eventDurationHint( startDt, endDt, allDay );
    if ( valid )
      return true;
  }
  return false;
}

void MultiAgendaView::slotClearTimeSpanSelection()
{
  FOREACH_VIEW( agenda ) {
    if ( agenda != sender() )
      agenda->clearTimeSpanSelection();
  }
}

void MultiAgendaView::setTypeAheadReceiver(TQObject * o)
{
  FOREACH_VIEW( agenda )
    agenda->setTypeAheadReceiver( o );
}

void MultiAgendaView::finishTypeAhead()
{
  FOREACH_VIEW( agenda )
    agenda->finishTypeAhead();
}

void MultiAgendaView::addView( const TQString &label, ResourceCalendar *res, const TQString &subRes )
{
  bool readOnlyView = false;

  TQVBox *box = new TQVBox( mTopBox );

  // First, the calendar folder title
  TQHeader *title = new TQHeader( 1, box );
  title->setClickEnabled( false );
  title->setStretchEnabled( true );
  if ( res->readOnly() || !res->subresourceWritable( subRes ) ) {
    readOnlyView = true;
    title->setLabel( 0, TQIconSet( KOGlobals::self()->smallIcon( "readonlyevent" ) ), label );
  } else {
    TQColor resColor;
    if ( subRes.isEmpty() ) {
      resColor = *KOPrefs::instance()->resourceColor( res->identifier() );
    } else {
      resColor = *KOPrefs::instance()->resourceColor( subRes );
    }
    TQFontMetrics fm = fontMetrics();
    TQPixmap px( fm.height(), fm.height() );
    px.fill( resColor );
    title->setLabel( 0, TQIconSet( px, TQIconSet::Small ), label );
  }

  // Now, the sub agenda view
  KOAgendaView* av = new KOAgendaView( calendar(), mCalendarView, box, 0, true );
  av->setReadOnly( readOnlyView );
  av->setResource( res, subRes );
  av->setIncidenceChanger( mChanger );
  av->agenda()->setVScrollBarMode( TQScrollView::AlwaysOff );
  mAgendaViews.append( av );
  mAgendaWidgets.append( box );
  box->show();
  mTimeLabels->setAgenda( av->agenda() );

  connect( av->agenda()->verticalScrollBar(), TQT_SIGNAL(valueChanged(int)),
           mTimeLabels, TQT_SLOT(positionChanged(int)) );
  connect( mTimeLabels->verticalScrollBar(), TQT_SIGNAL(valueChanged(int)),
           av, TQT_SLOT(setContentsPos(int)) );

  av->installEventFilter( this );
  installSplitterEventFilter( av->splitter() );
}

void MultiAgendaView::resizeEvent(TQResizeEvent * ev)
{
  resizeScrollView( ev->size() );
  AgendaView::resizeEvent( ev );
}

void MultiAgendaView::resizeScrollView(const TQSize & size)
{
  const int widgetWidth = size.width() - mTimeLabels->width() - mScrollBar->width();
  int width = QMAX( mTopBox->sizeHint().width(), widgetWidth );
  int height = size.height();
  if ( width > widgetWidth ) {
    const int sbHeight = mScrollView->horizontalScrollBar()->height();
    height -= sbHeight;
    mLeftBottomSpacer->setFixedHeight( sbHeight );
    mRightBottomSpacer->setFixedHeight( sbHeight );
  } else {
    mLeftBottomSpacer->setFixedHeight( 0 );
    mRightBottomSpacer->setFixedHeight( 0 );
  }
  mScrollView->resizeContents( width, height );
  mTopBox->resize( width, height );
}

void MultiAgendaView::setIncidenceChanger(IncidenceChangerBase * changer)
{
  AgendaView::setIncidenceChanger( changer );
  FOREACH_VIEW( agenda )
    agenda->setIncidenceChanger( changer );
}

void MultiAgendaView::updateConfig()
{
  AgendaView::updateConfig();
  mTimeLabels->updateConfig();
  FOREACH_VIEW( agenda )
    agenda->updateConfig();
}

bool MultiAgendaView::eventFilter(TQObject * obj, TQEvent * event)
{
  if ( obj->className() == TQCString("QSplitterHandle") ) {
    // KDE4: not needed anymore, TQSplitter has a moved signal there
    if ( (event->type() == TQEvent::MouseMove && KGlobalSettings::opaqueResize())
           || event->type() == TQEvent::MouseButtonRelease ) {
      FOREACH_VIEW( agenda ) {
        if ( agenda->splitter() == obj->parent() )
          mLastMovedSplitter = agenda->splitter();
      }
      if ( mLeftSplitter == obj->parent() )
        mLastMovedSplitter = mLeftSplitter;
      else if ( mRightSplitter == obj->parent() )
        mLastMovedSplitter = mRightSplitter;
      TQTimer::singleShot( 0, this, TQT_SLOT(resizeSplitters()) );
    }
  }

  if ( obj->className() == TQCString( "KOAgendaView" ) ) {
    if ( event->type() == TQEvent::MouseButtonRelease ||
         event->type() == TQEvent::MouseButtonPress ) {
      mSelectedAgendaView = (KOAgendaView *)obj;
    }
  }

  return AgendaView::eventFilter( obj, event );
}

KOAgendaView *MultiAgendaView::selectedAgendaView()
{
  return mSelectedAgendaView;
}

void MultiAgendaView::resizeSplitters()
{
  if ( !mLastMovedSplitter )
    mLastMovedSplitter = mAgendaViews.first()->splitter();
  FOREACH_VIEW( agenda ) {
    if ( agenda->splitter() == mLastMovedSplitter )
      continue;
    agenda->splitter()->setSizes( mLastMovedSplitter->sizes() );
  }
  if ( mLastMovedSplitter != mLeftSplitter )
    mLeftSplitter->setSizes( mLastMovedSplitter->sizes() );
  if ( mLastMovedSplitter != mRightSplitter )
    mRightSplitter->setSizes( mLastMovedSplitter->sizes() );
}

void MultiAgendaView::resizeSpacers( int newY )
{
  // this slot is needed because the Agenda view's day labels frame height
  // can change depending if holidays are shown. When this happens, all
  // the widgets move down except the timelabels, so we need to change
  // the top spacer height accordingly to move the timelabels up/down.
  // kolab/issue2656
  Q_UNUSED( newY );
  TQFontMetrics fm( font() );
  int topLabelHeight = mAgendaViews.first()->dayLabels()->height() +
                       fm.height() + mLeftSplitter->handleWidth();
  mLeftTopSpacer->setFixedHeight( topLabelHeight );
  mRightTopSpacer->setFixedHeight( topLabelHeight );
}

void MultiAgendaView::zoomView( const int delta, const TQPoint & pos, const Qt::Orientation ori )
{
  if ( ori == Qt::Vertical ) {
    if ( delta > 0 ) {
      if ( KOPrefs::instance()->mHourSize > 4 )
        KOPrefs::instance()->mHourSize--;
    } else {
      KOPrefs::instance()->mHourSize++;
    }
  }

  FOREACH_VIEW( agenda )
    agenda->zoomView( delta, pos, ori );

  mTimeLabels->updateConfig();
  mTimeLabels->positionChanged();
  mTimeLabels->repaint();
}

// KDE4: not needed, use existing TQSplitter signals instead
void MultiAgendaView::installSplitterEventFilter(TQSplitter * splitter)
{
  TQObjectList *objlist = splitter->queryList( "QSplitterHandle" );
  // HACK: when not being visible, the splitter handle is sometimes not found
  // for unknown reasons, so trigger an update when we are shown again
  if ( objlist->count() == 0 && !isVisible() )
    mUpdateOnShow = true;
  TQObjectListIt it( *objlist );
  TQObject *obj;
  while ( (obj = it.current()) != 0 ) {
    obj->removeEventFilter( this );
    obj->installEventFilter( this );
    ++it;
  }
  delete objlist;
}

void MultiAgendaView::slotResizeScrollView()
{
  resizeScrollView( size() );
}

void MultiAgendaView::show()
{
  AgendaView::show();
  if ( mUpdateOnShow ) {
    mUpdateOnShow = false;
    mPendingChanges = true; // force a full view recreation
    showDates( mStartDate, mEndDate );
  }
}

void MultiAgendaView::resourcesChanged()
{
  mPendingChanges = true;

  kdDebug() << "mAgendaViews.size is " << mAgendaViews.size()
            << "; mAgendaWidgets.size is " << mAgendaWidgets.size()
            << "; mSelectedAgendaView is " << mSelectedAgendaView
            << endl;

  if ( mSelectedAgendaView ) {
    ResourceCalendar *res = mSelectedAgendaView->resourceCalendar();
    if ( res ) {
      if ( res->canHaveSubresources() ) {
        TQString subRes = mSelectedAgendaView->subResourceCalendar();
        if ( !res->subresourceWritable( subRes ) ||
             !res->subresourceActive( subRes ) ) {
          mSelectedAgendaView = 0;
        }
      } else {
        if ( res->readOnly() || !res->isActive() ) {
          mSelectedAgendaView = 0;
        }
      }
    } else {
      mSelectedAgendaView = 0;
    }
  }

  FOREACH_VIEW( agenda )
    agenda->resourcesChanged();
}

void MultiAgendaView::setupScrollBar()
{
  if ( !mAgendaViews.isEmpty() && mAgendaViews.first()->agenda() ) {
    TQScrollBar *scrollBar = mAgendaViews.first()->agenda()->verticalScrollBar();
    mScrollBar->setMinValue( scrollBar->minValue() );
    mScrollBar->setMaxValue( scrollBar->maxValue() );
    mScrollBar->setLineStep( scrollBar->lineStep() );
    mScrollBar->setPageStep( scrollBar->pageStep() );
    mScrollBar->setValue( scrollBar->value() );
  }
}

#include "multiagendaview.moc"

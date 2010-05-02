/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
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

#include "eventview.h"

#include "prefs.h"

#include "recurrenceactions.h"

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/calendarsearch.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/kcal/collectionselectionproxymodel.h>
#include <akonadi/kcal/incidencechanger.h>
#include <akonadi/kcal/utils.h>
#include <akonadi/akonadi_next/entitymodelstatesaver.h>

#include <kholidays/holidayregion.h>
#include <KCal/Incidence>
#include <KLocale>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>
#include <KRandom>
#include <KGuiItem>

#include <QMenu>
#include <QApplication>
#include <QKeyEvent>

using namespace Akonadi;
using namespace EventViews;

class EventView::Private
{
  EventView *const q;

  public:
    explicit Private( EventView* qq )
      : q( qq ),
        calendar( 0 ),
        customCollectionSelection( 0 ),
        collectionSelectionModel( 0 ),
        stateSaver( 0 ),
        mReturnPressed( false ),
        mTypeAhead( false ),
        mTypeAheadReceiver( 0 ),
        mPrefs( new Prefs() ),
        mChanger( 0 )
    {
      QByteArray cname = q->metaObject()->className();
      cname.replace( ":", "_" );
      identifier = cname + "_" + KRandom::randomString( 8 ).toLatin1();
      calendarSearch = new CalendarSearch( q );
      connect( calendarSearch->model(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
               q, SLOT( rowsInserted( const QModelIndex&, int, int ) ) );
      connect( calendarSearch->model(), SIGNAL( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ),
               q, SLOT( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ) );
      connect( calendarSearch->model(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
               q, SLOT( dataChanged( const QModelIndex&, const QModelIndex& ) ) );
      connect( calendarSearch->model(), SIGNAL( modelReset() ), q, SLOT( calendarReset() ) );
    }

    ~Private()
    {
      delete collectionSelectionModel;
    }

    void setUpModels();
    void reconnectCollectionSelection();

  public:
    Akonadi::Calendar *calendar;
    CalendarSearch *calendarSearch;
    CollectionSelection *customCollectionSelection;
    CollectionSelectionProxyModel* collectionSelectionModel;
    EntityModelStateSaver* stateSaver;
    QByteArray identifier;
    KDateTime startDateTime;
    KDateTime endDateTime;
    KDateTime actualStartDateTime;
    KDateTime actualEndDateTime;

    /* When we receive a QEvent with a key_Return release
     * we will only show a new event dialog if we previously received a
     * key_Return press, otherwise a new event dialog appears when
     * you hit return in some yes/no dialog */
    bool mReturnPressed;

    bool mTypeAhead;
    QObject *mTypeAheadReceiver;
    QList<QEvent*> mTypeAheadEvents;
    static Akonadi::CollectionSelection* sGlobalCollectionSelection;

    KHolidays::HolidayRegionPtr mHolidayRegion;
    PrefsPtr mPrefs;

    IncidenceChanger *mChanger;
};

CollectionSelection* EventView::Private::sGlobalCollectionSelection = 0;

/* static */
void EventView::setGlobalCollectionSelection( CollectionSelection* s )
{
  Private::sGlobalCollectionSelection = s;
}

void EventView::Private::setUpModels()
{
  delete stateSaver;
  stateSaver = 0;
  delete customCollectionSelection;
  customCollectionSelection = 0;
  if ( collectionSelectionModel ) {
    customCollectionSelection = new CollectionSelection( collectionSelectionModel->selectionModel() );
    stateSaver = new EntityModelStateSaver( collectionSelectionModel, q );
    stateSaver->addRole( Qt::CheckStateRole, "CheckState" );
    calendarSearch->setSelectionModel( collectionSelectionModel->selectionModel() );

  } else {
    calendarSearch->setSelectionModel( globalCollectionSelection()->model() );
  }
#if 0
  QDialog* dlg = new QDialog( q );
  dlg->setModal( false );
  QVBoxLayout* layout = new QVBoxLayout( dlg );
  EntityTreeView* testview = new EntityTreeView( dlg );
  layout->addWidget( testview );
  testview->setModel( calendarSearch->model() );
  dlg->show();
#endif
  reconnectCollectionSelection();
}

void EventView::Private::reconnectCollectionSelection()
{
  if ( q->globalCollectionSelection() ) {
    q->globalCollectionSelection()->disconnect( q );
  }

  if ( customCollectionSelection ) {
    customCollectionSelection->disconnect( q );
  }

  QObject::connect( q->collectionSelection(),
                    SIGNAL(selectionChanged(Akonadi::Collection::List,Akonadi::Collection::List)),
                    q, SLOT(collectionSelectionChanged()) );
}


EventView::EventView( QWidget *parent ) : QWidget( parent ), d( new Private( this ) )
{

  //AKONADI_PORT review: the FocusLineEdit in the editor emits focusReceivedSignal(), which triggered finishTypeAhead.
  //But the global focus widget in QApplication is changed later, thus subsequent keyevents still went to this view, triggering another editor, for each keypress
  //Thus listen to the global focusChanged() signal (seen with Qt 4.6-stable-patched 20091112 -Frank)
  connect( QApplication::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focusChanged(QWidget*,QWidget*)) );
}

EventView::~EventView()
{
  delete d;
}

void EventView::defaultAction( const Item &aitem )
{
  kDebug();
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );
  if ( !incidence ) {
    return;
  }

  kDebug() << "  type:" << incidence->type();

  if ( incidence->isReadOnly() ) {
    emit showIncidenceSignal(aitem);
  } else {
    emit editIncidenceSignal(aitem);
  }
}

void EventView::setHolidayRegion( const KHolidays::HolidayRegionPtr &holidayRegion )
{
  d->mHolidayRegion = holidayRegion;
}

int EventView::showMoveRecurDialog( const Item &aitem, const QDate &date )
{
  const Incidence::Ptr inc = Akonadi::incidence( aitem );

  KDateTime dateTime( date, preferences()->timeSpec() );

  int availableOccurrences = RecurrenceActions::availableOccurrences( inc, dateTime );

  const QString caption = i18nc( "@title:window", "Changing Recurring Item" );
  KGuiItem itemFuture( i18n( "Also &Future Items" ) );
  KGuiItem itemSelected( i18n( "Only &This Item" ) );
  KGuiItem itemAll( i18n( "&All Occurrences" ) );

  switch ( availableOccurrences ) {
    case RecurrenceActions::NoOccurrence:
        return RecurrenceActions::NoOccurrence;
    case RecurrenceActions::SelectedOccurrence:
      return RecurrenceActions::SelectedOccurrence;

    default:
      if ( availableOccurrences & RecurrenceActions::FutureOccurrences ) {
        const QString message = i18n( "The item you are trying to change is a recurring item. "
                                      "Should the changes be applied only to this single occurrence, "
                                      "also to future items, or to all items in the recurrence?" );
        return RecurrenceActions::questionSelectedFutureAllCancel( message, caption, itemSelected, itemFuture, itemAll, this );
      } else {
        const QString message = i18n( "The item you are trying to change is a recurring item. "
                                      "Should the changes be applied only to this single occurrence "
                                      "or to all items in the recurrence?" );
        return RecurrenceActions::questionSelectedAllCancel( message, caption, itemSelected, itemAll, this );
      }
      break;
  }

  return RecurrenceActions::NoOccurrence;
}

void EventView::setCalendar( Akonadi::Calendar *cal )
{
  if ( d->calendar != cal ) {
    d->calendar = cal;
    if ( cal && d->collectionSelectionModel ) {
      d->collectionSelectionModel->setSourceModel( cal->model() );
    }
  }
}

Akonadi::Calendar *EventView::calendar() const
{
  return d->calendar;
}

void EventView::setPreferences( const PrefsPtr &preferences )
{
  if ( d->mPrefs != preferences ) {
    if ( preferences ) {
        d->mPrefs = preferences;
    } else {
        d->mPrefs = PrefsPtr( new Prefs() );
    }
    updateConfig();
  }
}

PrefsPtr EventView::preferences() const
{
  return d->mPrefs;
}

Akonadi::CalendarSearch* EventView::calendarSearch() const
{
  return d->calendarSearch;
}

void EventView::dayPassed( const QDate & )
{
  updateView();
}

void EventView::setIncidenceChanger( IncidenceChanger *changer )
{
  d->mChanger = changer;
}

void EventView::flushView()
{}

EventView* EventView::viewAt( const QPoint & )
{
  return this;
}

void EventView::updateConfig()
{
}

QDateTime EventView::selectionStart() const
{
  return QDateTime();
}

QDateTime EventView::selectionEnd() const
{
  return QDateTime();
}

bool EventView::supportsZoom() const
{
  return false;
}

bool EventView::hasConfigurationDialog() const
{
  return false;
}

void EventView::setDateRange( const KDateTime &start, const KDateTime &end )
{
#if 0 //AKONADI_PORT the old code called showDates() (below), which triggers a repaint, which the old code relies on
  if ( d->startDateTime == start && d->endDateTime == end ) {
    return;
  }
#endif
  d->startDateTime = start;
  d->endDateTime = end;
  showDates( start.date(), end.date() );
  const QPair<KDateTime,KDateTime> adjusted = actualDateRange( start, end );
  d->actualStartDateTime = adjusted.first;
  d->actualEndDateTime = adjusted.second;
  d->calendarSearch->setStartDate( d->actualStartDateTime );
  d->calendarSearch->setEndDate( d->actualEndDateTime );
}

KDateTime EventView::startDateTime() const
{
  return d->startDateTime;
}

KDateTime EventView::endDateTime() const
{
  return d->endDateTime;
}

KDateTime EventView::actualStartDateTime() const
{
  return d->actualStartDateTime;
}

KDateTime EventView::actualEndDateTime() const
{
  return d->actualEndDateTime;
}

void EventView::showConfigurationDialog( QWidget* )
{
}


bool EventView::processKeyEvent( QKeyEvent *ke )
{ // TODO_SPLIT: review this

  // If Return is pressed bring up an editor for the current selected time span.
/*  if ( ke->key() == Qt::Key_Return ) {
    if ( ke->type() == QEvent::KeyPress ) {
      mReturnPressed = true;
    } else if ( ke->type() == QEvent::KeyRelease ) {
      if ( mReturnPressed ) {
        // TODO(AKONADI_PORT) Remove this hack when the calendarview is ported to CalendarSearch
        if ( AgendaView *view = dynamic_cast<AgendaView*>( this ) ) {
          if ( view->collection() >= 0 ) {
            emit newEventSignal( Akonadi::Collection::List() << Collection( view->collection() ) );
          } else {
            emit newEventSignal( collectionSelection()->selectedCollections() );
          }
        } else {
          emit newEventSignal( collectionSelection()->selectedCollections() );
        }
        mReturnPressed = false;
        return true;
      } else {
        mReturnPressed = false;
      }
    }
  }

  // Ignore all input that does not produce any output
  if ( ke->text().isEmpty() || ( ke->modifiers() & Qt::ControlModifier ) ) {
    return false;
  }

  if ( ke->type() == QEvent::KeyPress ) {
    switch ( ke->key() ) {
    case Qt::Key_Escape:
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_Control:
    case Qt::Key_Meta:
    case Qt::Key_Alt:
      break;
    default:
      mTypeAheadEvents.append(
        new QKeyEvent( ke->type(),
                       ke->key(),
                       ke->modifiers(),
                       ke->text(),
                       ke->isAutoRepeat(),
                       static_cast<ushort>( ke->count() ) ) );
      if ( !mTypeAhead ) {
        mTypeAhead = true;
        // TODO(AKONADI_PORT) Remove this hack when the calendarview is ported to CalendarSearch
        if ( AgendaView *view = dynamic_cast<AgendaView*>( this ) ) {
          if ( view->collection() >= 0 ) {
            emit newEventSignal( Akonadi::Collection::List() << Collection( view->collection() ) );
          } else {
            emit newEventSignal( collectionSelection()->selectedCollections() );
          }
        } else {
          emit newEventSignal( collectionSelection()->selectedCollections() );
        }
      }
      return true;
    }
    } */
  return false;
}

void EventView::setTypeAheadReceiver( QObject *o )
{
  d->mTypeAheadReceiver = o;
}

void EventView::focusChanged( QWidget*, QWidget* now )
{
  if ( d->mTypeAhead && now && now == d->mTypeAheadReceiver )
    finishTypeAhead();
}

void EventView::finishTypeAhead()
{
  if ( d->mTypeAheadReceiver ) {
    foreach ( QEvent *e, d->mTypeAheadEvents ) {
      QApplication::sendEvent( d->mTypeAheadReceiver, e );
    }
  }
  qDeleteAll( d->mTypeAheadEvents );
  d->mTypeAheadEvents.clear();
  d->mTypeAhead = false;
}

CollectionSelection* EventView::collectionSelection() const
{
  return d->customCollectionSelection ? d->customCollectionSelection : globalCollectionSelection();
}

void EventView::setCustomCollectionSelectionProxyModel( Akonadi::CollectionSelectionProxyModel* model )
{
  if ( d->collectionSelectionModel == model )
    return;

  delete d->collectionSelectionModel;
  d->collectionSelectionModel = model;
  d->setUpModels();
}

void EventView::collectionSelectionChanged()
{

}

CollectionSelectionProxyModel *EventView::customCollectionSelectionProxyModel() const
{
  return d->collectionSelectionModel;
}

CollectionSelectionProxyModel *EventView::takeCustomCollectionSelectionProxyModel()
{
  CollectionSelectionProxyModel* m = d->collectionSelectionModel;
  d->collectionSelectionModel = 0;
  d->setUpModels();
  return m;
}

CollectionSelection *EventView::customCollectionSelection() const
{
  return d->customCollectionSelection;
}

void EventView::clearSelection()
{
}

bool EventView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay ) const
{
  Q_UNUSED( startDt );
  Q_UNUSED( endDt );
  Q_UNUSED( allDay );
  return false;
}

Akonadi::IncidenceChanger *EventView::changer() const
{
  return d->mChanger;
}

void EventView::doRestoreConfig( const KConfigGroup & )
{
}

void EventView::doSaveConfig( KConfigGroup & )
{
}

QPair<KDateTime,KDateTime> EventView::actualDateRange( const KDateTime& start, const KDateTime& end ) const
{
  return qMakePair( start, end );
}

void EventView::incidencesAdded( const Akonadi::Item::List & )
{
}

void EventView::incidencesAboutToBeRemoved( const Akonadi::Item::List & )
{
}

void EventView::incidencesChanged( const Akonadi::Item::List& )
{
}

void EventView::handleBackendError( const QString &errorString )
{
  kError() << errorString;
}

bool EventView::isWorkDay( const QDate &date ) const
{
  int mask( ~( preferences()->workWeekMask() ) );

  bool nonWorkDay = ( mask & ( 1 << ( date.dayOfWeek() - 1 ) ) );
  if ( preferences()->excludeHolidays() && d->mHolidayRegion ) {
    const KHolidays::Holiday::List list = d->mHolidayRegion->holidays( date );
    for ( int i = 0; i < list.count(); ++i ) {
      nonWorkDay = nonWorkDay || ( list.at( i ).dayType() == KHolidays::Holiday::NonWorkday );
    }
  }
  return !nonWorkDay;
}

QStringList EventView::holidayNames( const QDate &date ) const
{
  QStringList hdays;

  if ( d->mHolidayRegion ) {
    const KHolidays::Holiday::List list = d->mHolidayRegion->holidays( date );
    Q_FOREACH( const KHolidays::Holiday &holiday, list ) {
        hdays.append( holiday.text() );
    }
  }
  return hdays;
}

void EventView::backendErrorOccurred()
{
  handleBackendError( d->calendarSearch->errorString() );
}

void EventView::calendarReset()
{
}

void EventView::dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
  Q_ASSERT( topLeft.parent() == bottomRight.parent() );

  incidencesChanged( Akonadi::itemsFromModel( d->calendarSearch->model(), topLeft.parent(),
                     topLeft.row(), bottomRight.row() ) );
}

void EventView::rowsInserted( const QModelIndex& parent, int start, int end )
{
  incidencesAdded( Akonadi::itemsFromModel( d->calendarSearch->model(), parent, start, end ) );
}

void EventView::rowsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
  incidencesAboutToBeRemoved( Akonadi::itemsFromModel( d->calendarSearch->model(), parent, start, end ) );
}

CollectionSelection* EventView::globalCollectionSelection()
{
  return Private::sGlobalCollectionSelection;
}

/* static */
bool EventView::usesCompletedTodoPixmap( const Item &aitem, const QDate &date )
{
  const Todo::Ptr todo = Akonadi::todo( aitem );
  if ( !todo ) {
    return false;
  }

  if ( todo->isCompleted() ) {
    return true;
  } else if ( todo->recurs() ) {
    QTime time;
    if ( todo->allDay() ) {
      time = QTime( 0, 0 );
    } else {
      time = todo->dtDue().toTimeSpec( preferences()->timeSpec() ).time();
    }

    KDateTime itemDateTime( date, time, preferences()->timeSpec() );

    return itemDateTime < todo->dtDue( false );

  } else {
    return false;
  }
}

QByteArray EventView::identifier() const
{
  return d->identifier;
}

void EventView::setIdentifier( const QByteArray &identifier )
{
  d->identifier = identifier;
}

#include "eventview.moc"
// kate: space-indent on; indent-width 2; replace-tabs on;

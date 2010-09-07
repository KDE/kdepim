/*
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
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

#include "eventview_p.h"

#include "prefs.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/calendarmodel.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/collectionselectionproxymodel.h>
#include <calendarsupport/entitymodelstatesaver.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KCalCore/Incidence>
#include <KCalCore/Todo>
using namespace KCalCore;

#include <KCalUtils/RecurrenceActions>

#include <KHolidays/Holidays>

#include <KGuiItem>
#include <KLocale>

#include <QApplication>
#include <QKeyEvent>

using namespace EventViews;

CalendarSupport::CollectionSelection *EventViewPrivate::sGlobalCollectionSelection = 0;

/* static */
void EventView::setGlobalCollectionSelection( CalendarSupport::CollectionSelection *s )
{
  EventViewPrivate::sGlobalCollectionSelection = s;
}

EventView::EventView( QWidget *parent ) : QWidget( parent ), d_ptr( new EventViewPrivate( this ) )
{

  //AKONADI_PORT review: the FocusLineEdit in the editor emits focusReceivedSignal(),
  //which triggered finishTypeAhead.  But the global focus widget in QApplication is
  //changed later, thus subsequent keyevents still went to this view, triggering another
  //editor, for each keypress.
  //Thus, listen to the global focusChanged() signal (seen in Qt 4.6-stable-patched 20091112 -Frank)
  connect( qobject_cast<QApplication*>( QApplication::instance() ),
           SIGNAL(focusChanged(QWidget*,QWidget*)),
           this, SLOT(focusChanged(QWidget*,QWidget*)) );

  // Moved out of the ctor because we cannot always garuantee that q is already
  // fully initialized, causing crashes when this is not the case.
  d_ptr->initIdentifier();
  d_ptr->setUpModels();
}

EventView::EventView( EventViewPrivate *dd, QWidget *parent )
  : QWidget( parent ),
    d_ptr( dd )
{

  //AKONADI_PORT review: the FocusLineEdit in the editor emits focusReceivedSignal(),
  //which triggered finishTypeAhead.  But the global focus widget in QApplication is
  //changed later, thus subsequent keyevents still went to this view, triggering another
  //editor, for each keypress.
  //Thus, listen to the global focusChanged() signal (seen in Qt 4.6-stable-patched 20091112 -Frank)
  connect( qobject_cast<QApplication*>( QApplication::instance() ),
           SIGNAL(focusChanged(QWidget*,QWidget*)),
           this, SLOT(focusChanged(QWidget*,QWidget*)) );

  // Moved out of the ctor because we cannot always garuantee that q is already
  // fully initialized, causing crashes when this is not the case.
  d_ptr->initIdentifier();
  d_ptr->setUpModels();
}

EventView::~EventView()
{
  delete d_ptr;
}

void EventView::defaultAction( const Akonadi::Item &aitem )
{
  kDebug();
  const Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  if ( !incidence ) {
    return;
  }

  kDebug() << "  type:" << int( incidence->type() );

  if ( incidence->isReadOnly() ) {
    emit showIncidenceSignal(aitem);
  } else {
    emit editIncidenceSignal(aitem);
  }
}

void EventView::setHolidayRegion( const KHolidays::HolidayRegionPtr &holidayRegion )
{
  Q_D( EventView );
  d->mHolidayRegion = holidayRegion;
}

int EventView::showMoveRecurDialog( const Akonadi::Item &aitem, const QDate &date )
{
  const Incidence::Ptr inc = CalendarSupport::incidence( aitem );

  KDateTime dateTime( date, preferences()->timeSpec() );

  int availableOccurrences = KCalUtils::RecurrenceActions::availableOccurrences( inc, dateTime );

  const QString caption = i18nc( "@title:window", "Changing Recurring Item" );
  KGuiItem itemFuture( i18n( "Also &Future Items" ) );
  KGuiItem itemSelected( i18n( "Only &This Item" ) );
  KGuiItem itemAll( i18n( "&All Occurrences" ) );

  switch ( availableOccurrences ) {
  case KCalUtils::RecurrenceActions::NoOccurrence:
    return KCalUtils::RecurrenceActions::NoOccurrence;

  case KCalUtils::RecurrenceActions::SelectedOccurrence:
    return KCalUtils::RecurrenceActions::SelectedOccurrence;

  case KCalUtils::RecurrenceActions::AllOccurrences:
  {
    Q_ASSERT( availableOccurrences & KCalUtils::RecurrenceActions::SelectedOccurrence );

    // if there are all kinds of ooccurrences (i.e. past present and future) the user might
    // want the option only apply to current and future occurrences, leaving the past ones
    // provide a third choice for that ("Also future")
    if ( availableOccurrences == KCalUtils::RecurrenceActions::AllOccurrences ) {
      const QString message = i18n( "The item you are trying to change is a recurring item. "
                                    "Should the changes be applied only to this single occurrence, "
                                    "also to future items, or to all items in the recurrence?" );
      return KCalUtils::RecurrenceActions::questionSelectedFutureAllCancel(
        message, caption, itemSelected, itemFuture, itemAll, this );
    }
  }

  default:
    Q_ASSERT( availableOccurrences & KCalUtils::RecurrenceActions::SelectedOccurrence );
    // selected occurrence and either past or future occurrences
    const QString message = i18n( "The item you are trying to change is a recurring item. "
                                  "Should the changes be applied only to this single occurrence "
                                  "or to all items in the recurrence?" );
    return KCalUtils::RecurrenceActions::questionSelectedAllCancel(
      message, caption, itemSelected, itemAll, this );
    break;
  }

  return KCalUtils::RecurrenceActions::NoOccurrence;
}

void EventView::setCalendar( CalendarSupport::Calendar *cal )
{
  Q_D( EventView );
  if ( d->calendar != cal ) {
    d->calendar = cal;
    if ( cal && d->collectionSelectionModel ) {
      d->collectionSelectionModel->setSourceModel( cal->model() );
    }
  }
}

CalendarSupport::Calendar *EventView::calendar() const
{
  Q_D( const EventView );
  return d->calendar;
}

void EventView::setPreferences( const PrefsPtr &preferences )
{
  Q_D( EventView );
  if ( d->mPrefs != preferences ) {
    if ( preferences ) {
      d->mPrefs = preferences;
    } else {
      d->mPrefs = PrefsPtr( new Prefs() );
    }
    updateConfig();
  }
}

void EventView::setKCalPreferences( const KCalPrefsPtr &preferences )
{
  Q_D( EventView );
  if ( d->mKCalPrefs != preferences ) {
    if ( preferences ) {
      d->mKCalPrefs = preferences;
    } else {
      d->mKCalPrefs = KCalPrefsPtr( new CalendarSupport::KCalPrefs() );
    }
    updateConfig();
  }
}

PrefsPtr EventView::preferences() const
{
  Q_D( const EventView );
  return d->mPrefs;
}

KCalPrefsPtr EventView::kcalPreferences() const
{
  Q_D( const EventView );
  return d->mKCalPrefs;
}

void EventView::dayPassed( const QDate & )
{
  updateView();
}

void EventView::setIncidenceChanger( CalendarSupport::IncidenceChanger *changer )
{
  Q_D( EventView );
  d->mChanger = changer;
}

void EventView::flushView()
{
}

EventView *EventView::viewAt( const QPoint & )
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
  Q_D( EventView );
#if 0
  //AKONADI_PORT the old code called showDates() (below), which triggers a repaint,
  //which the old code relies on
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
}

KDateTime EventView::startDateTime() const
{
  Q_D( const EventView );
  return d->startDateTime;
}

KDateTime EventView::endDateTime() const
{
  Q_D( const EventView );
  return d->endDateTime;
}

KDateTime EventView::actualStartDateTime() const
{
  Q_D( const EventView );
  return d->actualStartDateTime;
}

KDateTime EventView::actualEndDateTime() const
{
  Q_D( const EventView );
  return d->actualEndDateTime;
}

void EventView::showConfigurationDialog( QWidget * )
{
}

bool EventView::processKeyEvent( QKeyEvent *ke )
{
  Q_D( EventView );
  // If Return is pressed bring up an editor for the current selected time span.
  if ( ke->key() == Qt::Key_Return ) {
    if ( ke->type() == QEvent::KeyPress ) {
      d->mReturnPressed = true;
    } else if ( ke->type() == QEvent::KeyRelease ) {
      if ( d->mReturnPressed ) {
        if ( collectionId() >= 0 ) {
          emit newEventSignal(
            Akonadi::Collection::List() << Akonadi::Collection( collectionId() ) );
        } else {
          emit newEventSignal( collectionSelection()->selectedCollections() );
        }
        d->mReturnPressed = false;
        return true;
      } else {
        d->mReturnPressed = false;
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
      d->mTypeAheadEvents.append(
        new QKeyEvent( ke->type(),
                       ke->key(),
                       ke->modifiers(),
                       ke->text(),
                       ke->isAutoRepeat(),
                       static_cast<ushort>( ke->count() ) ) );
      if ( !d->mTypeAhead && !collectionSelection()->selectedCollections().isEmpty() ) {
        d->mTypeAhead = true;
        if ( collectionId() >= 0 ) {
          emit newEventSignal(
            Akonadi::Collection::List() << Akonadi::Collection( collectionId() ) );
        } else {
          emit newEventSignal( collectionSelection()->selectedCollections() );
        }
      }
      return true;
    }
  }
  return false;
}

void EventView::setTypeAheadReceiver( QObject *o )
{
  Q_D( EventView );
  d->mTypeAheadReceiver = o;
}

void EventView::focusChanged( QWidget *, QWidget *now )
{
  Q_D( EventView );
  if ( d->mTypeAhead && now && now == d->mTypeAheadReceiver ) {
    d->finishTypeAhead();
  }
}

CalendarSupport::CollectionSelection *EventView::collectionSelection() const
{
  Q_D( const EventView );
  return d->customCollectionSelection ? d->customCollectionSelection : globalCollectionSelection();
}

void EventView::setCustomCollectionSelectionProxyModel( CalendarSupport::CollectionSelectionProxyModel *model )
{
  Q_D( EventView );
  if ( d->collectionSelectionModel == model ) {
    return;
  }

  delete d->collectionSelectionModel;
  d->collectionSelectionModel = model;
  d->setUpModels();
}

void EventView::collectionSelectionChanged()
{

}

CalendarSupport::CollectionSelectionProxyModel *EventView::customCollectionSelectionProxyModel() const
{
  Q_D( const EventView );
  return d->collectionSelectionModel;
}

CalendarSupport::CollectionSelectionProxyModel *EventView::takeCustomCollectionSelectionProxyModel()
{
  Q_D( EventView );
  CalendarSupport::CollectionSelectionProxyModel *m = d->collectionSelectionModel;
  d->collectionSelectionModel = 0;
  d->setUpModels();
  return m;
}

CalendarSupport::CollectionSelection *EventView::customCollectionSelection() const
{
  Q_D( const EventView );
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

CalendarSupport::IncidenceChanger *EventView::changer() const
{
  Q_D( const EventView );
  return d->mChanger;
}

void EventView::doRestoreConfig( const KConfigGroup & )
{
}

void EventView::doSaveConfig( KConfigGroup & )
{
}

QPair<KDateTime,KDateTime> EventView::actualDateRange( const KDateTime &start,
                                                       const KDateTime &end ) const
{
  return qMakePair( start, end );
}

void EventView::incidencesAdded( const Akonadi::Item::List & )
{
}

void EventView::incidencesAboutToBeRemoved( const Akonadi::Item::List & )
{
}

void EventView::incidencesChanged( const Akonadi::Item::List & )
{
}

void EventView::handleBackendError( const QString &errorString )
{
  kError() << errorString;
}

bool EventView::isWorkDay( const QDate &date ) const
{
  Q_D( const EventView );
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
  Q_D( const EventView );
  QStringList hdays;

  if ( d->mHolidayRegion ) {
    const KHolidays::Holiday::List list = d->mHolidayRegion->holidays( date );
    Q_FOREACH( const KHolidays::Holiday &holiday, list ) {
        hdays.append( holiday.text() );
    }
  }
  return hdays;
}

void EventView::calendarReset()
{
}

CalendarSupport::CollectionSelection *EventView::globalCollectionSelection()
{
  return EventViewPrivate::sGlobalCollectionSelection;
}

/* static */
bool EventView::usesCompletedTodoPixmap( const Akonadi::Item &aitem, const QDate &date )
{
  const Todo::Ptr todo = CalendarSupport::todo( aitem );
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
  Q_D( const EventView );
  return d->identifier;
}

void EventView::setIdentifier( const QByteArray &identifier )
{
  Q_D( EventView );
  d->identifier = identifier;
}

void EventView::setChanges( Changes changes )
{
  Q_D( EventView );
  if ( d->mChanges == NothingChanged ) {
    QMetaObject::invokeMethod( this, "updateView", Qt::QueuedConnection );
  }

  d->mChanges = changes;
}

EventView::Changes EventView::changes() const
{
  Q_D( const EventView );
  return d->mChanges;
}

void EventView::restoreConfig( const KConfigGroup &configGroup )
{
  Q_D( EventView );
  const bool useCustom = configGroup.readEntry( "UseCustomCollectionSelection", false );
  if ( !d->collectionSelectionModel && !useCustom ) {
    delete d->collectionSelectionModel;
    d->collectionSelectionModel = 0;
    d->setUpModels();
  } else if ( useCustom ) {

    if ( !d->collectionSelectionModel ) {
      d->collectionSelectionModel = new CalendarSupport::CollectionSelectionProxyModel( this );
      d->collectionSelectionModel->
        setCheckableColumn( CalendarSupport::CalendarModel::CollectionTitle );
      d->collectionSelectionModel->setDynamicSortFilter( true );
      d->collectionSelectionModel->setSortCaseSensitivity( Qt::CaseInsensitive );
      if ( d->calendar ) {
        d->collectionSelectionModel->setSourceModel( d->calendar->treeModel() );
      }
      d->setUpModels();
    }

    const KConfigGroup selectionGroup =
      configGroup.config()->group( configGroup.name() + QLatin1String( "_selectionSetup" ) );
    d->stateSaver->restoreConfig( selectionGroup );
  }

  doRestoreConfig( configGroup );
}

void EventView::saveConfig( KConfigGroup &configGroup )
{
  Q_D( EventView );
  configGroup.writeEntry( "UseCustomCollectionSelection", d->collectionSelectionModel != 0 );
  if ( d->stateSaver ) {
    KConfigGroup selectionGroup =
      configGroup.config()->group( configGroup.name() + QLatin1String( "_selectionSetup" ) );
    d->stateSaver->saveConfig( selectionGroup );
  }

  doSaveConfig( configGroup );
}

void EventView::setCollectionId( Akonadi::Collection::Id id )
{
  Q_D( EventView );
  if ( d->mCollectionId != id ) {
    d->mCollectionId = id;
  }
}

Akonadi::Collection::Id EventView::collectionId() const
{
  Q_D( const EventView );
  return d->mCollectionId;
}

#include "eventview.moc"
// kate: space-indent on; indent-width 2; replace-tabs on;

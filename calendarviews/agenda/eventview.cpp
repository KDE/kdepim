/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#include "agendaview.h" // TODO AKONADI_PORT
#include "prefs.h"

#include <libkdepim/pimmessagebox.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/kcal/collectionselectionproxymodel.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/akonadi_next/entitymodelstatesaver.h>
#include <akonadi/kcal/utils.h>
#include <Akonadi/Item>

#include <QApplication>
#include <QKeyEvent>
#include <KCal/Incidence>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>
#include <KRandom>

#include <QMenu>

using namespace Akonadi;
CollectionSelection* EventView::sGlobalCollectionSelection = 0;


/* static */
void EventView::setGlobalCollectionSelection( CollectionSelection* s )
{
  sGlobalCollectionSelection = s;
}

class EventView::Private
{
  EventView *const q;

  public:
    explicit Private( EventView* qq )
      : q( qq ),
        calendar( 0 ),
        customCollectionSelection( 0 ),
        collectionSelectionModel( 0 ),
        stateSaver( 0 )
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
    void setUpModels();
    void reconnectCollectionSelection();
};

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


EventView::EventView( QWidget *parent ) : QWidget( parent ), mChanger( 0 ), d( new Private( this ) )
{
  //TODO_SPLIT, tirar o unused
  Q_UNUSED( parent );
  mReturnPressed = false;
  mTypeAhead = false;
  mTypeAheadReceiver = 0;

  //AKONADI_PORT review: the FocusLineEdit in the editor emits focusReceivedSignal(), which triggered finishTypeAhead.
  //But the global focus widget in QApplication is changed later, thus subsequent keyevents still went to this view, triggering another editor, for each keypress
  //Thus listen to the global focusChanged() signal (seen with Qt 4.6-stable-patched 20091112 -Frank)
  connect( QApplication::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focusChanged(QWidget*,QWidget*)) );
}

EventView::~EventView()
{
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

int EventView::showMoveRecurDialog( const Item &aitem, const QDate &date )
{
  const Incidence::Ptr inc = Akonadi::incidence( aitem );
  int answer = KMessageBox::Ok;
  KGuiItem itemFuture( i18n( "Also &Future Items" ) );

  KDateTime dateTime( date, Prefs::instance()->timeSpec() );
  bool isFirst = !inc->recurrence()->getPreviousDateTime( dateTime ).isValid();
  bool isLast  = !inc->recurrence()->getNextDateTime( dateTime ).isValid();

  QString message;

  if ( !isFirst && !isLast ) {
    itemFuture.setEnabled( true );
    message = i18n( "The item you try to change is a recurring item. "
                    "Shall the changes be applied only to this single occurrence, "
                    "also to future items, or to all items in the recurrence?" );
  } else {
    itemFuture.setEnabled( false );
    message = i18n( "The item you try to change is a recurring item. "
                    "Shall the changes be applied only to this single occurrence "
                    "or to all items in the recurrence?" );
  }

  if ( !( isFirst && isLast ) ) {
    answer = PIMMessageBox::fourBtnMsgBox(
      this,
      QMessageBox::Question,
      message,
      i18n( "Changing Recurring Item" ),
      KGuiItem( i18n( "Only &This Item" ) ),
      itemFuture,
      KGuiItem( i18n( "&All Occurrences" ) ) );
  }

  return answer;
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
  mChanger = changer;
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
{ // TODO_SPLIT
  Q_UNUSED( ke );
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
        // TODO_SPLIT rename singleagendaview to agendaview
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
  mTypeAheadReceiver = o;
}

void EventView::focusChanged( QWidget*, QWidget* now )
{
  if ( mTypeAhead && now && now == mTypeAheadReceiver )
    finishTypeAhead();
}

void EventView::finishTypeAhead()
{
  if ( mTypeAheadReceiver ) {
    foreach ( QEvent *e, mTypeAheadEvents ) {
      QApplication::sendEvent( mTypeAheadReceiver, e );
    }
  }
  qDeleteAll( mTypeAheadEvents );
  mTypeAheadEvents.clear();
  mTypeAhead = false;
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
  return sGlobalCollectionSelection;
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
      time = todo->dtDue().toTimeSpec( Prefs::instance()->timeSpec() ).time();
    }

    KDateTime itemDateTime( date, time, Prefs::instance()->timeSpec() );

    return itemDateTime < todo->dtDue( false );

  } else {
    return false;
  }
}

QByteArray EventView::identifier() const
{
  return d->identifier;
}

void EventView::setIdentifier( const QByteArray& identifier )
{
  d->identifier = identifier;
}

#include "eventview.moc"

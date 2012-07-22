/*
  Copyright (C) 2010 Kevin Ottens <ervin@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "calendaradaptor.h"
#include "calendar.h"
#include "kcalprefs.h"
#include "groupware.h"
#include "mailscheduler.h"
#include "utils.h"
#include "incidencechanger.h"

#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>

#include <KMessageBox>

using namespace CalendarSupport;

template<class T>
inline QSharedPointer<T> itemToIncidence( const Akonadi::Item &item )
{
  if ( !item.hasPayload< QSharedPointer<T> >() ) {
    return QSharedPointer<T>();
  }
  return item.payload< QSharedPointer<T> >();
}

template<class T>
inline QVector<QSharedPointer<T> > itemsToIncidences( QList<Akonadi::Item> items )
{
  QVector<QSharedPointer<T> > list;
  foreach ( const Akonadi::Item &item, items ) {
    list.append( itemToIncidence<T>( item ) );
  }

  return list;
}

template<class T>
inline Akonadi::Item incidenceToItem( T *incidence )
{
  Akonadi::Item item;
  item.setPayload<QSharedPointer<T> > ( QSharedPointer<T>( incidence->clone() ) );
  return item;
}

class CalendarAdaptor::Private
{
  public:
    Private( CalendarSupport::Calendar *calendar, QWidget *parent ) : mCalendar( calendar )
    {
      Q_ASSERT( mCalendar );
      mChanger = new IncidenceChanger( mCalendar, parent );
      mChanger->setDestinationPolicy( IncidenceChanger::USE_DEFAULT_DESTINATION );
      mLastDialogCode = -1;
    }

    ~Private()
    {
      delete mChanger;
    }

    IncidenceChanger *mChanger;
    CalendarSupport::Calendar *mCalendar;
    int mLastDialogCode;
};

CalendarAdaptor::CalendarAdaptor( CalendarSupport::Calendar *calendar,
                                  QWidget *parent,
                                  bool storeDefaultCollection )
  : MemoryCalendar( KCalPrefs::instance()->timeSpec() ),
    mParent( parent ), mDeleteCalendar( false ),
    mStoreDefaultCollection( storeDefaultCollection ),
    d( new Private( calendar, parent ) )
{
}

CalendarAdaptor::~CalendarAdaptor()
{
  delete d;
}

bool CalendarAdaptor::save()
{
  return true;
}

bool CalendarAdaptor::reload()
{
  return true;
}

void CalendarAdaptor::close()
{
}

bool CalendarAdaptor::addEvent( const KCalCore::Event::Ptr &event )
{
  return addIncidence( KCalCore::Incidence::Ptr( event->clone() ) );
}

bool CalendarAdaptor::deleteEvent( const KCalCore::Event::Ptr &event )
{
  return deleteIncidence( incidenceToItem( event.data() ) );
}

void CalendarAdaptor::deleteAllEvents()
{
  Q_ASSERT( false );
} //unused

KCalCore::Event::List CalendarAdaptor::rawEvents( KCalCore::EventSortField sortField,
                                                  KCalCore::SortDirection sortDirection ) const
{
  return itemsToIncidences<KCalCore::Event>(
    d->mCalendar->rawEvents( ( CalendarSupport::EventSortField )sortField,
                          ( CalendarSupport::SortDirection )sortDirection ) );
}

KCalCore::Event::List CalendarAdaptor::rawEventsForDate( const KDateTime &dt ) const
{
  return itemsToIncidences<KCalCore::Event>( d->mCalendar->rawEventsForDate( dt ) );
}

KCalCore::Event::List CalendarAdaptor::rawEvents( const QDate &start, const QDate &end,
                                                  const KDateTime::Spec &timeSpec,
                                                  bool inclusive ) const
{
  return itemsToIncidences<KCalCore::Event>(
    d->mCalendar->rawEvents( start, end, timeSpec, inclusive ) );
}

KCalCore::Event::List CalendarAdaptor::rawEventsForDate(
  const QDate &date, const KDateTime::Spec &timeSpec,
  KCalCore::EventSortField sortField, KCalCore::SortDirection sortDirection ) const
{
  return itemsToIncidences<KCalCore::Event>( d->mCalendar->rawEventsForDate(
                                               date, timeSpec,
                                               ( CalendarSupport::EventSortField )sortField,
                                               ( CalendarSupport::SortDirection )sortDirection ) );
}

KCalCore::Event::Ptr CalendarAdaptor::event( const QString &uid,
                                             const KDateTime &recurrenceId ) const
{
  Q_UNUSED( recurrenceId );
  return itemToIncidence<KCalCore::Event>(
    d->mCalendar->event( d->mCalendar->itemIdForIncidenceUid( uid ) ) );
}

bool CalendarAdaptor::addTodo( const KCalCore::Todo::Ptr &todo )
{
  return addIncidence( KCalCore::Incidence::Ptr( todo->clone() ) );
}

bool CalendarAdaptor::deleteTodo( const KCalCore::Todo::Ptr &todo )
{
  return deleteIncidence( incidenceToItem( todo.data() ) );
}

void CalendarAdaptor::deleteAllTodos()
{
  Q_ASSERT( false );
} //unused

KCalCore::Todo::List CalendarAdaptor::rawTodos( KCalCore::TodoSortField sortField,
                                                KCalCore::SortDirection sortDirection ) const
{
  return itemsToIncidences<KCalCore::Todo>( d->mCalendar->rawTodos(
                                              ( CalendarSupport::TodoSortField )sortField,
                                              ( CalendarSupport::SortDirection )sortDirection ) );
}

KCalCore::Todo::List CalendarAdaptor::rawTodosForDate( const QDate &date ) const
{
  return itemsToIncidences<KCalCore::Todo>( d->mCalendar->rawTodosForDate( date ) );
}

KCalCore::Todo::Ptr CalendarAdaptor::todo( const QString &uid, const KDateTime &recurrenceId ) const
{
  Q_UNUSED( recurrenceId );
  return itemToIncidence<KCalCore::Todo>(
    d->mCalendar->todo( d->mCalendar->itemIdForIncidenceUid( uid ) ) );
}

bool CalendarAdaptor::addJournal( const KCalCore::Journal::Ptr &journal )
{
  return addIncidence( KCalCore::Incidence::Ptr( journal->clone() ) );
}

bool CalendarAdaptor::deleteJournal( const KCalCore::Journal::Ptr &journal )
{
  return deleteIncidence( incidenceToItem( journal.data() ) );
}

void CalendarAdaptor::deleteAllJournals()
{
  Q_ASSERT( false );
} //unused

KCalCore::Journal::List CalendarAdaptor::rawJournals( KCalCore::JournalSortField sortField,
                                                      KCalCore::SortDirection sortDirection ) const
{
  return itemsToIncidences<KCalCore::Journal>(
    d->mCalendar->rawJournals(
      ( CalendarSupport::JournalSortField )sortField,
      ( CalendarSupport::SortDirection )sortDirection ) );
}

KCalCore::Journal::List CalendarAdaptor::rawJournalsForDate( const QDate &dt ) const
{
  return itemsToIncidences<KCalCore::Journal>( d->mCalendar->rawJournalsForDate( dt ) );
}

KCalCore::Journal::Ptr CalendarAdaptor::journal( const QString &uid,
                                                 const KDateTime &recurrenceId ) const
{
  Q_UNUSED( recurrenceId );
  return itemToIncidence<KCalCore::Journal>( d->mCalendar->journal(
                                               d->mCalendar->itemIdForIncidenceUid( uid ) ) );
}

KCalCore::Alarm::List CalendarAdaptor::alarms( const KDateTime &from,
                                               const KDateTime &to ) const
{
  return d->mCalendar->alarms( from, to );
}

// From IncidenceChanger
bool CalendarAdaptor::addIncidence( const KCalCore::Incidence::Ptr &incidence )
{

  /**
     The user pressed cancel. Don't ask again.
  */
  if ( batchAdding() && d->mLastDialogCode == QDialog::Rejected ) {
    return true;
  }

  if ( mStoreDefaultCollection ) {
    d->mChanger->setDestinationPolicy( IncidenceChanger::USE_DEFAULT_DESTINATION );
  } else {
    d->mChanger->setDestinationPolicy( IncidenceChanger::ASK_DESTINATION );
  }

  Akonadi::Collection selectedCollection;
  const bool result = d->mChanger->addIncidence( incidence,
                                                 mParent,
                                                 /*by ref*/selectedCollection,
                                                 /*by ref*/d->mLastDialogCode );

  if ( mStoreDefaultCollection ) {
    /** The first time, IncidenceChanger asks the destination.
        The second time it doesn't ask, because it has a defaultCollection.
        Meanwhile, it can ask again, if for example, the defaultCollection
        doesn't support the mimetype we are trying to add. So, when importing
        a calendar, it will probably ask once, add a batch of events, and
        ask again to add a batch of to-dos ( supposing you didn't choose
        a calendar that supports both mimeTypes **/
    d->mChanger->setDefaultCollectionId( selectedCollection.id() );
  }

  return result;
}

bool CalendarAdaptor::deleteIncidence( const Akonadi::Item &aitem, bool deleteCalendar )
{
  mDeleteCalendar = deleteCalendar;
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  if ( !incidence ) {
    return true;
  }

  kDebug() << "\"" << incidence->summary() << "\"";
  bool doDelete = sendGroupwareMessage( aitem, KCalCore::iTIPCancel,
                                        IncidenceChanger::INCIDENCEDELETED );
  if ( !doDelete ) {
    if ( mDeleteCalendar ) {
      deleteLater();
    }
    return false;
  }
  Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob( aitem );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(deleteIncidenceFinished(KJob*)) );
  return true;
}

void CalendarAdaptor::deleteIncidenceFinished( KJob * j )
{
  kDebug();
  const Akonadi::ItemDeleteJob* job = qobject_cast<const Akonadi::ItemDeleteJob*>( j );
  Q_ASSERT( job );
  const Akonadi::Item::List items = job->deletedItems();
  Q_ASSERT( items.count() == 1 );
  KCalCore::Incidence::Ptr tmp = CalendarSupport::incidence( items.first() );
  Q_ASSERT( tmp );
  if ( job->error() ) {
    KMessageBox::sorry( mParent,
                        i18n( "Unable to delete incidence %1 \"%2\": %3",
                              i18n( tmp->typeStr() ),
                              tmp->summary(),
                              job->errorString() ) );
    return;
  }

  if ( !KCalPrefs::instance()->thatIsMe( tmp->organizer()->email() ) ) {
    const QStringList myEmails = KCalPrefs::instance()->allEmails();
    bool notifyOrganizer = false;
    QStringList::ConstIterator end = myEmails.constEnd();
    for ( QStringList::ConstIterator it = myEmails.constBegin(); it != end; ++it ) {
      QString email = *it;
      KCalCore::Attendee::Ptr me = tmp->attendeeByMail( email );
      if ( me ) {
        if ( me->status() == KCalCore::Attendee::Accepted ||
             me->status() == KCalCore::Attendee::Delegated ) {
          notifyOrganizer = true;
        }
        KCalCore::Attendee::Ptr newMe( new KCalCore::Attendee( *me ) );
        newMe->setStatus( KCalCore::Attendee::Declined );
        tmp->clearAttendees();
        tmp->addAttendee( newMe );
        break;
      }
    }

    if ( !Groupware::instance()->doNotNotify() && notifyOrganizer ) {
      MailScheduler scheduler( d->mCalendar );
      scheduler.performTransaction( tmp, KCalCore::iTIPReply );
    }
    //reset the doNotNotify flag
    Groupware::instance()->setDoNotNotify( false );
  }
}

bool CalendarAdaptor::sendGroupwareMessage( const Akonadi::Item &aitem,
                                            KCalCore::iTIPMethod method,
                                            IncidenceChanger::HowChanged action )
{
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  if ( !incidence ) {
    return false;
  }

  if ( KCalPrefs::instance()->thatIsMe( incidence->organizer()->email() ) &&
       incidence->attendeeCount() > 0 &&
       !KCalPrefs::instance()->mUseGroupwareCommunication ) {
    schedule( method, aitem );
    return true;
  } else if ( KCalPrefs::instance()->mUseGroupwareCommunication ) {
    Groupware::SendICalMessageDialogAnswers dialogAnswers;
    MailScheduler scheduler( d->mCalendar );
    return Groupware::instance()->sendICalMessage( mParent, method,
                                                   incidence, action, false,
                                                   dialogAnswers,
                                                   scheduler );
  }
  return true;
}

//Coming from CalendarView
void CalendarAdaptor::schedule( KCalCore::iTIPMethod method, const Akonadi::Item &item )
{
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );

  if ( incidence->attendeeCount() == 0 && method != KCalCore::iTIPPublish ) {
    KMessageBox::information( mParent, i18n( "The item has no attendees." ),
                              QString(), //TODO: add a caption when string freeze is off
                              QLatin1String( "ScheduleNoIncidences" ) );
    return;
  }

  KCalCore::Incidence::Ptr inc( incidence->clone() );
  inc->registerObserver( 0 );
  inc->clearAttendees();

  // Send the mail
  MailScheduler scheduler( d->mCalendar );
  if ( scheduler.performTransaction( incidence, method ) ) {
    KMessageBox::information( mParent,
                              i18n( "The groupware message for item '%1' "
                                    "was successfully sent.\nMethod: %2",
                                    incidence->summary(),
                                    KCalCore::ScheduleMessage::methodName( method ) ),
                              i18n( "Sending Free/Busy" ),
                              QLatin1String( "FreeBusyPublishSuccess" ) );
  } else {
    KMessageBox::error( mParent,
                        i18nc( "Groupware message sending failed. "
                               "%2 is request/reply/add/cancel/counter/etc.",
                               "Unable to send the item '%1'.\nMethod: %2",
                               incidence->summary(),
                               KCalCore::ScheduleMessage::methodName( method ) ) );
  }
}

void CalendarAdaptor::incidenceUpdate( const QString &uid, const KDateTime &recurrenceId )
{
  Q_UNUSED( uid );
  Q_UNUSED( recurrenceId );
}

void CalendarAdaptor::incidenceUpdated( const QString &uid, const KDateTime &recurrenceId )
{
  Q_UNUSED( uid );
  Q_UNUSED( recurrenceId );
}

void CalendarAdaptor::endBatchAdding()
{
  KCalCore::Calendar::endBatchAdding();
  d->mLastDialogCode = -1;
}

KCalCore::Incidence::List CalendarAdaptor::incidencesFromSchedulingID( const QString &sid ) const
{
  return  itemsToIncidences<KCalCore::Incidence>( d->mCalendar->incidencesFromSchedulingID( sid ) );
}

KCalCore::Incidence::Ptr CalendarAdaptor::incidenceFromSchedulingID( const QString &sid ) const
{
  return  itemToIncidence<KCalCore::Incidence>( d->mCalendar->incidenceFromSchedulingID( sid ) );
}


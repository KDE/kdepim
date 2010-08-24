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

CalendarAdaptor::CalendarAdaptor( CalendarSupport::Calendar *calendar, QWidget *parent,
                                  bool storeDefaultCollection )
  : MemoryCalendar( KCalPrefs::instance()->timeSpec() ),
    mCalendar( calendar ), mParent( parent ), mDeleteCalendar( false ),
    mStoreDefaultCollection( storeDefaultCollection )
{
  Q_ASSERT( mCalendar );
}

CalendarAdaptor::~CalendarAdaptor() {}

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
    mCalendar->rawEvents( ( CalendarSupport::EventSortField ) sortField,
                          ( CalendarSupport::SortDirection ) sortDirection ) );
}

KCalCore::Event::List CalendarAdaptor::rawEventsForDate( const KDateTime &dt ) const
{
  return itemsToIncidences<KCalCore::Event>( mCalendar->rawEventsForDate( dt ) );
}

KCalCore::Event::List CalendarAdaptor::rawEvents( const QDate &start, const QDate &end,
                                                  const KDateTime::Spec &timeSpec,
                                                  bool inclusive ) const
{
  return itemsToIncidences<KCalCore::Event>(
    mCalendar->rawEvents( start, end, timeSpec, inclusive ) );
}

KCalCore::Event::List CalendarAdaptor::rawEventsForDate( const QDate &date,
                                                         const KDateTime::Spec &timeSpec,
                                                         KCalCore::EventSortField sortField,
                                                         KCalCore::SortDirection sortDirection ) const
{
  return itemsToIncidences<KCalCore::Event>( mCalendar->rawEventsForDate(
                                               date, timeSpec,
                                               ( CalendarSupport::EventSortField ) sortField,
                                               ( CalendarSupport::SortDirection ) sortDirection ) );
}

KCalCore::Event::Ptr CalendarAdaptor::event( const QString &uid,
                                             const KDateTime &recurrenceId ) const
{
  Q_UNUSED( recurrenceId );
  return itemToIncidence<KCalCore::Event>(
    mCalendar->event( mCalendar->itemIdForIncidenceUid( uid ) ) );
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
  return itemsToIncidences<KCalCore::Todo>( mCalendar->rawTodos(
                                              ( CalendarSupport::TodoSortField ) sortField,
                                              ( CalendarSupport::SortDirection ) sortDirection ) );
}

KCalCore::Todo::List CalendarAdaptor::rawTodosForDate( const QDate &date ) const
{
  return itemsToIncidences<KCalCore::Todo>( mCalendar->rawTodosForDate( date ) );
}

KCalCore::Todo::Ptr CalendarAdaptor::todo( const QString &uid, const KDateTime &recurrenceId ) const
{
  Q_UNUSED( recurrenceId );
  return itemToIncidence<KCalCore::Todo>(
    mCalendar->todo( mCalendar->itemIdForIncidenceUid( uid ) ) );
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
  return itemsToIncidences<KCalCore::Journal>( mCalendar->rawJournals(
                                                 ( CalendarSupport::JournalSortField ) sortField,
                                                 ( CalendarSupport::SortDirection ) sortDirection ) );
}

KCalCore::Journal::List CalendarAdaptor::rawJournalsForDate( const QDate &dt ) const
{
  return itemsToIncidences<KCalCore::Journal>( mCalendar->rawJournalsForDate( dt ) );
}

KCalCore::Journal::Ptr CalendarAdaptor::journal( const QString &uid,
                                                 const KDateTime &recurrenceId ) const
{
  Q_UNUSED( recurrenceId );
  return itemToIncidence<KCalCore::Journal>( mCalendar->journal(
                                               mCalendar->itemIdForIncidenceUid( uid ) ) );
}

KCalCore::Alarm::List CalendarAdaptor::alarms( const KDateTime &from, const KDateTime &to ) const
{
  return mCalendar->alarms( from, to );
}

// From IncidenceChanger
bool CalendarAdaptor::addIncidence( const KCalCore::Incidence::Ptr &incidence )
{
  if( !incidence ) {
    return false;
  }
  Akonadi::Collection collection;

  const QString incidenceMimeType = incidence->mimeType();

  if ( mStoreDefaultCollection && mDefaultCollection.isValid() ) {
    collection = mDefaultCollection;
  } else {
    int dialogCode = 0;
    QStringList mimeTypes( incidenceMimeType );
    collection = CalendarSupport::selectCollection( mParent, dialogCode, mimeTypes );
  }

  if ( !collection.isValid() ) {
    return false;
  }

  if ( mStoreDefaultCollection && !mDefaultCollection.isValid() ) {
    mDefaultCollection = collection;
  }

  kDebug() << "\"" << incidence->summary() << "\"";

  Akonadi::Item item;
  item.setPayload( incidence );

  item.setMimeType( incidenceMimeType );
  Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, collection );
  // The connection needs to be queued to be sure addIncidenceFinished
  // is called after the kjob finished it's eventloop. That's needed
  // because Groupware uses synchron job->exec() calls.
  connect( job, SIGNAL(result(KJob *)),
           this, SLOT(addIncidenceFinished(KJob *)), Qt::QueuedConnection );
  return true;
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
  connect( job, SIGNAL(result(KJob *)), this, SLOT(deleteIncidenceFinished(KJob *)) );
  return true;
}

void CalendarAdaptor::addIncidenceFinished( KJob *j )
{
  kDebug();
  const Akonadi::ItemCreateJob* job = qobject_cast<const Akonadi::ItemCreateJob*>( j );
  Q_ASSERT( job );
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( job->item() );

  if  ( job->error() ) {
    KMessageBox::sorry(
      mParent,
      i18n( "Unable to save %1 \"%2\": %3",
            i18n( incidence->typeStr() ),
            incidence->summary(),
            job->errorString() ) );
    if ( mDeleteCalendar ) {
      deleteLater();
    }
    return;
  }

  Q_ASSERT( incidence );
  if ( KCalPrefs::instance()->mUseGroupwareCommunication ) {
    if ( !Groupware::instance()->sendICalMessage(
           mParent,
           KCalCore::iTIPRequest,
           incidence, IncidenceChanger::INCIDENCEADDED, false ) ) {
      kError() << "sendIcalMessage failed.";
    }
  }
  if ( mDeleteCalendar ) {
    deleteLater();
  }
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
    for ( QStringList::ConstIterator it = myEmails.begin(); it != myEmails.end(); ++it ) {
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
      MailScheduler scheduler( mCalendar );
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
    return Groupware::instance()->sendICalMessage( mParent, method,
                                                   incidence, action, false );
  }
  return true;
}

//Coming from CalendarView
void CalendarAdaptor::schedule( KCalCore::iTIPMethod method, const Akonadi::Item &item )
{
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );

  if ( incidence->attendeeCount() == 0 && method != KCalCore::iTIPPublish ) {
    KMessageBox::information( mParent, i18n( "The item has no attendees." ),
                              QLatin1String( "ScheduleNoIncidences" ) );
    return;
  }

  KCalCore::Incidence::Ptr inc( incidence->clone() );
  inc->registerObserver( 0 );
  inc->clearAttendees();

  // Send the mail
  MailScheduler scheduler( mCalendar );
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

void CalendarAdaptor::incidenceUpdate( const QString & )
{

}

void CalendarAdaptor::incidenceUpdated( const QString & )
{
}


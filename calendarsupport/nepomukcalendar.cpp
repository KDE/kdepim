/*
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Sérgio Martins <sergio.martins@kdab.com>

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

#include "nepomukcalendar.h"
#include "utils.h"
#include "kcalprefs.h"
#include "groupware.h"
#include "mailscheduler.h"
#include "next/incidencesearchjob.h"
#include "next/incidencefetchjob.h"

#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>

#include <KMessageBox>

#include <QPointer>

using namespace CalendarSupport;

class NepomukCalendar::Private
{
  public:
    Private( QWidget *parent )
    {
      mChanger = new IncidenceChanger2();
      mChanger->setDestinationPolicy( IncidenceChanger2::DestinationPolicyAsk );
      mParent = parent;
      mCalendar =
        KCalCore::MemoryCalendar::Ptr(
          new KCalCore::MemoryCalendar( KCalPrefs::instance()->timeSpec() ) );
      mJobsInProgress = 0;
    }

    ~Private()
    {
      delete mChanger;
    }

    Akonadi::Item itemForIncidenceUid( const QString &uid )
    {
      return mItemsByIncidenceUid.value( uid );
    }

    uint mJobsInProgress;
    QPointer<QWidget> mParent;
    IncidenceChanger2 *mChanger;
    KCalCore::MemoryCalendar::Ptr mCalendar;

    // MailScheduler receives a NepomukCalendar::Ptr, and we don't have
    // the strong ref to ourselfs
    QWeakPointer<NepomukCalendar> mWeakPointer;

    QHash<QString,Akonadi::Item> mItemsByIncidenceUid;
};

NepomukCalendar::NepomukCalendar( QWidget *parent )
  : MemoryCalendar( KCalPrefs::instance()->timeSpec() ),
    d( new Private( parent ) )
{
  connect( d->mChanger,
           SIGNAL(createFinished(int,Akonadi::Item,CalendarSupport::IncidenceChanger2::ResultCode,QString)),
           SLOT(createFinished(int,Akonadi::Item,CalendarSupport::IncidenceChanger2::ResultCode,QString)) );

  connect( d->mChanger,
           SIGNAL(modifyFinished(int,Akonadi::Item,CalendarSupport::IncidenceChanger2::ResultCode,QString)),
           SLOT(modifyFinished(int,Akonadi::Item,CalendarSupport::IncidenceChanger2::ResultCode,QString)) );

  IncidenceFetchJob *job = new IncidenceFetchJob();

/*
  IncidenceSearchJob *job = new IncidenceSearchJob();
#endif
*/

  connect( job, SIGNAL(result(KJob*)), this, SLOT(searchResult(KJob*)) );
}

NepomukCalendar::~NepomukCalendar()
{
  kDebug();
  delete d;
}

bool NepomukCalendar::save()
{
  return true;
}

bool NepomukCalendar::reload()
{
  return true;
}

void NepomukCalendar::close()
{
}

bool NepomukCalendar::addEvent( const KCalCore::Event::Ptr &event )
{
  return addIncidence( KCalCore::Incidence::Ptr( event->clone() ) );
}

bool NepomukCalendar::deleteEvent( const KCalCore::Event::Ptr &event )
{
  return deleteIncidence( event );
}

void NepomukCalendar::deleteAllEvents()
{
  Q_ASSERT( false );
  kDebug() << "AKONADI_PORT: deleteAllEvents not implemented";
}

KCalCore::Event::List NepomukCalendar::rawEvents( KCalCore::EventSortField sortField,
                                                  KCalCore::SortDirection sortDirection ) const
{
  return d->mCalendar->rawEvents( sortField, sortDirection );
}

KCalCore::Event::List NepomukCalendar::rawEventsForDate( const KDateTime &dt ) const
{
  return d->mCalendar->rawEventsForDate( dt );
}

KCalCore::Event::List NepomukCalendar::rawEvents( const QDate &start, const QDate &end,
                                                  const KDateTime::Spec &timeSpec,
                                                  bool inclusive ) const
{
  return d->mCalendar->rawEvents( start, end, timeSpec, inclusive );
}

KCalCore::Event::List NepomukCalendar::rawEventsForDate(
  const QDate &date, const KDateTime::Spec &timeSpec,
  KCalCore::EventSortField sortField, KCalCore::SortDirection sortDirection ) const
{
  return d->mCalendar->rawEventsForDate( date, timeSpec, sortField, sortDirection );
}

KCalCore::Event::Ptr NepomukCalendar::event( const QString &uid,
                                             const KDateTime &recurrenceId ) const
{
  Q_UNUSED( recurrenceId );
  return d->mCalendar->event( uid );
}

bool NepomukCalendar::addTodo( const KCalCore::Todo::Ptr &todo )
{
  return addIncidence( KCalCore::Incidence::Ptr( todo->clone() ) );
}

bool NepomukCalendar::deleteTodo( const KCalCore::Todo::Ptr &todo )
{
  return deleteIncidence( todo );
}

void NepomukCalendar::deleteAllTodos()
{
  Q_ASSERT( false );
  kDebug() << "AKONADI_PORT: deleteAllTodos not implemented";
}

KCalCore::Todo::List NepomukCalendar::rawTodos( KCalCore::TodoSortField sortField,
                                                KCalCore::SortDirection sortDirection ) const
{
  return d->mCalendar->rawTodos( sortField, sortDirection );
}

KCalCore::Todo::List NepomukCalendar::rawTodosForDate( const QDate &date ) const
{
  return d->mCalendar->rawTodosForDate( date );
}

KCalCore::Todo::Ptr NepomukCalendar::todo( const QString &uid, const KDateTime &recurrenceId ) const
{
  Q_UNUSED( recurrenceId );
  return d->mCalendar->todo( uid );
}

bool NepomukCalendar::addJournal( const KCalCore::Journal::Ptr &journal )
{
  return addIncidence( KCalCore::Incidence::Ptr( journal->clone() ) );
}

bool NepomukCalendar::deleteJournal( const KCalCore::Journal::Ptr &journal )
{
  return deleteIncidence( journal );
}

void NepomukCalendar::deleteAllJournals()
{
  Q_ASSERT( false );
  kDebug() << "AKONADI_PORT: deleteAllJournals() not implemented";
}

KCalCore::Journal::List NepomukCalendar::rawJournals( KCalCore::JournalSortField sortField,
                                                      KCalCore::SortDirection sortDirection ) const
{
  return d->mCalendar->rawJournals( sortField, sortDirection );
}

KCalCore::Journal::List NepomukCalendar::rawJournalsForDate( const QDate &dt ) const
{
  return d->mCalendar->rawJournalsForDate( dt );
}

KCalCore::Journal::Ptr NepomukCalendar::journal( const QString &uid,
                                                 const KDateTime &recurrenceId ) const
{
  Q_UNUSED( recurrenceId );
  return d->mCalendar->journal( uid );
}

KCalCore::Alarm::List NepomukCalendar::alarms( const KDateTime &from,
                                               const KDateTime &to ) const
{
  return d->mCalendar->alarms( from, to );
}

bool NepomukCalendar::addIncidence( const KCalCore::Incidence::Ptr &incidence )
{
  if ( d->mChanger->createIncidence( incidence ) >= 0 ) {
    d->mJobsInProgress++;
    return true;
  } else {
    kDebug() << "Couldn't create inicdence";
    return false;
  }
}

bool NepomukCalendar::deleteIncidence( const KCalCore::Incidence::Ptr &incidence )
{
  if ( incidence ) {
    Akonadi::Item item = d->itemForIncidenceUid( incidence->uid() );
    Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob( item );
    connect( job, SIGNAL(result(KJob*)), this, SLOT(deleteIncidenceFinished(KJob*)) );
    d->mJobsInProgress++;
  }

  return true;
}

void NepomukCalendar::deleteIncidenceFinished( KJob * j )
{
  kDebug();
  d->mJobsInProgress--;

  QString errorString;
  bool result = true;

  const Akonadi::ItemDeleteJob* job = qobject_cast<const Akonadi::ItemDeleteJob*>( j );
  Q_ASSERT( job );
  const Akonadi::Item::List items = job->deletedItems();
  Q_ASSERT( items.count() == 1 );
  KCalCore::Incidence::Ptr tmp = CalendarSupport::incidence( items.first() );
  Q_ASSERT( tmp );
  if ( job->error() ) {

    result = false;
    errorString = job->errorString();

    KMessageBox::sorry( d->mParent,
                        i18n( "Unable to delete incidence %1 \"%2\": %3",
                              i18n( tmp->typeStr() ),
                              tmp->summary(),
                              job->errorString() ) );

  } else {
    d->mItemsByIncidenceUid.remove( tmp->uid() );

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
        if ( !d->mWeakPointer.isNull() ) {
          NepomukCalendar::Ptr strongRef = d->mWeakPointer.toStrongRef();
          MailScheduler scheduler( strongRef );
          scheduler.performTransaction( tmp, KCalCore::iTIPReply );
        }
      }
      //reset the doNotNotify flag
      Groupware::instance()->setDoNotNotify( false );
    }
  }

  emit deleteFinished( result, errorString );
}

void NepomukCalendar::incidenceUpdate( const QString &uid, const KDateTime &recurrenceId )
{
  Q_UNUSED( uid );
  Q_UNUSED( recurrenceId );
}

void NepomukCalendar::incidenceUpdated( const QString &, const KDateTime & )
{
}

KCalCore::Incidence::List NepomukCalendar::incidencesFromSchedulingID( const QString &sid ) const
{
  return d->mCalendar->incidencesFromSchedulingID( sid );
}

KCalCore::Incidence::Ptr NepomukCalendar::incidenceFromSchedulingID( const QString &sid ) const
{
  return d->mCalendar->incidenceFromSchedulingID( sid );
}

// TODO: Move to private class
void NepomukCalendar::createFinished( int changeId,
                                      const Akonadi::Item &item,
                                      IncidenceChanger2::ResultCode changerResultCode,
                                      const QString &errorMessage )
{
  Q_UNUSED( changeId );
  d->mJobsInProgress--;

  if ( changerResultCode == IncidenceChanger2::ResultCodeSuccess ) {
    KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();
    if ( incidence ) {
      d->mItemsByIncidenceUid[incidence->uid()] = item;
    }
  } else {
    kWarning() << "Error creating incidence:" << errorMessage;
  }

  emit addFinished( changerResultCode == IncidenceChanger2::ResultCodeSuccess,
                    errorMessage );
}

void NepomukCalendar::modifyFinished(
  int changeId, const Akonadi::Item &item,
  CalendarSupport::IncidenceChanger2::ResultCode changerResultCode, const QString &errorMessage )
{
  d->mJobsInProgress--;
  Q_UNUSED( changeId );
  Q_UNUSED( item );
  emit changeFinished( changerResultCode == IncidenceChanger2::ResultCodeSuccess,
                       errorMessage );
}

void NepomukCalendar::searchResult( KJob *job )
{
  bool success;
  QString errorMessage;
  if ( job->error() ) {
    success = false;
    errorMessage = job->errorString();
  } else {
    success = true;

    IncidenceFetchJob *searchJob = qobject_cast<IncidenceFetchJob*>( job );
/*
    IncidenceSearchJob *searchJob = qobject_cast<IncidenceSearchJob*>( job );
*/
    const Akonadi::Item::List list = searchJob->items();
    foreach ( const Akonadi::Item &item, list ) {
      if ( item.hasPayload<KCalCore::Incidence::Ptr>() ) {
        KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();
        if ( incidence ) {
          d->mCalendar->addIncidence( incidence );
          d->mItemsByIncidenceUid[incidence->uid()] = item;
        }
      }
    }
  }

  emit loadFinished( success, errorMessage );
}

bool NepomukCalendar::jobsInProgress() const
{
  return d->mJobsInProgress > 0;
}

void NepomukCalendar::setWeakPointer( const QWeakPointer<NepomukCalendar> &pointer )
{
  d->mWeakPointer = pointer;
}

QWeakPointer<NepomukCalendar> NepomukCalendar::weakPointer() const
{
  return d->mWeakPointer;
}

/** static */
NepomukCalendar::Ptr NepomukCalendar::create( QWidget *parent )
{
  NepomukCalendar::Ptr nepomukCalendar( new NepomukCalendar( parent ) );
  nepomukCalendar->setWeakPointer( nepomukCalendar.toWeakRef() );
  return nepomukCalendar;
}

bool NepomukCalendar::changeIncidence( const KCalCore::Incidence::Ptr &incidence )
{
  Q_ASSERT( incidence );
  Akonadi::Item item = d->mItemsByIncidenceUid.value( incidence->uid() );

  if ( item.isValid() && d->mChanger->modifyIncidence( item ) >= 0 ) {
    d->mJobsInProgress++;
    return true;
  } else {
    return false;
  }
}

Akonadi::Item NepomukCalendar::itemForIncidenceUid( const QString &uid ) const
{
  return d->itemForIncidenceUid( uid );
}

#include "nepomukcalendar.moc"

/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2000-2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
/**
   @file calendar.cpp
   Provides the main "calendar" object class.

   @author Preston Brown
   @author Cornelius Schumacher
   @author Reinhold Kainhofer
*/
#include <stdlib.h>

#include <kdebug.h>
#include <klocale.h>

#include "exceptions.h"
#include "calfilter.h"

#include "calendar.h"

using namespace KCal;

Calendar::Calendar( const QString &timeZoneId )
{
  mTimeZoneId = timeZoneId;
  mLocalTime = false;

  init();
}

void Calendar::init()
{
  mNewObserver = false;
  mObserversEnabled = true;

  mModified = false;

  // Setup default filter, which does nothing
  mDefaultFilter = new CalFilter;
  mFilter = mDefaultFilter;
  mFilter->setEnabled( false );

  // user information...
  setOwner( Person( i18n( "Unknown Name" ), i18n( "unknown@nowhere" ) ) );
}

Calendar::~Calendar()
{
  delete mDefaultFilter;
}

const Person &Calendar::getOwner() const
{
  return mOwner;
}

void Calendar::setOwner( const Person &owner )
{
  mOwner = owner;

  setModified( true );
}

void Calendar::setTimeZoneId( const QString &timeZoneId )
{
  mTimeZoneId = timeZoneId;
  mLocalTime = false;

  setModified( true );
  doSetTimeZoneId( timeZoneId );
}

QString Calendar::timeZoneId() const
{
  return mTimeZoneId;
}

void Calendar::setLocalTime()
{
  mLocalTime = true;
  mTimeZoneId = "";

  setModified( true );
}

bool Calendar::isLocalTime() const
{
  return mLocalTime;
}

void Calendar::setFilter( CalFilter *filter )
{
  if ( filter ) {
    mFilter = filter;
  } else {
    mFilter = mDefaultFilter;
  }
}

CalFilter *Calendar::filter()
{
  return mFilter;
}

QStringList Calendar::incidenceCategories()
{
  Incidence::List rawInc( rawIncidences() );
  QStringList categories, thisCats;
  // @TODO: For now just iterate over all incidences. In the future,
  // the list of categories should be built when reading the file.
  for ( Incidence::List::ConstIterator i = rawInc.constBegin();
        i != rawInc.constEnd(); ++i ) {
    thisCats = (*i)->categories();
    for ( QStringList::ConstIterator si = thisCats.constBegin();
          si != thisCats.constEnd(); ++si ) {
      if ( categories.find( *si ) == categories.end() ) {
        categories.append( *si );
      }
    }
  }
  return categories;
}

Incidence::List Calendar::incidences( const QDate &date )
{
  return mergeIncidenceList( events( date ), todos( date ), journals( date ) );
}

Incidence::List Calendar::incidences()
{
  return mergeIncidenceList( events(), todos(), journals() );
}

Incidence::List Calendar::rawIncidences()
{
  return mergeIncidenceList( rawEvents(), rawTodos(), rawJournals() );
}

Event::List Calendar::sortEvents( Event::List *eventList,
                                  EventSortField sortField,
                                  SortDirection sortDirection )
{
  Event::List eventListSorted;
  Event::List tempList, t;
  Event::List alphaList;
  Event::List::Iterator sortIt;
  Event::List::Iterator eit;

  // Notice we alphabetically presort Summaries first.
  // We do this so comparison "ties" stay in a nice order.

  switch( sortField ) {
  case EventSortUnsorted:
    eventListSorted = *eventList;
    break;

  case EventSortStartDate:
    alphaList = sortEvents( eventList, EventSortSummary, sortDirection );
    for ( eit = alphaList.begin(); eit != alphaList.end(); ++eit ) {
      sortIt = eventListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != eventListSorted.end() &&
                (*eit)->dtStart() >= (*sortIt)->dtStart() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != eventListSorted.end() &&
                (*eit)->dtStart() < (*sortIt)->dtStart() ) {
          ++sortIt;
        }
      }
      eventListSorted.insert( sortIt, *eit );
    }
    break;

  case EventSortEndDate:
    alphaList = sortEvents( eventList, EventSortSummary, sortDirection );
    for ( eit = alphaList.begin(); eit != alphaList.end(); ++eit ) {
      if ( (*eit)->hasEndDate() ) {
        sortIt = eventListSorted.begin();
        if ( sortDirection == SortDirectionAscending ) {
          while ( sortIt != eventListSorted.end() &&
                  (*eit)->dtEnd() >= (*sortIt)->dtEnd() ) {
            ++sortIt;
          }
        } else {
          while ( sortIt != eventListSorted.end() &&
                  (*eit)->dtEnd() < (*sortIt)->dtEnd() ) {
            ++sortIt;
          }
        }
      } else {
        // Keep a list of the Events without End DateTimes
        tempList.append( *eit );
      }
      eventListSorted.insert( sortIt, *eit );
    }
    if ( sortDirection == SortDirectionAscending ) {
      // Append the list of Events without End DateTimes
      eventListSorted += tempList;
    } else {
      // Prepend the list of Events without End DateTimes
      tempList += eventListSorted;
      eventListSorted = tempList;
    }
    break;

  case EventSortSummary:
    for ( eit = eventList->begin(); eit != eventList->end(); ++eit ) {
      sortIt = eventListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != eventListSorted.end() &&
                (*eit)->summary() >= (*sortIt)->summary() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != eventListSorted.end() &&
                (*eit)->summary() < (*sortIt)->summary() ) {
          ++sortIt;
        }
      }
      eventListSorted.insert( sortIt, *eit );
    }
    break;
  }

  return eventListSorted;

}

Event::List Calendar::events( const QDate &date,
                              EventSortField sortField,
                              SortDirection sortDirection )
{
  Event::List el = rawEventsForDate( date, sortField, sortDirection );
  mFilter->apply( &el );
  return el;
}

Event::List Calendar::events( const QDateTime &qdt )
{
  Event::List el = rawEventsForDate( qdt );
  mFilter->apply( &el );
  return el;
}

Event::List Calendar::events( const QDate &start, const QDate &end,
                              bool inclusive)
{
  Event::List el = rawEvents( start, end, inclusive );
  mFilter->apply( &el );
  return el;
}

Event::List Calendar::events( EventSortField sortField,
                              SortDirection sortDirection )
{
  Event::List el = rawEvents( sortField, sortDirection );
  mFilter->apply( &el );
  return el;
}

bool Calendar::addIncidence( Incidence *incidence )
{
  Incidence::AddVisitor<Calendar> v( this );

  return incidence->accept(v);
}

bool Calendar::deleteIncidence( Incidence *incidence )
{
  if ( beginChange( incidence ) ) {
    Incidence::DeleteVisitor<Calendar> v( this );
    bool result = incidence->accept( v );
    endChange( incidence );
    return result;
  } else
    return false;
}

Incidence *Calendar::dissociateOccurrence( Incidence *incidence, QDate date,
                                           bool single )
{
  if ( !incidence || !incidence->doesRecur() )
    return 0;

  Incidence *newInc = incidence->clone();
  newInc->recreate();
  newInc->setRelatedTo( incidence );
  Recurrence *recur = newInc->recurrence();
  if ( single ) {
    recur->unsetRecurs();
  } else {
    // Adjust the recurrence for the future incidences. In particular
    // adjust the "end after n occurences" rules! "No end date" and "end by ..."
    // don't need to be modified.
    int duration = recur->duration();
    if ( duration > 0 ) {
      int doneduration = recur->durationTo( date.addDays(-1) );
      if ( doneduration >= duration ) {
        kdDebug(5850) << "The dissociated event already occured more often "
                      << "than it was supposed to ever occur. ERROR!" << endl;
        recur->unsetRecurs();
      } else {
        recur->setDuration( duration - doneduration );
      }
    }
  }
  // Adjust the date of the incidence
  if ( incidence->type() == "Event" ) {
    Event *ev = static_cast<Event *>( newInc );
    QDateTime start( ev->dtStart() );
    int daysTo = start.date().daysTo( date );
    ev->setDtStart( start.addDays( daysTo ) );
    ev->setDtEnd( ev->dtEnd().addDays( daysTo ) );
  } else if ( incidence->type() == "Todo" ) {
    Todo *td = static_cast<Todo *>( newInc );
    bool haveOffset = false;
    int daysTo = 0;
    if ( td->hasDueDate() ) {
      QDateTime due( td->dtDue() );
      daysTo = due.date().daysTo( date );
      td->setDtDue( due.addDays( daysTo ), true );
      haveOffset = true;
    }
    if ( td->hasStartDate() ) {
      QDateTime start( td->dtStart() );
      if ( !haveOffset )
        daysTo = start.date().daysTo( date );
      td->setDtStart( start.addDays( daysTo ) );
      haveOffset = true;
    }
  }
  if ( addIncidence( newInc ) ) {
    if ( single ) {
      incidence->addExDate( date );
    } else {
      recur = incidence->recurrence();
      if ( recur ) {
        // Make sure the recurrence of the past events ends
        // at the corresponding day
        recur->setEndDate( date.addDays(-1) );
      }
    }
  } else {
    delete newInc;
    return 0;
  }
  return newInc;
}

Incidence *Calendar::incidence( const QString &uid )
{
  Incidence *i = event( uid );
  if ( i )
    return i;
  i = todo( uid );
  if ( i )
    return i;
  i = journal( uid );
  return i;
}

Incidence *Calendar::incidenceFromSchedulingID( const QString &sid )
{
  Incidence::List incidences = rawIncidences();
  Incidence::List::iterator it = incidences.begin();
  for ( ; it != incidences.end(); ++it )
    if ( (*it)->schedulingID() == sid )
      // Touchdown, and the crowd goes wild
      return *it;
  // Not found
  return 0;
}

Todo::List Calendar::sortTodos( Todo::List *todoList,
                                TodoSortField sortField,
                                SortDirection sortDirection )
{
  Todo::List todoListSorted;
  Todo::List tempList, t;
  Todo::List alphaList;
  Todo::List::Iterator sortIt;
  Todo::List::Iterator eit;

  // Notice we alphabetically presort Summaries first.
  // We do this so comparison "ties" stay in a nice order.

  // Note that Todos may not have Start DateTimes nor due DateTimes.

  switch( sortField ) {
  case TodoSortUnsorted:
    todoListSorted = *todoList;
    break;

  case TodoSortStartDate:
    alphaList = sortTodos( todoList, TodoSortSummary, sortDirection );
    for ( eit = alphaList.begin(); eit != alphaList.end(); ++eit ) {
      if ( (*eit)->hasStartDate() ) {
        sortIt = todoListSorted.begin();
        if ( sortDirection == SortDirectionAscending ) {
          while ( sortIt != todoListSorted.end() &&
                  (*eit)->dtStart() >= (*sortIt)->dtStart() ) {
            ++sortIt;
          }
        } else {
          while ( sortIt != todoListSorted.end() &&
                  (*eit)->dtStart() < (*sortIt)->dtStart() ) {
            ++sortIt;
          }
        }
        todoListSorted.insert( sortIt, *eit );
      } else {
        // Keep a list of the Todos without Start DateTimes
        tempList.append( *eit );
      }
    }
    if ( sortDirection == SortDirectionAscending ) {
      // Append the list of Todos without Start DateTimes
      todoListSorted += tempList;
    } else {
      // Prepend the list of Todos without Start DateTimes
      tempList += todoListSorted;
      todoListSorted = tempList;
    }
    break;

  case TodoSortDueDate:
    alphaList = sortTodos( todoList, TodoSortSummary, sortDirection );
    for ( eit = alphaList.begin(); eit != alphaList.end(); ++eit ) {
      if ( (*eit)->hasDueDate() ) {
        sortIt = todoListSorted.begin();
        if ( sortDirection == SortDirectionAscending ) {
          while ( sortIt != todoListSorted.end() &&
                  (*eit)->dtDue() >= (*sortIt)->dtDue() ) {
            ++sortIt;
          }
        } else {
          while ( sortIt != todoListSorted.end() &&
                  (*eit)->dtDue() < (*sortIt)->dtDue() ) {
            ++sortIt;
          }
        }
        todoListSorted.insert( sortIt, *eit );
      } else {
        // Keep a list of the Todos without Due DateTimes
        tempList.append( *eit );
      }
    }
    if ( sortDirection == SortDirectionAscending ) {
      // Append the list of Todos without Due DateTimes
      todoListSorted += tempList;
    } else {
      // Prepend the list of Todos without Due DateTimes
      tempList += todoListSorted;
      todoListSorted = tempList;
    }
    break;

  case TodoSortPriority:
    alphaList = sortTodos( todoList, TodoSortSummary, sortDirection );
    for ( eit = alphaList.begin(); eit != alphaList.end(); ++eit ) {
      sortIt = todoListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != todoListSorted.end() &&
                (*eit)->priority() >= (*sortIt)->priority() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != todoListSorted.end() &&
                (*eit)->priority() < (*sortIt)->priority() ) {
          ++sortIt;
        }
      }
      todoListSorted.insert( sortIt, *eit );
    }
    break;

  case TodoSortPercentComplete:
    alphaList = sortTodos( todoList, TodoSortSummary, sortDirection );
    for ( eit = alphaList.begin(); eit != alphaList.end(); ++eit ) {
      sortIt = todoListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != todoListSorted.end() &&
                (*eit)->percentComplete() >= (*sortIt)->percentComplete() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != todoListSorted.end() &&
                (*eit)->percentComplete() < (*sortIt)->percentComplete() ) {
          ++sortIt;
        }
      }
      todoListSorted.insert( sortIt, *eit );
    }
    break;

  case TodoSortSummary:
    for ( eit = todoList->begin(); eit != todoList->end(); ++eit ) {
      sortIt = todoListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != todoListSorted.end() &&
                (*eit)->summary() >= (*sortIt)->summary() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != todoListSorted.end() &&
                (*eit)->summary() < (*sortIt)->summary() ) {
          ++sortIt;
        }
      }
      todoListSorted.insert( sortIt, *eit );
    }
    break;
  }

  return todoListSorted;
}

Todo::List Calendar::todos( TodoSortField sortField,
                            SortDirection sortDirection )
{
  Todo::List tl = rawTodos( sortField, sortDirection );
  mFilter->apply( &tl );
  return tl;
}

Todo::List Calendar::todos( const QDate &date )
{
  Todo::List el = rawTodosForDate( date );
  mFilter->apply( &el );
  return el;
}

Journal::List Calendar::sortJournals( Journal::List *journalList,
                                      JournalSortField sortField,
                                      SortDirection sortDirection )
{
  Journal::List journalListSorted;
  Journal::List::Iterator sortIt;
  Journal::List::Iterator eit;

  switch( sortField ) {
  case JournalSortUnsorted:
    journalListSorted = *journalList;
    break;

  case JournalSortDate:
    for ( eit = journalList->begin(); eit != journalList->end(); ++eit ) {
      sortIt = journalListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != journalListSorted.end() &&
                (*eit)->dtStart() >= (*sortIt)->dtStart() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != journalListSorted.end() &&
                (*eit)->dtStart() < (*sortIt)->dtStart() ) {
          ++sortIt;
        }
      }
      journalListSorted.insert( sortIt, *eit );
    }
    break;

  case JournalSortSummary:
    for ( eit = journalList->begin(); eit != journalList->end(); ++eit ) {
      sortIt = journalListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != journalListSorted.end() &&
                (*eit)->summary() >= (*sortIt)->summary() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != journalListSorted.end() &&
                (*eit)->summary() < (*sortIt)->summary() ) {
          ++sortIt;
        }
      }
      journalListSorted.insert( sortIt, *eit );
    }
    break;
  }

  return journalListSorted;
}

Journal::List Calendar::journals( JournalSortField sortField,
                                  SortDirection sortDirection )
{
  Journal::List jl = rawJournals( sortField, sortDirection );
  mFilter->apply( &jl );
  return jl;
}

Journal::List Calendar::journals( const QDate &date )
{
  Journal::List el = rawJournalsForDate( date );
  mFilter->apply( &el );
  return el;
}

// When this is called, the todo have already been added to the calendar.
// This method is only about linking related todos
void Calendar::setupRelations( Incidence *forincidence )
{
  QString uid = forincidence->uid();

  // First, go over the list of orphans and see if this is their parent
  while ( Incidence* i = mOrphans[ uid ] ) {
    mOrphans.remove( uid );
    i->setRelatedTo( forincidence );
    forincidence->addRelation( i );
    mOrphanUids.remove( i->uid() );
  }

  // Now see about this incidences parent
  if ( !forincidence->relatedTo() && !forincidence->relatedToUid().isEmpty() ) {
    // This incidence has a uid it is related to but is not registered to it yet
    // Try to find it
    Incidence* parent = incidence( forincidence->relatedToUid() );
    if ( parent ) {
      // Found it
      forincidence->setRelatedTo( parent );
      parent->addRelation( forincidence );
    } else {
      // Not found, put this in the mOrphans list
      mOrphans.insert( forincidence->relatedToUid(), forincidence );
      mOrphanUids.insert( forincidence->uid(), forincidence );
    }
  }
}

// If a task with subtasks is deleted, move it's subtasks to the orphans list
void Calendar::removeRelations( Incidence *incidence )
{
  if( !incidence ) {
    kdDebug(5800) << "Warning: Calendar::removeRelations( 0 )!\n";
    return;
  }

  QString uid = incidence->uid();

  Incidence::List relations = incidence->relations();
  Incidence::List::ConstIterator it;
  for ( it = relations.begin(); it != relations.end(); ++it ) {
    Incidence *i = *it;
    if ( !mOrphanUids.find( i->uid() ) ) {
      mOrphans.insert( uid, i );
      mOrphanUids.insert( i->uid(), i );
      i->setRelatedTo( 0 );
      i->setRelatedToUid( uid );
    }
  }

  // If this incidence is related to something else, tell that about it
  if ( incidence->relatedTo() )
    incidence->relatedTo()->removeRelation( incidence );

  // Remove this one from the orphans list
  if ( mOrphanUids.remove( uid ) )
    // This incidence is located in the orphans list - it should be removed
    if ( !( incidence->relatedTo() != 0 &&
            mOrphans.remove( incidence->relatedTo()->uid() ) ) ) {
      // Removing wasn't that easy
      for ( QDictIterator<Incidence> it( mOrphans ); it.current(); ++it ) {
        if ( it.current()->uid() == uid ) {
          mOrphans.remove( it.currentKey() );
          break;
        }
      }
    }
}

void Calendar::registerObserver( Observer *observer )
{
  if( !mObservers.contains( observer ) )
    mObservers.append( observer );
  mNewObserver = true;
}

void Calendar::unregisterObserver( Observer *observer )
{
  mObservers.remove( observer );
}

void Calendar::setModified( bool modified )
{
  if ( modified != mModified || mNewObserver ) {
    mNewObserver = false;
    Observer *observer;
    for ( observer = mObservers.first(); observer;
          observer = mObservers.next() ) {
      observer->calendarModified( modified, this );
    }
    mModified = modified;
  }
}

void Calendar::incidenceUpdated( IncidenceBase *incidence )
{
  incidence->setSyncStatus( Event::SYNCMOD );
  incidence->setLastModified( QDateTime::currentDateTime() );
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.

  // The static_cast is ok as the CalendarLocal only observes Incidence objects
  notifyIncidenceChanged( static_cast<Incidence *>( incidence ) );

  setModified( true );
}

void Calendar::notifyIncidenceAdded( Incidence *i )
{
  if ( !mObserversEnabled )
    return;

  Observer *observer;
  for ( observer = mObservers.first(); observer;
        observer = mObservers.next() ) {
    observer->calendarIncidenceAdded( i );
  }
}

void Calendar::notifyIncidenceChanged( Incidence *i )
{
  if ( !mObserversEnabled )
    return;

  Observer *observer;
  for ( observer = mObservers.first(); observer;
        observer = mObservers.next() ) {
    observer->calendarIncidenceChanged( i );
  }
}

void Calendar::notifyIncidenceDeleted( Incidence *i )
{
  if ( !mObserversEnabled )
    return;

  Observer *observer;
  for ( observer = mObservers.first(); observer;
        observer = mObservers.next() ) {
    observer->calendarIncidenceDeleted( i );
  }
}

void Calendar::setProductId( const QString &productId )
{
  mProductId = productId;
}

QString Calendar::productId()
{
  return mProductId;
}

Incidence::List Calendar::mergeIncidenceList( const Event::List &events,
                                              const Todo::List &todos,
                                              const Journal::List &journals )
{
  Incidence::List incidences;

  Event::List::ConstIterator it1;
  for ( it1 = events.begin(); it1 != events.end(); ++it1 )
    incidences.append( *it1 );

  Todo::List::ConstIterator it2;
  for ( it2 = todos.begin(); it2 != todos.end(); ++it2 )
    incidences.append( *it2 );

  Journal::List::ConstIterator it3;
  for ( it3 = journals.begin(); it3 != journals.end(); ++it3 )
    incidences.append( *it3 );

  return incidences;
}

bool Calendar::beginChange( Incidence * )
{
  return true;
}

bool Calendar::endChange( Incidence * )
{
  return true;
}

void Calendar::setObserversEnabled( bool enabled )
{
  mObserversEnabled = enabled;
}

#include "calendar.moc"

/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown
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

#include <stdlib.h>

#include <kdebug.h>
#include <klocale.h>

#include "exceptions.h"
#include "calfilter.h"

#include "calendar.h"

using namespace KCal;

Calendar::Calendar()
{
  mTimeZoneId = QString::fromLatin1( "UTC" );
  mLocalTime = false;

  init();
}

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
  mFilter->setEnabled(false);

  // initialize random numbers.  This is a hack, and not
  // even that good of one at that.
//  srandom(time(0));

  // user information...
  setOwner( Person( i18n("Unknown Name"), i18n("unknown@nowhere") ) );

#if 0
  tmpStr = KOPrefs::instance()->mTimeZone;
//  kdDebug(5800) << "Calendar::Calendar(): TimeZone: " << tmpStr << endl;
  int dstSetting = KOPrefs::instance()->mDaylightSavings;
  extern long int timezone;
  struct tm *now;
  time_t curtime;
  curtime = time(0);
  now = localtime(&curtime);
  int hourOff = - ((timezone / 60) / 60);
  if (now->tm_isdst)
    hourOff += 1;
  QString tzStr;
  tzStr.sprintf("%.2d%.2d",
		hourOff,
		abs((timezone / 60) % 60));

  // if no time zone was in the config file, write what we just discovered.
  if (tmpStr.isEmpty()) {
//    KOPrefs::instance()->mTimeZone = tzStr;
  } else {
    tzStr = tmpStr;
  }

  // if daylight savings has changed since last load time, we need
  // to rewrite these settings to the config file.
  if ((now->tm_isdst && !dstSetting) ||
      (!now->tm_isdst && dstSetting)) {
    KOPrefs::instance()->mTimeZone = tzStr;
    KOPrefs::instance()->mDaylightSavings = now->tm_isdst;
  }

  setTimeZone(tzStr);
#endif

//  KOPrefs::instance()->writeConfig();
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

void Calendar::setTimeZoneId(const QString &id)
{
  mTimeZoneId = id;
  mLocalTime = false;

  setModified( true );
  doSetTimeZoneId( id );
}

QString Calendar::timeZoneId() const
{
  return mTimeZoneId;
}

void Calendar::setLocalTime()
{
  mLocalTime = true;
  mTimeZone = 0;
  mTimeZoneId = "";

  setModified( true );
}

bool Calendar::isLocalTime() const
{
  return mLocalTime;
}

void Calendar::setFilter(CalFilter *filter)
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
  // TODO: For now just iterate over all incidences. In the future, 
  // the list of categories should be built when reading the file.
  for ( Incidence::List::ConstIterator i = rawInc.constBegin(); i != rawInc.constEnd(); ++i ) {
    thisCats = (*i)->categories();
    for ( QStringList::ConstIterator si = thisCats.constBegin(); si != thisCats.constEnd(); ++si ) {
      if ( categories.find( *si ) == categories.end() ) {
        categories.append( *si );
      }
    }
  }
  return categories;
}

Incidence::List Calendar::incidences( const QDate &qdt )
{
  Journal::List jnls;
  Journal*jnl = journal(qdt);
  if (jnl) jnls.append( journal(qdt) );
  return mergeIncidenceList( events( qdt ), todos( qdt ), jnls );
}

Incidence::List Calendar::incidences()
{
  return mergeIncidenceList( events(), todos(), journals() );
}

Incidence::List Calendar::rawIncidences()
{
  return mergeIncidenceList( rawEvents(), rawTodos(), journals() );
}

Event::List Calendar::events( const QDate &date, bool sorted )
{
  Event::List el = rawEventsForDate( date, sorted );

  mFilter->apply(&el);

  return el;
}

Event::List Calendar::events( const QDateTime &qdt )
{
  Event::List el = rawEventsForDate(qdt);
  mFilter->apply(&el);
  return el;
}

Event::List Calendar::events( const QDate &start, const QDate &end,
                                  bool inclusive)
{
  Event::List el = rawEvents(start,end,inclusive);
  mFilter->apply(&el);
  return el;
}

Event::List Calendar::events()
{
  Event::List el = rawEvents();
  mFilter->apply(&el);
  return el;
}


bool Calendar::addIncidence(Incidence *i)
{
  Incidence::AddVisitor<Calendar> v(this);

  return i->accept(v);
}

bool Calendar::deleteIncidence( Incidence *i )
{
  if ( beginChange( i ) ) {
    Incidence::DeleteVisitor<Calendar> v( this );
    bool result = i->accept( v );
    endChange( i );
    return result;
  } else
    return false;    
}

Incidence *Calendar::dissociateOccurrence( Incidence *incidence, QDate date,
                                           bool single )
{
  if ( !incidence || !incidence->doesRecur() ) return 0;
  
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
        kdDebug(5850) << "The dissociated event already occured more often that it was supposed to ever occur. ERROR!" << endl;
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
      daysTo = due.date().daysTo( date ) ;
      td->setDtDue( due.addDays( daysTo ), true );
      haveOffset = true;
    }
    if ( td->hasStartDate() ) {
      QDateTime start( td->dtStart() );
      if ( !haveOffset ) daysTo = start.date().daysTo( date );
      td->setDtStart( start.addDays( daysTo ) );
      haveOffset = true;
    }
  }
  if ( addIncidence( newInc ) ) {
    if (single) {
      incidence->addExDate( date );
    } else {
      recur = incidence->recurrence();
      if ( recur ) {
        // Make sure the recurrence of the past events ends at the corresponding day
        recur->setEndDate( date.addDays(-1) );
      }
    }
  } else {
    delete newInc;
    return 0;
  }
  return newInc;
}

Incidence *Calendar::incidence( const QString& uid )
{
  Incidence *i = event( uid );
  if ( i ) return i;
  i = todo( uid );
  if ( i ) return i;
  i = journal( uid );
  return i;
}

Incidence *Calendar::incidenceFromSchedulingID( const QString &UID )
{
  Incidence::List incidences = rawIncidences();
  Incidence::List::iterator it = incidences.begin();
  for ( ; it != incidences.end(); ++it )
    if ( (*it)->schedulingID() == UID )
      // Touchdown, and the crowd goes wild
      return *it;
  // Not found
  return 0;
}

Todo::List Calendar::todos()
{
  Todo::List tl = rawTodos();
  mFilter->apply( &tl );
  return tl;
}

Todo::List Calendar::todos( const QDate &date )
{
  Todo::List el = rawTodosForDate( date );

  mFilter->apply(&el);

  return el;
}


// When this is called, the todo have already been added to the calendar.
// This method is only about linking related todos
void Calendar::setupRelations( Incidence *forincidence )
{
  QString uid = forincidence->uid();

  // First, go over the list of orphans and see if this is their parent
  while( Incidence* i = mOrphans[ uid ] ) {
    mOrphans.remove( uid );
    i->setRelatedTo( forincidence );
    forincidence->addRelation( i );
    mOrphanUids.remove( i->uid() );
  }

  // Now see about this incidences parent
  if( !forincidence->relatedTo() && !forincidence->relatedToUid().isEmpty() ) {
    // This incidence has a uid it is related to, but is not registered to it yet
    // Try to find it
    Incidence* parent = incidence( forincidence->relatedToUid() );
    if( parent ) {
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
  for( it = relations.begin(); it != relations.end(); ++it ) {
    Incidence *i = *it;
    if( !mOrphanUids.find( i->uid() ) ) {
      mOrphans.insert( uid, i );
      mOrphanUids.insert( i->uid(), i );
      i->setRelatedTo( 0 );
      i->setRelatedToUid( uid );
    }
  }

  // If this incidence is related to something else, tell that about it
  if( incidence->relatedTo() )
    incidence->relatedTo()->removeRelation( incidence );

  // Remove this one from the orphans list
  if( mOrphanUids.remove( uid ) )
    // This incidence is located in the orphans list - it should be removed
    if( !( incidence->relatedTo() != 0 && mOrphans.remove( incidence->relatedTo()->uid() ) ) ) {
      // Removing wasn't that easy
      for( QDictIterator<Incidence> it( mOrphans ); it.current(); ++it ) {
        if( it.current()->uid() == uid ) {
          mOrphans.remove( it.currentKey() );
          break;
        }
      }
    }
}

void Calendar::registerObserver( Observer *observer )
{
  if( !mObservers.contains( observer ) ) mObservers.append( observer );
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
    for( observer = mObservers.first(); observer;
         observer = mObservers.next() ) {
      observer->calendarModified( modified, this );
    }
    mModified = modified;
  }
}

void Calendar::incidenceUpdated( IncidenceBase *incidence )
{
//  kdDebug(5800) << "CalendarResources::incidenceUpdated( IncidenceBase * ): Not yet implemented\n";
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
  if ( !mObserversEnabled ) return;

  Observer *observer;
  for( observer = mObservers.first(); observer;
       observer = mObservers.next() ) {
    observer->calendarIncidenceAdded( i );
  }
}

void Calendar::notifyIncidenceChanged( Incidence *i )
{
  if ( !mObserversEnabled ) return;

  Observer *observer;
  for( observer = mObservers.first(); observer;
       observer = mObservers.next() ) {
    observer->calendarIncidenceChanged( i );
  }
}

void Calendar::notifyIncidenceDeleted( Incidence *i )
{
  if ( !mObserversEnabled ) return;

  Observer *observer;
  for( observer = mObservers.first(); observer;
       observer = mObservers.next() ) {
    observer->calendarIncidenceDeleted( i );
  }
}

void Calendar::setLoadedProductId( const QString &id )
{
  mLoadedProductId = id;
}

QString Calendar::loadedProductId()
{
  return mLoadedProductId;
}

Incidence::List Calendar::mergeIncidenceList( const Event::List &e,
                                              const Todo::List &t,
                                              const Journal::List &j )
{
  Incidence::List incidences;
  
  Event::List::ConstIterator it1;
  for( it1 = e.begin(); it1 != e.end(); ++it1 ) incidences.append( *it1 );

  Todo::List::ConstIterator it2;
  for( it2 = t.begin(); it2 != t.end(); ++it2 ) incidences.append( *it2 );

  Journal::List::ConstIterator it3;
  for( it3 = j.begin(); it3 != j.end(); ++it3 ) incidences.append( *it3 );

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

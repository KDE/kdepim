/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
  mObserver = 0;
  mNewObserver = false;

  mModified = false;

  // Setup default filter, which does nothing
  mDefaultFilter = new CalFilter;
  mFilter = mDefaultFilter;
  mFilter->setEnabled(false);

  // initialize random numbers.  This is a hack, and not
  // even that good of one at that.
//  srandom(time(0));

  // user information...
  setOwner(i18n("Unknown Name"));
  setEmail(i18n("unknown@nowhere"));

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

const QString &Calendar::getOwner() const
{
  return mOwner;
}

void Calendar::setOwner(const QString &os)
{
  int i;
  mOwner = os;
  i = mOwner.find(',');
  if (i != -1)
    mOwner = mOwner.left(i);

  setModified( true );
}

void Calendar::setTimeZone(const QString & tz)
{
  bool neg = FALSE;
  int hours, minutes;
  QString tmpStr(tz);

  if (tmpStr.left(1) == "-")
    neg = TRUE;
  if (tmpStr.left(1) == "-" || tmpStr.left(1) == "+")
    tmpStr.remove(0, 1);
  hours = tmpStr.left(2).toInt();
  if (tmpStr.length() > 2)
    minutes = tmpStr.right(2).toInt();
  else
    minutes = 0;
  mTimeZone = (60*hours+minutes);
  if (neg)
    mTimeZone = -mTimeZone;
  mLocalTime = false;

  setModified( true );
}

QString Calendar::getTimeZoneStr() const
{
  if (mLocalTime)
    return QString();
  QString tmpStr;
  int hours = abs(mTimeZone / 60);
  int minutes = abs(mTimeZone % 60);
  bool neg = mTimeZone < 0;

  tmpStr.sprintf("%c%.2d%.2d",
		 (neg ? '-' : '+'),
		 hours, minutes);
  return tmpStr;
}

void Calendar::setTimeZone(int tz)
{
  mTimeZone = tz;
  mLocalTime = false;

  setModified( true );
}

int Calendar::getTimeZone() const
{
  return mTimeZone;
}

void Calendar::setTimeZoneId(const QString &id)
{
  mTimeZoneId = id;
  mLocalTime = false;

  setModified( true );
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

const QString &Calendar::getEmail()
{
  return mOwnerEmail;
}

void Calendar::setEmail(const QString &e)
{
  mOwnerEmail = e;

  setModified( true );
}

void Calendar::setFilter(CalFilter *filter)
{
  mFilter = filter;
}

CalFilter *Calendar::filter()
{
  return mFilter;
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
  Incidence::DeleteVisitor<Calendar> v( this );
  return i->accept( v );
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

Todo::List Calendar::todos()
{
  Todo::List tl = rawTodos();
  mFilter->apply( &tl );
  return tl;
}

// When this is called, the todo have already been added to the calendar.
// This method is only about linking related todos
void Calendar::setupRelations( Incidence *incidence )
{
  QString uid = incidence->uid();

  // First, go over the list of orphans and see if this is their parent
  while( Incidence* i = mOrphans[ uid ] ) {
    mOrphans.remove( uid );
    i->setRelatedTo( incidence );
    incidence->addRelation( i );
    mOrphanUids.remove( i->uid() );
  }

  // Now see about this incidences parent
  if( !incidence->relatedTo() && !incidence->relatedToUid().isEmpty() ) {
    // This incidence has a uid it is related to, but is not registered to it yet
    // Try to find it
    Incidence* parent = this->incidence( incidence->relatedToUid() );
    if( parent ) {
      // Found it
      incidence->setRelatedTo( parent );
      parent->addRelation( incidence );
    } else {
      // Not found, put this in the mOrphans list
      mOrphans.insert( incidence->relatedToUid(), incidence );
      mOrphanUids.insert( incidence->uid(), incidence );
    }
  }
}

// If a task with subtasks is deleted, move it's subtasks to the orphans list
void Calendar::removeRelations( Incidence *incidence )
{
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
  mObserver = observer;
  mNewObserver = true;
}

void Calendar::setModified( bool modified )
{
  if ( modified != mModified || mNewObserver ) {
    mNewObserver = false;
    if ( mObserver ) mObserver->calendarModified( modified, this );
    mModified = modified;
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

#include "calendar.moc"

/*
    This file is part of KitchenSync.

    Copyright (C) 2002,2004 Holger Hans Peter Freyther <freyther@kde.org>

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

#include "calendarmerger.h"

#include "calendarsyncee.h"

#include <libkcal/event.h>
#include <libkcal/todo.h>

#include <kstaticdeleter.h>

#include <qmap.h>

namespace KSync {

/**
 *  Iternal classes and methods to do the merge of the attributes.
 *  First their declaration then their definition.
 *  Then a common template baseclass for Event and Todo.
 *
 */
namespace CalendarMergerInternal {
/* forwards */
template<class T> void mergeOrg( T* const, const T* const );
template<class T> void mergeRead( T* const, const T* const );
template<class T> void mergedtStart( T* const, const T* const );
template<class T> void mergeDur( T* const, const T* const );
template<class T> void mergeFloat( T* const, const T* const );
template<class T> void mergeAttend( T* const, const T* const );
template<class T> void mergeCreated( T* const, const T* const );
template<class T> void mergeRev( T* const, const T* const );
template<class T> void mergeDes( T* const, const T* const );
template<class T> void mergeSum( T* const, const T* const );
template<class T> void mergeCat( T* const, const T* const );
template<class T> void mergeRel( T* const, const T* const );
template<class T> void mergeExDates( T* const, const T* const );
template<class T> void mergeAtt( T* const, const T* const );
template<class T> void mergeSec( T* const,  const T* const );
template<class T> void mergeRes( T* const,  const T* const );
template<class T> void mergePrio( T* const, const T* const );
template<class T> void mergeAlarm( T* const, const T* const );
template<class T> void mergeRecur( T* const, const T* const );
template<class T> void mergeLoc( T* const, const T* const );

template<class T> void mergeDue( T* const, const T* const);
template<class T> void mergeStart( T* const, const T* const );
template<class T> void mergeComp(T* const, const T* const );
template<class T> void mergePer(T* const, const T* const );

template<class T> void mergeDtEnd( T* const, const T* const );
template<class T> void mergeStartFloat( T* const, const T* const ); /* merge date and float attribute */
template<class T> void mergeDueFloat( T* const, const T* const );   /* merge date and float attribute */

template <class T>
class MergeBase
{
public:
    MergeBase();
    virtual ~MergeBase();
    typedef void (*merge)(T* const, const T* const );
    typedef QMap<int, merge> MergeMap;
    typedef typename QMap<int, merge>::Iterator Iterator;
    void invoke( int, T* const, const T* const );
    void add( int, merge );
protected:
    MergeMap map;
};

typedef MergeBase<KCal::Event> MergeCal;
typedef MergeBase<KCal::Todo> MergeTodo;

static MergeCal  *mergeEventMap = 0;
static MergeTodo *mergeTodoMap  = 0;

static KStaticDeleter<MergeCal> mergeEventDeleter;
static KStaticDeleter<MergeTodo> mergeTodoDeleter;

template <class T> MergeBase<T>::MergeBase()
{
  map.insert( CalendarMerger::Organizer,    mergeOrg );
  map.insert( CalendarMerger::ReadOnly,     mergeRead );
  map.insert( CalendarMerger::DtStart,      mergedtStart );
  map.insert( CalendarMerger::Duration,     mergeDur );
  map.insert( CalendarMerger::Float,        mergeFloat );
  map.insert( CalendarMerger::Attendee,     mergeAttend );
  map.insert( CalendarMerger::CreatedDate,  mergeCreated );
  map.insert( CalendarMerger::Revision,     mergeRev );
  map.insert( CalendarMerger::Description,  mergeDes );
  map.insert( CalendarMerger::Summary,      mergeSum );
  map.insert( CalendarMerger::Category,     mergeCat );
  map.insert( CalendarMerger::Relations,    mergeRel );
  map.insert( CalendarMerger::ExDates,      mergeExDates );
  map.insert( CalendarMerger::Attachments,  mergeAtt );
  map.insert( CalendarMerger::Secrecy,      mergeSec );
  map.insert( CalendarMerger::Resources,    mergeRes );
  map.insert( CalendarMerger::Priority,     mergePrio );
  map.insert( CalendarMerger::Alarms,       mergeAlarm );
  map.insert( CalendarMerger::Recurrence,   mergeRecur );
  map.insert( CalendarMerger::Location,     mergeLoc );
}

template <class T> MergeBase<T>::~MergeBase()
{
}

template <class T> void MergeBase<T>::invoke(int i, T* const dest, const T* const src)
{
  Iterator it= map.find( i );
  if ( it != map.end() )
    (*it.data())(dest, src );
}

template<class T>
void MergeBase<T>::add(int res, merge mer )
{
  map.insert( res, mer );
}

void init()
{
  if ( mergeTodoMap )
    return;

  mergeTodoDeleter. setObject(mergeTodoMap,  new MergeTodo);
  mergeEventDeleter.setObject(mergeEventMap, new MergeCal );

  /* todo specefic additional information */
  mergeTodoMap->add( CalendarMerger::DtDue,     mergeDue   );
  mergeTodoMap->add( CalendarMerger::StartDate, mergeStart );
  mergeTodoMap->add( CalendarMerger::Completed, mergeComp  );
  mergeTodoMap->add( CalendarMerger::Percent,   mergePer   );
  mergeTodoMap->add( CalendarMerger::StartDateTime, mergeStartFloat );
  mergeTodoMap->add( CalendarMerger::DueDateTime,   mergeDueFloat   );

  /* event specefic additional information */
  mergeEventMap->add( CalendarMerger::DtEnd,    mergeDtEnd );
}

// implementation of the merge functions
/*
 * Merge the Organizer Field.
 */
template <class Todo> void mergeOrg( Todo* const dest, const Todo* const src)
{
  dest->setOrganizer( src->organizer() );
}

/*
 * Merge the ReadOnly Field.
 */
template <class Todo> void mergeRead( Todo* const dest, const Todo* const src)
{
  dest->setReadOnly( src->isReadOnly() );
}

/*
 * Merge the Start Date.
 */
template <class Todo> void mergedtStart( Todo* const dest, const Todo* const src)
{
  dest->setDtStart( src->dtStart() );
}

/*
 * Merge the Duration.
 */
template <class Todo> void mergeDur( Todo* const dest, const Todo* const src)
{
  dest->setDuration( src->duration() );
}

/*
 * Merge if it is floating.
 */
template <class Todo> void mergeFloat( Todo* const dest, const Todo* const src)
{
  dest->setFloats( src->doesFloat() );
}

/*
 * Merge in the Attendees.
 */
template <class Todo>  void mergeAttend( Todo* const dest, const Todo* const src)
{
  KCal::Attendee::List att = src->attendees();
  KCal::Attendee::List::ConstIterator it;
  for ( it = att.begin(); it != att.end(); ++it )
    dest->addAttendee( new KCal::Attendee( **it ) );
}

/*
 * Merge the Created on field.
 */
template <class Todo>  void mergeCreated( Todo* const dest, const Todo* const src)
{
  dest->setCreated( src->created() );
}

/*
 * Merge the Revision Field.
 */
template <class Todo> void mergeRev( Todo* const dest, const Todo* const src)
{
  dest->setRevision( src->revision() );
}

/*
 * Merge the Description.
 */
template <class Todo> void mergeDes( Todo* const dest, const Todo* const src)
{
  dest->setDescription( src->description() );
}

/*
 * Merge the Summary.
 */
template <class Todo> void mergeSum( Todo* const dest, const Todo* const src)
{
  dest->setSummary( src->summary() );
}

/*
 * Merge the Category.
 */
template <class Todo> void mergeCat( Todo* const dest, const Todo* const src)
{
  dest->setCategories( src->categories() );
}

/*
 * Merge the Relations by cloning.
 */
template <class Todo> void mergeRel( Todo* const dest, const Todo* const src)
{
  KCal::Incidence::List rel = src->relations();
  KCal::Incidence::List::ConstIterator it;
  for ( it = rel.begin(); it != rel.end(); ++it )
    dest->addRelation( (*it)->clone() );

}

/*
 * Merge the EXteded Dates.
 */
template <class Todo> void mergeExDates( Todo* const dest, const Todo* const src)
{
  dest->setExDates( src->exDates() );
}

/*
 * @todo
 */
template <class Todo> void mergeAtt( Todo* const, const Todo* const )
{
// FIXME!!!
}

/*
 * Merge the secrecy attribute
 */
template <class Todo> void mergeSec( Todo* const dest,  const Todo* const src)
{
  dest->setSecrecy( src->secrecy() );
}

/**
 * Merge the 'resources' list. Not KRES but what is needed for the Incidence
 */
template <class Todo> void mergeRes( Todo* const dest,  const Todo* const src)
{
  dest->setResources( src->resources() );
}

/*
 * Merge the Priority.
 */
template <class Todo> void mergePrio( Todo* const dest, const Todo* const src)
{
    dest->setPriority( src->priority() );
}

/*
 * Merge the Alarms
 */
template <class Todo> void mergeAlarm( Todo* const dest, const Todo* const src )
{
    KCal::Alarm::List als = src->alarms();
    KCal::Alarm::List::ConstIterator it;
    for ( it = als.begin(); it != als.end(); ++it )
        dest->addAlarm( new KCal::Alarm( **it ) );

}

/*
 * Merge Recurrence @todo
 */
template <class Todo> void mergeRecur( Todo* const , const Todo* const )
{
  // not available
}

/*
 * Merget the Location of the Incidence.
 */
template <class Todo> void mergeLoc( Todo* const dest , const Todo* const src)
{
    dest->setLocation( src->location() );
}


/// KCal::Todo Specefic
/*
 * Merge the Due Date
 */
template<class T>
void mergeDue( T* const dest, const T* const src)
{
 dest->setDtDue( src->dtDue() );
}

/*
 * Merge the Start Date
 */
template<class T>
void mergeStart( T* const dest, const T* const src)
{
  dest->setHasStartDate( src->hasStartDate() );
}

/*
 * Merge if they were completed
 */
template<class T>
void mergeComp( T* const dest, const T* const src)
{
  dest->setCompleted( src->isCompleted() );
}

/*
 * Merge how many percent where completed
 */
template<class T>
void mergePer( T* const dest, const T* const src)
{
  dest->setPercentComplete( src->percentComplete() );
}

// Calendar specefic

/*
 * The End of an Event
 */
template<class T>
void mergeDtEnd( T* const dest, const T* src)
{
  dest->setDtEnd( src->dtEnd() );
}

template<class Todo> void mergeStartFloat( Todo* const dest, const Todo* const src ) {
  /*
   * merge the start Time of the src into dest!
   * Merge only if both have startDates and finally
   * src doesFloat
   */
  if ( dest->hasStartDate() && src->hasStartDate() && src->doesFloat() ) {
    QDateTime dt = dest->dtStart( true );
    dt.setTime( src->dtStart( true ).time() );
    dest->setDtStart( dt );
  }
}

/*
 * same as mergeStartFloat
 */
template<class Todo> void mergeDueFloat( Todo* const dest, const Todo* const src ) {
  /*
   * merge the due Time of the src into dest!
   * Merge only if both have dueDates and finally
   * src doesFloat
   */
  if ( dest->hasDueDate() && src->hasDueDate() && src->doesFloat() ) {
    QDateTime dt = dest->dtDue( true );
    dt.setTime( src->dtDue( true ).time() );
    dest->setDtDue( dt, true );
  }
}

}

// End of the Namespace



//////////////////
// Now the actual and way shorter implementation of ther Merger which
// actually calls the functios above
//


/**
 * Create a New Merger for CalendarSyncee. This works for Todo and Calendar.
 * You need to specify the support map for Todo and Event.
 *
 * @param todo  The Support Map for Todos.
 * @param event The Support Map for Events.
 */
CalendarMerger::CalendarMerger( const QBitArray& todo, const QBitArray& event )
    : mEvent( event ), mTodo( todo )
{
  setSynceeType( QString::fromLatin1("CalendarSyncee") );
}

CalendarMerger::~CalendarMerger()
{}

/**
 * Now merge according to Merger::merge
 */
bool CalendarMerger::merge( SyncEntry* _entry, SyncEntry* _other )
{
  if ( !sameType( _entry, _other, QString::fromLatin1("CalendarSyncEntry") ) )
    return false;

  /*
   * Lets cast
   */
  CalendarSyncEntry *entry, *other;
  entry = static_cast<CalendarSyncEntry*>( _entry );
  other = static_cast<CalendarSyncEntry*>( _other );


  /*
   * Wouldn't be able to sync two different types successfully right
   */
  if ( entry->incidence()->type() != other->incidence()->type() )
    return false;

  CalendarMergerInternal::init();
  if ( entry->incidence()->type() == "Event" )
    mergeCal ( entry, other );
  else
    mergeTodo( entry, other );

  return true;
}


void CalendarMerger::mergeTodo( CalendarSyncEntry* entry, CalendarSyncEntry* other )
{
  QBitArray otherSup;
  if ( other->syncee() && other->syncee()->merger() )
    otherSup = otherMerger<CalendarMerger>( other )->mTodo;
  else {
    otherSup = QBitArray( mTodo.size() );
    otherSup.fill( true );
  }


  for ( uint i=0;i < otherSup.size() && i < mTodo.size(); ++i )
    if ( otherSup[i] && !mTodo[i] )
      CalendarMergerInternal::mergeTodoMap->invoke( i, static_cast<KCal::Todo*>( entry->incidence() ),
                                                    static_cast<KCal::Todo*>( other->incidence() ) );

}

void CalendarMerger::mergeCal( CalendarSyncEntry* entry, CalendarSyncEntry* other ) {
  QBitArray otherSup;
  if ( other->syncee() && other->syncee()->merger() )
    otherSup = otherMerger<CalendarMerger>( other )->mEvent;
  else {
    otherSup = QBitArray( mEvent.size() );
    otherSup.fill( true );
  }

  for ( uint i=0;i < otherSup.size() && i < mEvent.size(); ++i )
    if ( otherSup[i] && !mEvent[i] )
      CalendarMergerInternal::mergeEventMap->invoke( i, static_cast<KCal::Event*>( entry->incidence() ),
                                                     static_cast<KCal::Event*>( other->incidence() ) );

}
}

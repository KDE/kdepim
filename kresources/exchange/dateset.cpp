/*
    This file is part of libkpimexchange.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
    02111-1307, USA.
*/

// $Id$

#include <qptrlist.h>
#include <qdatetime.h>
#include <qpair.h>

#include <kdebug.h>

#include "dateset.h"

DateSet::DateSet()
{
  kdDebug() << "Creating DateSet" << endl;
  // mOldestDate = 
  mDates = new RangeList();
  mDates->setAutoDelete( true );
}

DateSet::~DateSet()
{
  kdDebug() << "Deleting DateSet" << endl;
  delete mDates;
}

void DateSet::add( QDate const& date )
{
  if (mDates->isEmpty()) {
    mDates->insert( 0, new QPair<QDate,QDate>( date, date ) );
    return;
  } 
  int i = find( date );
  mDates->insert( i, new QPair<QDate,QDate>( date, date ) );
  tryMerge( i );
  tryMerge( i-1 );
}

void DateSet::add( QDate const& from, QDate const& to )
{
  if (mDates->isEmpty()) {
    mDates->insert( 0, new QPair<QDate,QDate>( from, to ) );
    return;
  } 
  uint i = find( from );
  kdDebug() << "Adding range at position " << i << endl;
  mDates->insert( i, new QPair<QDate,QDate>( from, to ) );

  do {
  } while ( tryMerge( i ) );
  do {
  } while ( tryMerge( i-1 ) );
/*  
  QPair<QDate,QDate>* item = mDates->at( i );

  if (to >= item->first)
    return;

  if (to.daysTo( item->first ) == 1 )
    item->first = from;
  else
    mDates->insert( i, new QPair<QDate,QDate>( from, to ) );
*/
}

void DateSet::remove( QDate const& date )
{
  if (mDates->isEmpty()) {
    return;
  } 

  uint i = find( date );
  if ( i == mDates->count() )
    return;

  QPair<QDate,QDate>* item = mDates->at( i );
  if ( date < item->first )
    return;
  if ( date == item->first ) {
    if ( date == item->second ) {
      mDates->remove( i );
    } else {
      item->first = item->first.addDays( 1 );
    }
    return;
  }

  if ( date == item->second ) {
    item->second = item->second.addDays( -1 );
  } else {
    mDates->insert( i, new QPair<QDate,QDate>(item->first, date.addDays( -1 ) ) );
    item->first = date.addDays( 1 );
  }
}

void DateSet::remove( QDate const& from, QDate const& to )
{
  if (mDates->isEmpty()) {
    return;
  } 
  
  uint i = find( from );
  if ( i == mDates->count() )
    return;
  
  while( i < mDates->count() ) {
    QPair<QDate,QDate>* item = mDates->at( i );
    // Check if we're done: next item is later dan removal period
    if ( to < item->first )
      break;

    // Check if entire item should be removed
    if ( from <= item->first && to >= item->second ) {
      mDates->remove( i );
      // Don't skip the next range
      continue;
    }

    // Check if we should take a slice out of the middle of the item
    if ( from > item->first && to < item->second ) {
      mDates->insert( i, new QPair<QDate,QDate>( item->first, from.addDays( -1 ) ) );
      item->first = to.addDays( 1 );
      break; // We're done
    }

    // Now check if we should take off the beginning of the item
    if ( from <= item->first ) {
      item->first = to.addDays( 1 );
      // Finished
      break;
    }

    // Only one possibility left: we should take off the end
    // of the current range
    item->second = from.addDays( -1 );
    i++;
  }
}

bool DateSet::contains( QDate const& date )
{
  if (mDates->isEmpty()) {
    return false;
  } 

  uint i = find( date );
//  kdDebug() << "contains looking for " << date.toString() << " at range " << i << endl;
  if ( i == mDates->count() )
    return false;

  QPair<QDate,QDate>* item = mDates->at( i );
  // kdDebug() << "contains looking at range " << item->first.toString() << " -- " << item->second.toString() << endl;
  return ( item->first <= date );
}

// returns true if and only if the whole range is in the set
bool DateSet::contains( QDate const& from, QDate const& to )
{
  if (mDates->isEmpty()) {
    return false;
  } 

  uint i = find( from );
  if ( i == mDates->count() )
    return false;

  QPair<QDate,QDate>* item = mDates->at( i );

  return ( from >= item->first && to <= item->second );
}

// Finds the index in mDates of the range containing date, if it
// exists. Else, return the index of the range following the date.
// If mDates is empty, return 0.
// If date is later than the last item in mDates, return mDates->count()

int DateSet::find( QDate const& date )
{
  if ( mDates->isEmpty() )
    return 0;

  int start = 0;
  int end = mDates->count();
  while ( start < end ) {
    int i = start + (end-start) / 2;
    // kdDebug() << start << ", " << i << ", " << end << endl;
    QPair<QDate,QDate> *item = mDates->at( i );
    if ( item->first <= date && date <= item->second )
      return i;
    if ( date > item->second ) {
      start = i+1;
    } else { // this means date < item->first
      end = i;
    }
  }

  // kdDebug() << "Found for date " << date.toString() << " range " << end << endl;
  return end;
/*
  // Now either start==end or start+1 == end
  if ( mDates->at( end )->second < date )
    return end+1;
  else if (mDates->at( start )->first > date )
    return start;
  else
    return end;
*/
}

void DateSet::print()
{
  for( uint i=0; i<mDates->count(); i++ )
  {
    QDate start = mDates->at( i )->first;
    QDate end = mDates->at( i )->second;
    if (start == end)
      kdDebug() << start.toString() << endl;
    else
      kdDebug() << "(" << start.toString() << " , " << end.toString() << ")" << endl;
  }
}

// Try and merge range i with range i+1
// NOT TRUE preconditions: range i starts before range i+1, but MAY end later!
// preconditions: range i starts before or in range i+1
bool DateSet::tryMerge( int i )
{
  if ( i < 0 || i+1 >= mDates->count() )
    return false;

  QPair<QDate,QDate>* item1 = mDates->at( i );
  QPair<QDate,QDate>* item2 = mDates->at( i+1 );

  // First case: item1 starts before or on the same date as item2
  if ( item1->first <= item2->first ) {
    // Check for overlap
    if ( item1->second >= item2->first ||
         item1->second.daysTo( item2->first ) == 1 ) {
      kdDebug() << "Merging items " << i << " and " << (i+1) << endl;
      if (item1->second < item2->second) item1->second = item2->second;
      mDates->remove( i+1 );
      return true;
    }
    return false;
  }

  // Second case: item1 starts later than item2 (but at the latest on
  // the last day of item2, see preconditions!)
  
  // Check for overlap
  if ( item1->second >= item2->first ||
       item1->second.daysTo( item2->first ) == 1 ) {
    kdDebug() << "Merging items " << i << " and " << (i+1) << endl;
    if (item1->second < item2->second) item1->second = item2->second;
    item1->first = item2->first;
    mDates->remove( i+1 );
    return true;
  }
  return false;
}



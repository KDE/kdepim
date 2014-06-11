/*
  Copyright (c) 2000,2001,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>
  Copyright (C) 2010 Casey Link <casey@kdab.com>

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
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "conflictresolver.h"
#include "freebusyitemmodel.h"

#include <KCalendarSystem>
#include <QDebug>

#include <KLocale>

static const int DEFAULT_RESOLUTION_SECONDS = 15 * 60; // 15 minutes, 1 slot = 15 minutes

using namespace IncidenceEditorNG;

ConflictResolver::ConflictResolver( QWidget *parentWidget, QObject *parent )
  : QObject( parent ),
    mFBModel( new FreeBusyItemModel( this ) ),
    mParentWidget( parentWidget ),
    mWeekdays( 7 ),
    mSlotResolutionSeconds( DEFAULT_RESOLUTION_SECONDS )
{

  // trigger a reload in case any attendees were inserted before
  // the connection was made
  // triggerReload();

  // set default values, all the days
  mWeekdays.setBit( 0 ); // Monday
  mWeekdays.setBit( 1 );
  mWeekdays.setBit( 2 );
  mWeekdays.setBit( 3 );
  mWeekdays.setBit( 4 );
  mWeekdays.setBit( 5 );
  mWeekdays.setBit( 6 ); // Sunday
  mMandatoryRoles << KCalCore::Attendee::ReqParticipant
                  << KCalCore::Attendee::OptParticipant
                  << KCalCore::Attendee::NonParticipant
                  << KCalCore::Attendee::Chair;

  connect( mFBModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(freebusyDataChanged()) );

  connect( &mCalculateTimer, SIGNAL(timeout()), SLOT(findAllFreeSlots()) );
  mCalculateTimer.setSingleShot( true );
}

void ConflictResolver::insertAttendee( const KCalCore::Attendee::Ptr &attendee )
{
  if ( !mFBModel->containsAttendee( attendee ) ) {
    mFBModel->addItem( FreeBusyItem::Ptr( new FreeBusyItem( attendee, mParentWidget ) ) );
  }
}

void ConflictResolver::insertAttendee( const FreeBusyItem::Ptr &freebusy )
{
  if ( !mFBModel->containsAttendee( freebusy->attendee() ) ) {
    mFBModel->addItem( freebusy );
  }
}

void ConflictResolver::removeAttendee( const KCalCore::Attendee::Ptr &attendee )
{
  mFBModel->removeAttendee( attendee );
  calculateConflicts();
}

void ConflictResolver::clearAttendees()
{
  mFBModel->clear();
}

bool ConflictResolver::containsAttendee( const KCalCore::Attendee::Ptr &attendee )
{
  return mFBModel->containsAttendee( attendee );
}

void ConflictResolver::setEarliestDate( const QDate &newDate )
{
  KDateTime newStart = mTimeframeConstraint.start();
  newStart.setDate( newDate );
  mTimeframeConstraint = KCalCore::Period( newStart, mTimeframeConstraint.end() );
  calculateConflicts();
}

void ConflictResolver::setEarliestTime( const QTime &newTime )
{
  KDateTime newStart = mTimeframeConstraint.start();
  newStart.setTime( newTime );
  mTimeframeConstraint = KCalCore::Period( newStart, mTimeframeConstraint.end() );
  calculateConflicts();
}

void ConflictResolver::setLatestDate( const QDate &newDate )
{
  KDateTime newEnd = mTimeframeConstraint.end();
  newEnd.setDate( newDate );
  mTimeframeConstraint = KCalCore::Period( mTimeframeConstraint.start(), newEnd );
  calculateConflicts();
}

void ConflictResolver::setLatestTime( const QTime &newTime )
{
  KDateTime newEnd = mTimeframeConstraint.end();
  newEnd.setTime( newTime );
  mTimeframeConstraint = KCalCore::Period( mTimeframeConstraint.start(), newEnd );
  calculateConflicts();
}

void ConflictResolver::setEarliestDateTime( const KDateTime &newDateTime )
{
  mTimeframeConstraint = KCalCore::Period( newDateTime, mTimeframeConstraint.end() );
  calculateConflicts();
}

void ConflictResolver::setLatestDateTime( const KDateTime &newDateTime )
{
  mTimeframeConstraint = KCalCore::Period( mTimeframeConstraint.start(), newDateTime );
  calculateConflicts();
}

void ConflictResolver::freebusyDataChanged()
{
  calculateConflicts();
}

int ConflictResolver::tryDate( KDateTime &tryFrom, KDateTime &tryTo )
{
  int conflicts_count = 0;
  for ( int i = 0; i < mFBModel->rowCount(); ++i ) {
    QModelIndex index = mFBModel->index( i );
    KCalCore::Attendee::Ptr attendee =
      mFBModel->data( index, FreeBusyItemModel::AttendeeRole ).value<KCalCore::Attendee::Ptr>();
    if ( !matchesRoleConstraint( attendee ) ) {
      continue;
    }
    KCalCore::FreeBusy::Ptr freebusy =
      mFBModel->data( index, FreeBusyItemModel::FreeBusyRole ).value<KCalCore::FreeBusy::Ptr>();
    if ( !tryDate( freebusy, tryFrom, tryTo ) ) {
      ++conflicts_count;
    }
  }
  return conflicts_count;
}

bool ConflictResolver::tryDate( const KCalCore::FreeBusy::Ptr &fb,
                                KDateTime &tryFrom, KDateTime &tryTo )
{
  // If we don't have any free/busy information, assume the
  // participant is free. Otherwise a participant without available
  // information would block the whole allocation.
  if ( !fb ) {
    return true;
  }

  KCalCore::Period::List busyPeriods = fb->busyPeriods();
  for ( KCalCore::Period::List::Iterator it = busyPeriods.begin();
        it != busyPeriods.end(); ++it ) {
    if ( (*it).end() <= tryFrom || // busy period ends before try period
         (*it).start() >= tryTo ) { // busy period starts after try period
      continue;
    } else {
      // the current busy period blocks the try period, try
      // after the end of the current busy period
      const int secsDuration = tryFrom.secsTo( tryTo );
      tryFrom = ( *it ).end();
      tryTo = tryFrom.addSecs( secsDuration );
      // try again with the new try period
      tryDate( fb, tryFrom, tryTo );
      // we had to change the date at least once
      return false;
    }
  }
  return true;
}
bool ConflictResolver::findFreeSlot( const KCalCore::Period &dateTimeRange )
{
  KDateTime dtFrom = dateTimeRange.start();
  KDateTime dtTo = dateTimeRange.end();
  if ( tryDate( dtFrom, dtTo ) ) {
    // Current time is acceptable
    return true;
  }

  KDateTime tryFrom = dtFrom;
  KDateTime tryTo = dtTo;

  // Make sure that we never suggest a date in the past, even if the
  // user originally scheduled the meeting to be in the past.
  KDateTime now = KDateTime::currentUtcDateTime();
  if ( tryFrom < now ) {
    // The slot to look for is at least partially in the past.
    const int secs = tryFrom.secsTo( tryTo );
    tryFrom = now;
    tryTo = tryFrom.addSecs( secs );
  }

  bool found = false;
  while ( !found ) {
    found = tryDate( tryFrom, tryTo );
    // PENDING(kalle) Make the interval configurable
    if ( !found && dtFrom.daysTo( tryFrom ) > 365 ) {
      break; // don't look more than one year in the future
    }
  }

  dtFrom = tryFrom;
  dtTo = tryTo;

  return found;
}

void ConflictResolver::findAllFreeSlots()
{
  // Uses an O(p*n) (n number of attendees, p timeframe range / timeslot resolution ) algorithm to
  // locate all free blocks in a given timeframe that match the search constraints.
  // Does so by:
  // 1. convert each attendees schedule for the timeframe into a bitarray according to
  //    the time resolution, where each time slot has a value of 1 = busy, 0 = free.
  // 2. align the arrays vertically, and sum the columns
  // 3. the resulting summation indcates # of conflicts at each timeslot
  // 4. locate contiguous timeslots with a values of 0. these are the free time blocks.

  // define these locally for readability
  const KDateTime begin = mTimeframeConstraint.start();
  const KDateTime end =  mTimeframeConstraint.end();

  // calculate the time resolution
  // each timeslot in the arrays represents a unit of time
  // specified here.
  if ( mSlotResolutionSeconds < 1 ) {
    // fallback to default, if the user's value is invalid
    mSlotResolutionSeconds = DEFAULT_RESOLUTION_SECONDS;
  }

  // calculate the length of the timeframe in terms of the amount of timeslots.
  // Example: 1 week timeframe, with resolution of 15 minutes
  //          1 week = 10080 minutes / 15 = 672 15 min timeslots
  //          So, the array would have a length of 672
  const int range = begin.secsTo( end ) / mSlotResolutionSeconds;
  if ( range <= 0 ) {
    qWarning() << "free slot calculation: invalid range. range( " << begin.secsTo( end )
               << ") / mSlotResolutionSeconds(" << mSlotResolutionSeconds << ") = " << range;
    return;
  }

//QT5
#if 0
  qDebug() << "from " << begin << " to " << end
           << "; mSlotResolutionSeconds = " << mSlotResolutionSeconds
           << "; range = " << range;
#endif
  // filter out attendees for which we don't have FB data
  // and which don't match the mandatory role contrstaint
  QList<KCalCore::FreeBusy::Ptr> filteredFBItems;
  for ( int i = 0; i < mFBModel->rowCount(); ++i ) {
    QModelIndex index = mFBModel->index( i );
    KCalCore::Attendee::Ptr attendee =
      mFBModel->data( index, FreeBusyItemModel::AttendeeRole ).value<KCalCore::Attendee::Ptr>();
    if ( !matchesRoleConstraint( attendee ) ) {
      continue;
    }
    KCalCore::FreeBusy::Ptr freebusy =
      mFBModel->data( index, FreeBusyItemModel::FreeBusyRole ).value<KCalCore::FreeBusy::Ptr>();
    if( freebusy ) {
      filteredFBItems << freebusy;
    }
  }

  // now we know the number of attendees we are calculating for
  const int number_attendees = filteredFBItems.size();
  if ( number_attendees <= 0 ) {
    qDebug() << "no attendees match search criteria";
    return;
  }
  qDebug() << "num attendees: " << number_attendees;
  // this is a 2 dimensional array where the rows are attendees
  // and the columns are 0 or 1 denoting freee or busy respectively.
  QVector< QVector<int> > fbTable;

  // Explanation of the following loop:
  // iterate: through each attendee
  //   allocate: an array of length <range> and fill it with 0s
  //   iterate: through each attendee's busy period
  //     if: the period lies inside our timeframe
  //     then:
  //       calculate the array index within the timeframe range of the beginning of the busy period
  //       fill from that index until the period ends with a 1, representing busy
  //     fi
  //   etareti
  // append the allocated array to <fbTable>
  // etareti
  foreach ( KCalCore::FreeBusy::Ptr currentFB, filteredFBItems ) {
    Q_ASSERT( currentFB ); // sanity check
    KCalCore::Period::List busyPeriods = currentFB->busyPeriods();
    QVector<int> fbArray( range );
    fbArray.fill( 0 ); // initialize to zero
    for ( KCalCore::Period::List::Iterator it = busyPeriods.begin();
          it != busyPeriods.end(); ++it ) {
      if ( it->end() >= begin && it->start() <= end ) {
        int start_index = -1; // Initialize it to an invalid value.
        int duration = -1;    // Initialize it to an invalid value.
        // case1: the period is completely in our timeframe
        if( it->end() <= end && it->start() >= begin ) {
          start_index = begin.secsTo( it->start() ) / mSlotResolutionSeconds;
          duration = it->start().secsTo( it->end() ) / mSlotResolutionSeconds;
          duration -= 1; // vector starts at 0
          // case2: the period begins before our timeframe begins
        } else if( it->start() <= begin && it->end() <= end ) {
          start_index = 0;
          duration = ( begin.secsTo( it->end() ) / mSlotResolutionSeconds ) - 1;
          // case3: the period ends after our timeframe ends
        } else if( it->end() >= end && it->start() >= begin ) {
          start_index = begin.secsTo( it->start() ) / mSlotResolutionSeconds;
          duration = range - start_index - 1;
          // case4: case2+case3: our timeframe is inside the period
        } else if( it->start() <= begin  && it->end() >= end ) {
          start_index = 0;
          duration = range - 1;
        } else {
          //QT5
          //qFatal() << "impossible condition reached" << it->start() << it->end();
        }
        //      qDebug() << start_index << "+" << duration << "="
        //               << start_index + duration << "<=" << range;
        Q_ASSERT( ( start_index + duration ) < range ); // sanity check
        for ( int i = start_index; i <= start_index + duration; ++i ) {
          fbArray[i] = 1;
        }
      }
    }
    Q_ASSERT( fbArray.size() == range ); // sanity check
    fbTable.append( fbArray );
  }

  Q_ASSERT( fbTable.size() == number_attendees );

  // Now, create another array to represent the allowed weekdays constraints
  // All days which are not allowed, will be marked as busy
  const KCalendarSystem *calSys = KLocale::global()->calendar();
  QVector<int> fbArray( range );
  fbArray.fill( 0 ); // initialize to zero
  for ( int slot = 0; slot < fbArray.size(); ++slot ) {
    const KDateTime dateTime = begin.addSecs( slot * mSlotResolutionSeconds );
    const int dayOfWeek = calSys->dayOfWeek( dateTime.date() ) - 1; // bitarray is 0 indexed
    if ( !mWeekdays[dayOfWeek] ) {
      fbArray[slot] = 1;
    }
  }
  fbTable.append( fbArray );

  // Create the composite array that will hold the sums for
  // each 15 minute timeslot
  QVector<int> summed( range );
  summed.fill( 0 ); // initialize to zero

  // Sum the columns of the table
  for ( int i = 0; i < fbTable.size(); ++i ) {
    for ( int j = 0; j < range; ++j ) {
      summed[j] += fbTable[i][j];
    }
  }

  // Finally, iterate through the composite array locating contiguous free timeslots
  int free_count = 0;
  bool free_found = false;
  mAvailableSlots.clear();
  for ( int i = 0; i < range; ++i ) {
    // free timeslot encountered, increment counter
    if ( summed[i] == 0 ) {
      ++free_count;
    }
    if ( summed[i] != 0 || ( i == ( range - 1 ) && summed[i] == 0 ) ) {
      // current slot is not free, so push the previous free blocks
      // OR we are in the last slot and it is free
      if ( free_count > 0 ) {
        int free_start_i;// start index of the free block
        int free_end_i; // end index of the free block
        if ( summed[i] == 0 ) {
          // special case: we are on the last slot and it is free
          //               so we want to include this slot in the free block
          free_start_i = i - free_count + 1; // add one, to set us back inside the array because
                                             // free_count was incremented already this iteration
          free_end_i = i + 1; // add one to compensate for the fact that the array is 0 indexed
        } else {
          free_start_i = i - free_count;
          free_end_i = i - 1 + 1; // add one to compensate for the fact that the array is 0 indexed
          // compiler will optmize out the -1+1, but I leave it here to make the reasoning apparent.
        }
        // convert from our timeslot interval back into to normal seconds
        // then calculate the date times of the free block based on
        // our initial timeframe
        const KDateTime freeBegin = begin.addSecs( free_start_i * mSlotResolutionSeconds );
        const KDateTime freeEnd =
          freeBegin.addSecs( ( free_end_i - free_start_i ) * mSlotResolutionSeconds );
        // push the free block onto the list
        mAvailableSlots << KCalCore::Period( freeBegin, freeEnd );
        free_count = 0;
        if ( !free_found ) {
          free_found = true;
        }
      }
    }
  }
  if ( free_found ) {
    emit freeSlotsAvailable( mAvailableSlots );
  }
#if 0
  //DEBUG, dump the arrays. very helpful for debugging
  QTextStream dump( stdout );
  dump << "    ";
  dump.setFieldWidth( 3 );
  for ( int i = 0; i < range; ++i ) { // header
    dump << i;
  }
  dump.setFieldWidth( 1 );
  dump << "\n\n";
  for ( int i = 0; i < number_attendees; ++i ) {
    dump.setFieldWidth( 1 );
    dump << i << ":  ";
    dump.setFieldWidth( 3 );
    for ( int j = 0; j < range; ++j ) {
      dump << fbTable[i][j];
    }
    dump << "\n\n";
  }
  dump.setFieldWidth( 1 );
  dump << "    ";
  dump.setFieldWidth( 3 );
  for ( int i = 0; i < range; ++i ) {
    dump << summed[i];
  }
  dump << "\n";
#endif
}

void ConflictResolver::calculateConflicts()
{
  KDateTime start = mTimeframeConstraint.start();
  KDateTime end = mTimeframeConstraint.end();
  const int count = tryDate( start, end );
  emit conflictsDetected( count );

  if ( !mCalculateTimer.isActive() ) {
    mCalculateTimer.start( 0 );
  }
}

void ConflictResolver::setAllowedWeekdays( const QBitArray &weekdays )
{
  mWeekdays = weekdays;
  calculateConflicts();
}

void ConflictResolver::setMandatoryRoles( const QSet< KCalCore::Attendee::Role > &roles )
{
  mMandatoryRoles = roles;
  calculateConflicts();
}

bool ConflictResolver::matchesRoleConstraint( const KCalCore::Attendee::Ptr &attendee )
{
  return mMandatoryRoles.contains( attendee->role() );
}

KCalCore::Period::List ConflictResolver::availableSlots() const
{
  return mAvailableSlots;
}

void ConflictResolver::setResolution( int seconds )
{
  mSlotResolutionSeconds = seconds;
}

FreeBusyItemModel *ConflictResolver::model() const
{
  return mFBModel;
}

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

#include <QtCore/QVector>

#include <KCalendarSystem>
#include <KCal/FreeBusy>
#include <KDebug>

#include <akonadi/kcal/freebusymanager.h> //krazy:exclude=camelcase since kdepim/akonadi
#include <akonadi/kcal/groupware.h>       //krazy:exclude=camelcase since kdepim/akonadi

#include "attendeedata.h"
#include "freebusyitem.h"

static const int DEFAULT_RESOLUTION_SECONDS = 15 * 60; // 15 minutes, 1 slot = 15 minutes

using namespace IncidenceEditorsNG;

ConflictResolver::ConflictResolver( QWidget *parentWidget, QObject* parent ): QObject( parent ), mParentWidget( parentWidget ), mWeekdays( 7 ), mSlotResolutionSeconds( DEFAULT_RESOLUTION_SECONDS )
{
    Q_ASSERT( Akonadi::Groupware::instance() );
    // Groupware initializes the FreeBusyManager via a singleshot timer, so
    // the FreeBusyManager may not be initialized at this point. Queue up
    // a connection attempt.
    mManagerConnected = false;
    Akonadi::FreeBusyManager *m = Akonadi::Groupware::instance()->freeBusyManager();
    if ( !m ) {
        QTimer::singleShot( 0, this, SLOT( setupManager() ) );
    } else {
        setupManager();
    }
    // set default values
    mWeekdays.setBit( 0 ); //Monday
    mWeekdays.setBit( 1 ); //Tuesday
    mWeekdays.setBit( 2 ); //Wednesday
    mWeekdays.setBit( 3 ); //Thursday
    mWeekdays.setBit( 4 ); //Friday.. surprise!
    mMandatoryRoles << KCal::Attendee::ReqParticipant << KCal::Attendee::OptParticipant << KCal::Attendee::NonParticipant << KCal::Attendee::Chair;

    connect( &mReloadTimer, SIGNAL( timeout() ), SLOT( autoReload() ) );
    mReloadTimer.setSingleShot( true );
}

void ConflictResolver::setupManager()
{
    if ( mManagerConnected )
        return;
    static int attempt_count = 1;
    if ( attempt_count > 5 ) { // 5 chosen as an arbitrary limit
        kWarning() << "Free Busy Editor cannot connect to Akonadi's FreeBusyManager. Number of connection attempts exceeded";
        return;
    } else
        kDebug() << "Setting up freebusy manager. attempt: " << attempt_count;
    Akonadi::FreeBusyManager *m = Akonadi::Groupware::instance()->freeBusyManager();
    if ( !m ) {
        ++attempt_count;
        QTimer::singleShot( 1000, this, SLOT( setupManager() ) ); // min 1 second timer
        kDebug() << "FreeBusyManager not ready yet, will try again.";
    } else {
        connect( m, SIGNAL( freeBusyRetrieved( KCal::FreeBusy *, const QString & ) ),
                 SLOT( slotInsertFreeBusy( KCal::FreeBusy *, const QString & ) ) );
        kDebug() << "FreeBusyManager connection succeeded";
        mManagerConnected = true;
        // trigger a reload in case any attendees were inserted before
        // the connection was made
        triggerReload();
    }
}

void ConflictResolver::insertAttendee( const KCal::Attendee &attendee )
{
//     kDebug() << "inserted attendee" << attendee->email();
    FreeBusyItem *item = new FreeBusyItem( attendee, mParentWidget );
    mFreeBusyItems.append( item );
    if ( mManagerConnected )
        updateFreeBusyData( item );
}

void ConflictResolver::insertAttendee( FreeBusyItem* freebusy )
{
    mFreeBusyItems.append( freebusy );
}

void ConflictResolver::removeAttendee( const KCal::Attendee &attendee )
{
    FreeBusyItem *anItem = 0;
    for ( int i = 0; i < mFreeBusyItems.count(); i++ ) {
        anItem = mFreeBusyItems[i];
        if ( anItem->attendee() == attendee ) {
            if ( anItem->updateTimerID() != 0 ) {
                killTimer( anItem->updateTimerID() );
            }
            delete anItem;
            mFreeBusyItems.removeAt( i );
            break;
        }
    }
    calculateConflicts();
}

void ConflictResolver::clearAttendees()
{
    qDeleteAll( mFreeBusyItems );
    mFreeBusyItems.clear();
}

bool ConflictResolver::containsAttendee( const KCal::Attendee &attendee )
{
    FreeBusyItem *anItem = 0;
    for ( uint i = 0; i < mFreeBusyItems.count(); i++ ) {
        anItem = mFreeBusyItems[i];
        if ( anItem->attendee() == attendee ) {
            return true;
        }
    }
    return false;
}

void ConflictResolver::updateFreeBusyData( FreeBusyItem* item )
{
    if ( item->isDownloading() ) {
        // This item is already in the process of fetching the FB list
        return;
    }

    if ( item->updateTimerID() != 0 ) {
        // An update timer is already running. Reset it
        killTimer( item->updateTimerID() );
    }

    // This item does not have a download running, and no timer is set
    // Do the download in one second
    item->setUpdateTimerID( startTimer( 1000 ) );
}

void ConflictResolver::timerEvent( QTimerEvent* event )
{
    killTimer( event->timerId() );
    Q_FOREACH( FreeBusyItem * item, mFreeBusyItems ) {
        if ( item->updateTimerID() == event->timerId() ) {
            item->setUpdateTimerID( 0 );
            item->startDownload( mForceDownload );
            return;
        }
    }
}

void ConflictResolver::slotInsertFreeBusy( KCal::FreeBusy* fb, const QString& email )
{
    if ( fb ) {
        fb->sortList();
    }
    Q_FOREACH( FreeBusyItem *item, mFreeBusyItems ) {
        if ( item->email() == email ) {
            item->setFreeBusy( fb );
        }
    }
    calculateConflicts();
}

void ConflictResolver::setEarliestDate( const QDate& newDate )
{
    KDateTime newStart = mTimeframeConstraint.start();
    newStart.setDate( newDate );
    mTimeframeConstraint = KCal::Period( newStart, mTimeframeConstraint.end() );
    calculateConflicts();
}

void ConflictResolver::setEarliestTime( const QTime& newTime )
{
    KDateTime newStart = mTimeframeConstraint.start();
    newStart.setTime( newTime );
    mTimeframeConstraint = KCal::Period( newStart, mTimeframeConstraint.end() );
    calculateConflicts();
}

void ConflictResolver::setLatestDate( const QDate& newDate )
{
    KDateTime newEnd = mTimeframeConstraint.end();
    newEnd.setDate( newDate );
    mTimeframeConstraint = KCal::Period( mTimeframeConstraint.start(), newEnd );
    calculateConflicts();
}

void ConflictResolver::setLatestTime( const QTime& newTime )
{
    KDateTime newEnd = mTimeframeConstraint.end();
    newEnd.setTime( newTime );
    mTimeframeConstraint = KCal::Period( mTimeframeConstraint.start(), newEnd );
    calculateConflicts();
}

void ConflictResolver::setEarliestDateTime( const KDateTime& newDateTime )
{
    mTimeframeConstraint = KCal::Period( newDateTime, mTimeframeConstraint.end() );
}

void ConflictResolver::setLatestDateTime( const KDateTime& newDateTime )
{
    mTimeframeConstraint = KCal::Period( mTimeframeConstraint.start(), newDateTime );
}


void ConflictResolver::autoReload()
{
    mForceDownload = false;
    reload();
}

void ConflictResolver::reload()
{
    Q_FOREACH( FreeBusyItem * item, mFreeBusyItems ) {
        if ( mForceDownload ) {
            item->startDownload( mForceDownload );
        } else {
            updateFreeBusyData( item );
        }
    }
}

void ConflictResolver::triggerReload()
{
    mReloadTimer.start( 1000 );
}

void ConflictResolver::cancelReload()
{
    mReloadTimer.stop();
}

void ConflictResolver::manualReload()
{
    mForceDownload = true;
    reload();
}

int ConflictResolver::tryDate( KDateTime& tryFrom, KDateTime& tryTo )
{
    int conflicts_count = 0;
    Q_FOREACH( FreeBusyItem * currentItem, mFreeBusyItems ) {
        if ( !matchesRoleConstraint( currentItem->attendee() ) )
            continue;
        if ( !tryDate( currentItem, tryFrom, tryTo ) ) {
            ++conflicts_count;
        }
    }
    return conflicts_count;
}

bool ConflictResolver::tryDate( FreeBusyItem* attendee, KDateTime& tryFrom, KDateTime& tryTo )
{
    // If we don't have any free/busy information, assume the
    // participant is free. Otherwise a participant without available
    // information would block the whole allocation.
    KCal::FreeBusy *fb = attendee->freeBusy();
    if ( !fb ) {
        return true;
    }

    QList<KCal::Period> busyPeriods = fb->busyPeriods();
    for ( QList<KCal::Period>::Iterator it = busyPeriods.begin();
            it != busyPeriods.end(); ++it ) {
        if (( *it ).end() <= tryFrom || // busy period ends before try period
                ( *it ).start() >= tryTo ) { // busy period starts after try period
            continue;
        } else {
            // the current busy period blocks the try period, try
            // after the end of the current busy period
            int secsDuration = tryFrom.secsTo( tryTo );
            tryFrom = ( *it ).end();
            tryTo = tryFrom.addSecs( secsDuration );
            // try again with the new try period
            tryDate( attendee, tryFrom, tryTo );
            // we had to change the date at least once
            return false;
        }
    }
    return true;
}
bool ConflictResolver::findFreeSlot( const KCal::Period &dateTimeRange )
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
        int secs = tryFrom.secsTo( tryTo );
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
    if ( mSlotResolutionSeconds < 1 ) // fallback to default, if the user's value is invalid
        mSlotResolutionSeconds = DEFAULT_RESOLUTION_SECONDS;
    // calculate the length of the timeframe in terms of the amount of timeslots.
    // Example: 1 week timeframe, with resolution of 15 minutes
    //          1 week = 10080 minutes / 15 = 672 15 min timeslots
    //          So, the array would have a length of 672
    int range = begin.secsTo( end );
    range /=  mSlotResolutionSeconds;
    Q_ASSERT( range > 0 );
    // filter out attendees for which we don't have FB data
    // and which don't match the mandatory role contrstaint
    QList<FreeBusyItem* > filteredFBItems;
    foreach( FreeBusyItem * currentItem, mFreeBusyItems ) {
        if ( currentItem->freeBusy() && matchesRoleConstraint( currentItem->attendee() ) )
            filteredFBItems << currentItem;
    }

    // now we know the number of attendees we are calculating for
    const int number_attendees = filteredFBItems.size();
    Q_ASSERT( number_attendees > 0 );
    // this is a 2 dimensional array where the rows are attendees
    // and the columns are 0 or 1 denoting freee or busy respectively.
    QVector< QVector<int> > fbTable;

    // Explanation of the following loop:
    // iterate: through each attendee
    //    allocate: an array of length <range> and fill it with 0s
    //    iterate: through each attendee's busy period
    //       if: the period lies inside our timeframe
    //          then:
    //              calculate the array index within the timeframe range of the beginning of the busy peiod
    //              fill from that index until the period ends with a 1, representing busy
    //       fi
    //    etareti
    //    append the allocated array to <fbTable>
    // etareti
    foreach( FreeBusyItem * currentItem, filteredFBItems ) {
        Q_ASSERT( currentItem ); // sanity check
        QList<KCal::Period> busyPeriods = currentItem->freeBusy()->busyPeriods();
        QVector<int> fbArray( range );
        fbArray.fill( 0 ); // initialize to zero
        for ( QList<KCal::Period>::Iterator it = busyPeriods.begin();
                it != busyPeriods.end(); ++it ) {
            if ( it->end() >= begin && it->start() <= end ) {
                const int start_index = begin.secsTo( it->start() )  / mSlotResolutionSeconds;
                const int duration = it->start().secsTo( it->end() ) / mSlotResolutionSeconds;
                Q_ASSERT(( start_index + duration ) <= range ); // sanity check
                for ( int i = start_index; i <= start_index + duration; ++i )
                    fbArray[i] = 1;
            }
        }
        Q_ASSERT( fbArray.size() == range ); // sanity check
        fbTable.append( fbArray );
    }

    Q_ASSERT( fbTable.size() == number_attendees );

    // Create the composite array that will hold the sums for
    // each 15 minute timeslot
    QVector<int> summed( range );
    summed.fill( 0 ); // initialize to zero

    // Sum the columns of the table
    for ( int i = 0; i < number_attendees; ++i )
        for ( int j = 0; j < range; ++j ) {
            summed[j] += fbTable[i][j];
        }

    // Finally, iterate through the composite array locating contiguous free timeslots
    int free_count = 0;
    for ( int i = 0; i < range; ++i ) {
        // free timeslot encountered, increment counter
        if ( summed[i] == 0 ) {
            ++free_count;
        }
        if ( summed[i] != 0 || ( i == ( range - 1 ) && summed[i] == 0 ) ) {
            // current slot is not free, so push the previous free blocks OR we are in the last slot and it is free
            if ( free_count > 0 ) {
                int free_start_i;// start index of the free block
                int free_end_i; // end index of the free block
                if ( summed[i] == 0 ) {
                    // special case: we are on the last slot and it is free
                    //               so we want to include this slot in the free block
                    free_start_i = i - free_count + 1; // add one, to set us back inside the array (because free_count was incremented already this iteration)
                    free_end_i = i + 1; // add one to compensate for the fact that the array is 0 indexed
                } else {
                    free_start_i = i - free_count;
                    free_end_i = i - 1 + 1; // add one to compensate for the fact that the array is 0 indexed
                    // compiler will optmize out the -1+1, but I leave it here to make the reasoning apparent.
                }
                // convert from our timeslot interval back into to normal seconds
                // then calculate the date times of the free block based on
                // our initial timeframe
                KDateTime freeBegin = begin.addSecs( free_start_i * mSlotResolutionSeconds );
                KDateTime freeEnd = freeBegin.addSecs(( free_end_i - free_start_i ) * mSlotResolutionSeconds );
                // push the free block onto the list
                mAvailableSlots << KCal::Period( freeBegin, freeEnd );
                free_count = 0;
            }
        }
    }
#if 0
    //DEBUG, dump the arrays. very helpful for debugging
    QTextStream dump( stdout );
    dump << "    ";
    dump.setFieldWidth( 3 );
    for ( int i = 0; i < range; ++i ) // header
        dump << i;
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
    for ( int i = 0; i < range; ++i )
        dump << summed[i];
    dump << "\n";
#endif
}

void ConflictResolver::calculateConflicts()
{

//     int count = tryDate( mTimeframeConstraint.start(), mTimeframeConstraint.end() );
//     emit conflictsDetected( count );
//     kDebug() << "calculate conflicts" << count;
}

void ConflictResolver::setAllowedWeekdays( const QBitArray& weekdays )
{
    mWeekdays = weekdays;
}

void ConflictResolver::setMandatoryRoles( const QSet< KCal::Attendee::Role >& roles )
{
    mMandatoryRoles = roles;
}

void ConflictResolver::setAppointmentDuration( int seconds )
{
    mAppointmentDuration = seconds;
}

bool ConflictResolver::matchesRoleConstraint( const KCal::Attendee& attendee )
{
    return mMandatoryRoles.contains( attendee.role() );
}

KCal::Period::List ConflictResolver::availableSlots() const
{
    return mAvailableSlots;
}

void ConflictResolver::setResolution( int seconds )
{
    mSlotResolutionSeconds = seconds;
}

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

#include "attendeedata.h"
#include "freebusyitem.h"

#include <akonadi/kcal/freebusymanager.h> //krazy:exclude=camelcase since kdepim/akonadi
#include <akonadi/kcal/groupware.h> //krazy:exclude=camelcase since kdepim/akonadi

#include <KCal/FreeBusy>
#include <KDebug>


using namespace IncidenceEditorsNG;

ConflictResolver::ConflictResolver( QWidget *parentWidget, QObject* parent ): QObject( parent ), mParentWidget( parentWidget )
{
    Q_ASSERT( parentWidget );
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
    if( mManagerConnected )
        updateFreeBusyData( item );
}

void ConflictResolver::removeAttendee( const KCal::Attendee &attendee )
{
    FreeBusyItem *anItem = 0;
    for ( uint i = 0; i < mFreeBusyItems.count(); i++ ) {
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

void ConflictResolver::setDateTimes( const KDateTime& start, const KDateTime& end )
{
    mDtStart = start;
    mDtEnd = end;
    calculateConflicts();
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
bool ConflictResolver::findFreeSlot( KDateTime &dtFrom, KDateTime &dtTo )
{
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

void ConflictResolver::calculateConflicts()
{
    int count = tryDate( mDtStart, mDtEnd );
    emit conflictsDetected( count );
    kDebug() << "calculate conflicts" << count;
}


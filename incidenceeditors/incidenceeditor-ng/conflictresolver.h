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

#ifndef CONFLICTRESOLVER_H
#define CONFLICTRESOLVER_H

#include "attendeedata.h"

#include <KDateTime>

#include <QObject>
#include <QTimer>



namespace KCal
{
class FreeBusy;
class Attendee;
}

namespace IncidenceEditorsNG
{

class FreeBusyItem;

/**
 * Takes a list of attendees and event info (e.g., min time start, max time end)
 * fetches their freebusy information, then identifies conflicts and periods of non-conflict.
 *
 * It exposes these periods so another class can display them to the user and allow
 * them to choose a correct time.
 * @author Casey Link
 */
class ConflictResolver : public QObject
{
    Q_OBJECT
public:
    ConflictResolver( QWidget *parentWidget, QObject* parent = 0 );

    /**
     *  Add an attendee
     * The attendees free busy info will be fetched
     * and integrated into the resolver.
     */
    void insertAttendee( const KCal::Attendee &attendee );
    /**
     * Removes an attendee
     * The attendee will no longer be considered when
     * resolving conflicts
     * */
    void removeAttendee( const KCal::Attendee &attendee );
    /**
     * Clear all attendees
     **/
    void clearAttendees();

    /**
     * Returns whether the resolver contains the attendee
     */
    bool containsAttendee( const KCal::Attendee &attendee );

    /**
     * Queues a reload of free/busy data.
     * All current attendees will have their free/busy data
     * redownloaded from Akonadi.
     */
    void triggerReload();
    /**
     * cancel reloading
     * */
    void cancelReload();

signals:
    /**
     * Emitted when the user changes the start and end dateTimes
     * for the incidence.
     **/
    void dateTimesChanged( const KDateTime & newStart, const KDateTime & newEnd );

    /**
     * Emitted when there are conflicts
     * @param number the number of conflicts
     */
    void conflictsDetected( int number );

public slots:
    /**
     * Set the incidence's start and end datetimes
     * */
    void setDateTimes( const KDateTime & start, const KDateTime  & end );

protected:
    void timerEvent( QTimerEvent * );

private slots:
    void slotInsertFreeBusy( KCal::FreeBusy *fb, const QString &email );

    // Force the download of FB information
    void manualReload();
    // Only download FB if the auto-download option is set in config
    void autoReload();

    // connect to akonadi's free busy manager
    void setupManager();

private:
    void updateFreeBusyData( FreeBusyItem * );

    /**
      Finds a free slot in the future which has at least the same size as
      the initial slot.
    */
    bool findFreeSlot( KDateTime &dtFrom, KDateTime &dtTo );

    /**
      Checks whether the slot specified by (tryFrom, tryTo) is free
      for all participants. If yes, return true. The return value is the
      number of conflicts that were detected, and (tryFrom, tryTo) contain the next free slot for
      that participant. In other words, the returned slot does not have to
      be free for everybody else.
    */
    int tryDate( KDateTime &tryFrom, KDateTime &tryTo );

    /**
      Checks whether the slot specified by (tryFrom, tryTo) is available
      for the participant specified with attendee. If yes, return true. If
      not, return false and change (tryFrom, tryTo) to contain the next
      possible slot for this participant (not necessarily a slot that is
      available for all participants).
    */
    bool tryDate( FreeBusyItem *attendee, KDateTime &tryFrom, KDateTime &tryTo );

    void calculateConflicts();
    /**
     * Reload FB items
     * */
    void reload();

    KDateTime mDtStart, mDtEnd;
    QTimer mReloadTimer;
    bool mManagerConnected;
    bool mForceDownload;
    QList<FreeBusyItem*> mFreeBusyItems;
    QWidget *mParentWidget;
};

}

#endif // CONFLICTRESOLVER_H

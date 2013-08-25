/*
  Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef OPTIONS_H
#define OPTIONS_H

#include <Akonadi/Collection>
#include <QList>

class Options
{
public:

    enum SanityCheck {
        CheckNone,
        CheckEmptySummary,    // Checks for empty summary and description. In fix mode, it deletes them.
        CheckEmptyUid,        // Checks for an empty UID. In fix mode, a new UID is assigned.
        CheckEventDates,      // Check for missing DTSTART or DTEND. New dates will be assigned.
        CheckTodoDates,       // Check for recurring to-dos without DTSTART. DTDUE will be assigned to DTSTART, or current date if DTDUE is also invalid.
        CheckJournalDates,    // Check for journals without DTSTART
        CheckOrphans,         // Check for orphan to-dos. Will be unparented." <disabled for now>
        CheckDuplicateUIDs,   // Check for duplicated UIDs. Copies will be deleted if the payload is the same. Otherwise a new UID is assigned.
        CheckOrphanRecurId,   // Check if incidences with recurrence id belong to an nonexistant master incidence
        CheckStats,           // Gathers some statistics. No fixing is done.
        CheckCount            // For iteration purposes. Keep at end.
    };

    enum Action {
        ActionNone,
        ActionScan,
        ActionScanAndFix,
        ActionBackup
    };

    Options();

    void setAction(Action);
    Action action() const;

    /**
     * List of collections for backup or fix modes.
     * If empty, all collections will be considered.
     */
    QList<Akonadi::Collection::Id> collections() const;
    void setCollections(const QList<Akonadi::Collection::Id> &);
    bool testCollection(Akonadi::Collection::Id) const;

    bool stripOldAlarms() const;
    void setStripOldAlarms(bool strip);
private:
    QList<Akonadi::Collection::Id> m_collectionIds;
    Action m_action;
    bool m_stripOldAlarms;
};

#endif // OPTIONS_H

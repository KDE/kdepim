/*******************************************************************
 This file is part of KNotes.

 Copyright (c) 2004, Bo Thorsen <bo@sonofthor.dk>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 MA  02110-1301, USA.

 In addition, as a special exception, the copyright holders give
 permission to link the code of this program with any edition of
 the Qt library by Trolltech AS, Norway (or with modified versions
 of Qt that use the same license as Qt), and distribute linked
 combinations including the two.  You must obey the GNU General
 Public License in all respects for all of the code used other than
 Qt.  If you modify this file, you may extend this exception to
 your version of the file, but you are not obligated to do so.  If
 you do not wish to do so, delete this exception statement from
 your version.
*******************************************************************/

#ifndef RESOURCENOTES_H
#define RESOURCENOTES_H

#include <kresources/resource.h>
#include <kdepimmacros.h>
#include <libkcal/alarm.h>

class KConfig;
class KNotesResourceManager;

namespace KCal {
    class Journal;
}


/**
 * This class provides the interfaces for a KNotes resource. It makes use of
 * the kresources framework.
 *
 * \warning This code is still under heavy development. Don't expect source or
 *          binary compatibility in future versions.
 */
class KDE_EXPORT ResourceNotes : public KRES::Resource
{
public:
    ResourceNotes( const KConfig * );
    virtual ~ResourceNotes();

    /**
     * Load resource data.
     */
    virtual bool load() = 0;

    /**
     * Save resource data.
     */
    virtual bool save() = 0;

    virtual bool addNote( KCal::Journal * ) = 0;
    virtual bool deleteNote( KCal::Journal * ) = 0;

    virtual KCal::Alarm::List alarms( const QDateTime& from, const QDateTime& to ) = 0;

    void setManager( KNotesResourceManager *manager ) { mManager = manager; }
    KNotesResourceManager *manager() const            { return mManager; }

protected:
    KNotesResourceManager *mManager;
};


#endif // RESOURCENOTES_H

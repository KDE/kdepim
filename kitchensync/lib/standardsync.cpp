/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include "standardsync.h"

#include "syncer.h"
#include "syncee.h"

#include <kdebug.h>

using namespace KSync;

void StandardSync::syncToTarget( Syncee* source, Syncee* target,  bool override )
{
    kdDebug(5200) << "StandardSync::syncToTarget(): from: " << source->filename()
              << " to: " << target->filename() << "  override: "
              << override  << endl;

    SyncEntry *sourceEntry = source->firstEntry();
    while (sourceEntry) {
        SyncEntry *targetEntry = target->findEntry(sourceEntry->id());
        if (targetEntry) {
            // Entry already exists in target
            if (sourceEntry->equals(targetEntry)) {
                // Entries are equal, no action required
            } else {
                // Entries are different, resolve conflict
                if (override) {
                    // Force override
                    target->replaceEntry(targetEntry,sourceEntry);
                } else {
                    if (source->hasChanged(sourceEntry) &&
                        target->hasChanged(targetEntry)) {
                        // Both entries have changed
                        SyncEntry *result = deconflict(sourceEntry,targetEntry);
                        if (result == sourceEntry) {
                            target->replaceEntry(targetEntry,sourceEntry);
                        }
                    } else if (source->hasChanged(sourceEntry) &&
                               !target->hasChanged(targetEntry)) {
                        // take source entry
                        target->replaceEntry(targetEntry,sourceEntry);
                    } else if (!source->hasChanged(sourceEntry) &&
                               target->hasChanged(targetEntry)) {
                        // take target entry, no action required
                    }
                }
            }
        } else {
            // New entry
            target->addEntry(sourceEntry);
        }

        sourceEntry = source->nextEntry();
    }
}

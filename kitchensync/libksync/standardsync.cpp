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

void StandardSync::syncToTarget( Syncee *source, Syncee *target, bool override )
{
  kdDebug(5250) << "StandardSync::syncToTarget(): from: "
                << source->identifier()
                << " to: " << target->identifier() << "  override: "
                << override  << endl;

  SyncEntry *sourceEntry;
  for( sourceEntry = source->firstEntry(); sourceEntry;
       sourceEntry = source->nextEntry() ) {
    kdDebug(5250) << "SYNC: sourceEntry: " << sourceEntry->id()
                  << " (" << sourceEntry->name() << ")"
                  << " " << int( sourceEntry ) << endl;
    if ( sourceEntry->dontSync() ) {
      kdDebug(5250) << "SYNC:   source don't sync" << endl;
      continue;
    }
    SyncEntry *targetEntry = target->findEntry( sourceEntry->id() );
    if ( targetEntry ) {
      kdDebug(5250) << "SYNC:   targetEntry: " << targetEntry->id()
                    << " (" << targetEntry->name() << ")"
                    << " " << int( targetEntry ) << endl;
      if ( targetEntry->dontSync() ) {
        kdDebug(5250) << "SYNC:   target don't sync" << endl;
        continue;
      }
      kdDebug(5250) << "SYNC:   entry exists" << endl;
      // Entry already exists in target
      if ( sourceEntry->equals( targetEntry ) ) {
        // Entries are equal, no action required
        kdDebug(5250) << "SYNC:   equal" << endl;
      } else {
        kdDebug(5250) << "SYNC:   entries are different" << endl;
        // Entries are different, resolve conflict
        if ( override ) {
          // Force override
          target->replaceEntry( targetEntry, sourceEntry );
          kdDebug(5250) << "SYNC:   force replace" << endl;
        } else {
          if ( source->hasChanged( sourceEntry ) &&
               target->hasChanged( targetEntry ) ) {
            // Both entries have changed
            kdDebug(5250) << "SYNC:   Both have changed" << endl;
            SyncEntry *result = deconflict( sourceEntry, targetEntry );
            if ( !result ) {
              kdDebug(5250) << "SYNC:   no decision" << endl;
              sourceEntry->setDontSync( true );
              targetEntry->setDontSync( true );
            } else {
              if ( result == sourceEntry ) {
                kdDebug(5250) << "SYNC:   take source" << endl;
                target->replaceEntry( targetEntry, sourceEntry );
              } else {
                kdDebug(5250) << "SYNC:   take target" << endl;
              }
            }
          } else if ( source->hasChanged( sourceEntry ) &&
                      !target->hasChanged( targetEntry ) ) {
            // take source entry
            target->replaceEntry( targetEntry, sourceEntry );
            kdDebug(5250) << "SYNC:   source changed, target not" << endl;
            kdDebug(5250) << "SYNC:   Take source entry." << endl;
          } else if ( !source->hasChanged( sourceEntry ) &&
                      target->hasChanged( targetEntry ) ) {
            // take target entry, no action required
            kdDebug(5250) << "SYNC:   target changed, source not" << endl;
            kdDebug(5250) << "SYNC:   Take target entry." << endl;
          } else {
            kdDebug(5250) << "SYNC:   nothing has changed." << endl;
          }
        }
      }
    } else {
      // New entry
      target->addEntry( sourceEntry );
      kdDebug(5250) << "SYNC:   New entry." << endl;
    }
  }
}

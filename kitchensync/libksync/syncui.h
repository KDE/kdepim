/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
#ifndef KSYNC_SYNCUI_H
#define KSYNC_SYNCUI_H

namespace KSync {

class SyncEntry;

/**
   @short Syncing conflict resolution user interface.
   @author Cornelius Schumacher
   @see Syncer

   This class provides the abstract interface to a conflict resolution user
   interface. It is needed for cases, when a syncing process cannot resolve
   conflicts automatically. This is the case, when the same data entry has been
   changed in different data sets in an incompatible way.

   This class has to be implemented by a concrete subclass, which provides the
   actual user interface. While a GUI implementation, which provides interactive
   conflict resolution, is the most common implementation, there might also be
   use for a non-GUI or even non-interactive user interface.
*/
class SyncUi
{
  public:
    SyncUi();
    virtual ~SyncUi();

    /**
      Deconflict two conflicting @see SyncEntry objects. Returns the entry,
      which has been chosen by the user to take precedence over the other.

      The default implementation always returns 0, which should be interpreted
      to not sync the entries at all. Reimplement this function in a subclass to
      provide a more useful implementation to @see KSyncer.
    */
    virtual SyncEntry* deconflict( SyncEntry *syncEntry, SyncEntry *target );

    /**
      Confirm if the SyncEntry should be deleted. It gets called if one
      side was unchanged and the other deleted.
    */
    virtual bool confirmDelete( SyncEntry *syncEntry, SyncEntry *target );

    /**
      Inform the user that both items where deleted.
    */
    virtual void informBothDeleted( SyncEntry *syncEntry, SyncEntry *target );
};

}

#endif

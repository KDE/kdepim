/*
    This file is part of KitchenSync.

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
#ifndef KSYNC_SYNCALGORITHM_H
#define KSYNC_SYNCALGORITHM_H

namespace KSync {

class Syncee;
class SyncEntry;
class SyncUi;

/**
  A very simple sync interface for KitchenSync.
  It'll be possible to install different sync algorithms.
*/
class SyncAlgorithm
{
  public:
    /**
      Constructor.
    */
    SyncAlgorithm( SyncUi *ui = 0 ) { mUI = ui; }

    /**
      Destructor.
    */
    virtual ~SyncAlgorithm() {}

    /**
      Sync one Syncee object to another one. The data from the two Syncees is
      synced and written back to the target Syncee.

      @param syncee Source Syncee
      @param target Target Syncee
      @param override If set to true, override target in case of conflicts.
    */
    virtual void syncToTarget( Syncee *syncee, Syncee *target,
                               bool override = false ) = 0;

  protected:
    SyncEntry *deconflict( SyncEntry* syncEntry,  SyncEntry *target );
    bool confirmDelete( SyncEntry* syncEntry, SyncEntry* target );
    void informBothDeleted( SyncEntry* syncEntry, SyncEntry* target );

  private:
    SyncUi *mUI;

    class SyncAlgorithmPrivate;
    SyncAlgorithmPrivate* d;
};

}

#endif

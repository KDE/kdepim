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
#ifndef KSYNC_SYNCER_H
#define KSYNC_SYNCER_H

#include <qstring.h>
#include <qptrlist.h>
#include <kdepimmacros.h>

namespace KSync {

class SyncAlgorithm;
class SyncUi;
class Syncee;

/**
  @short This class provides syncing of sets of data entries.
  @author Cornelius Schumacher, zecke
  @see SyncEntry, Syncee, SyncUi

  The Syncer class provides the top level framework for syncing. It implements
  the actual syncing algorithm, which operates on data objects implementing the
  @ref Syncee and @ref SyncEntry interfaces. By this mechanism the syncing
  algorithm is decoupled from the concrete type of data and has to be implemented
  only once.

  To perform a syncing process you have to create objects of the concrete
  subclasses of @ref Syncee, representing the type of the data to be synced.
  They are added to Syncer with the function @ref addSyncee(). When all data
  sets to be synced are added, call the sync() functions to perform the actual
  syncing.

  For conflict resolution, a user interface is needed. This has to be a subclass
  of @ref SyncUi and is provided, when constructing a Syncer instance.
*/
class KDE_EXPORT Syncer
{
  public:
    /**
      Create a Syncer instance. You have to provide an instance of a conflict
      resolution user interface, which is used to resolve conflicts in the
      synced data, which cannot be resolved automatically. The UI does not
      necessarily have to be interactive.
    */
    Syncer( SyncUi *ui = 0, SyncAlgorithm *algorithm = 0 );
    virtual ~Syncer();

    /**
      Sets the syncing algorithm which is used to perform the sync.
    */
    void setSyncAlgorithm( SyncAlgorithm * );

    /**
      Sets user interface handler which is used for resolving conflicts.
    */
    void setSyncUi( SyncUi * );

    /**
      Add a data set, which is to be synced.
    */
    void addSyncee( Syncee * );

    /**
      Removes all Syncee objects added with addSyncee from the Syncer
    */
    void clear();

    /**
      Sync all data sets. After execution of this functions all data sets,
      which have been added to Syncer contain the same set of data.

      This function might call conflict resolution functions of the @ref
      SyncUi object.
    */
    void sync();

    /**
      Sync all data sets with a target data sets. After execution of this
      function the target @ref Syncee data set contains a combination of all
      data sets added to Syncer. The added data sets are not changed.

      This function might call conflict resolution functions of the @ref
      SyncUi object.
    */
    void syncAllToTarget( Syncee *target, bool writeback = false);

    /**
      Sync one specific data set to a target data set. After execution of this
      function the target contains the combination of the two data sets. Only
      the target is changed.

      This function might call conflict resolution functions of the @ref
      SyncUi object.
    */
    void syncToTarget( Syncee *syncee, Syncee *target, bool override = false );

  private:
    QPtrList<Syncee> mSyncees;

    SyncUi *mUi;
    SyncAlgorithm *mAlgorithm;

    bool mOwnUi;
    bool mOwnAlgorithm;

    class SyncerPrivate;
    SyncerPrivate *d;
};

}

#endif

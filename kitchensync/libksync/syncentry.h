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
#ifndef KSYNC_SYNCENTRY_H
#define KSYNC_SYNCENTRY_H

#include <qstring.h>
#include <qptrlist.h>
#include <kdepimmacros.h>

namespace KPIM {
class DiffAlgo;
}

namespace KSync {
class Merger;
class Syncee;

/**
  @short An entry of a dataset which is to be synced.
  @author Cornelius Schumacher
  @see Syncee, Syncer

  The SyncEntry class represents the basic unit of syncing. It provides an
  interface for identifying and comparing entries, which has to be
  implemented by concrete subclasses. This makes it possible to
  operate with one synchronisation algorithm on different Syncees.

  SyncEntry objects are collected by a @see Syncee objects.
*/
class KDE_EXPORT SyncEntry
{
  public:
    typedef QPtrList<SyncEntry> PtrList;

    enum Equalness { Different = -1, Equal = 0, EqualButModifiedThis = 1,
                     EqualButModifiedOther = 2, EqualButModifiedBoth = 3 };

    enum Status { Undefined = -1, Added = 0, Modified=1, Removed = 2 };

    /**
      This is the basic constructor of a SyncEntry.
      Every SyncEntry should have a parent Syncee where it belongs to.
    */
    SyncEntry( Syncee *parent );
    SyncEntry( const SyncEntry & );
    virtual ~SyncEntry();

    /**
      Return a string describing the type of the entry
    */
    QString type() const;

    /**
      Return a string describing this entry. This is presented to the user as
      identifier for the entry, when user interaction is required.
    */
    virtual QString name() = 0;

    /**
      Return a unique id. This is used to uniquely identify the entry. Two
      entries having the same id are considered to be two variants of the same
      entry. No two entries of the same @see KSyncee data set must have the same
      id.
    */
    virtual QString id() = 0;


    /**
       Set the ID of the underlying data. This is needed for example if the UID is not trusted
       and could come from a device.
     */
    virtual void setId( const QString& id );

    /**
      Return a time stamp representing the time of the last change. This is only
      used to compare, if an entry has changed or not. It is not used to define
      an order of changes. If an entry has been copied from one KSyncee data set
      to another KSyncee data set, the timestamp has to be the same on both
      entries. If the user has changed the entry in one data set the timestamp
      has to be different.

      Return QString::null, if there is no timestamp available. This means that
      the user has to manually select which entry is the new one.
    */
    virtual QString timestamp() = 0;

    /**
      Return, if the two entries are equal. Two entries are considered to be
      equal, if they contain exactly the same information, including the same id
      and timestamp.
    */
    virtual bool equals( SyncEntry * ) = 0;

    /**
      For future versions match should try to match one SyncEntry
      with another.

      @param entry The entry to be matched
      @return Return -1 if entry is from a different type than this entry
              or the percentage of equality. Or -2 if not implemented
    */
    virtual int match( SyncEntry *entry );

    /**
      Compares one SyncEntry to another. This functions differs from
      equals and match in some ways.
      it returns 0 if both are equal, -1 if not equal at all, or
      the state of equalnes
      Equal or EqualModifiedThis, EqualModifiedOther, EqualModifiedBoth
      -2 if not implemented
    */
    virtual int compareTo( SyncEntry *entry );

    /**
      The status of this SyncEntry
      either Undefined, Added, Modified or Removed
    */
    virtual int state() const;

    // TODO ### discuss for during sync added or such
    void setSyncState(int);
    int syncState()const;

    /**
      Convience functions for the state of an Entry
    */
    virtual bool wasAdded() const;

    /**
      Convience function for the state of an Entry
      modified
    */
    virtual bool wasModified() const;

    /**
      Convience function for the state of an Entry
    */
    virtual bool wasRemoved() const;

    /**
      Sets the stae of this SyncEntry
    */
    virtual void setState( int state = Undefined );


    /**
      Creates an exact copy of the this SyncEntry
      deleting the original is save and does not influence
      the clone.
      Syncee will be unset and SyncStates will be copied over
      as well.
    */
    virtual SyncEntry *clone() = 0;

    /**
      Set the @see Syncee data set, the entry belongs to.
    */
    void setSyncee( Syncee * );

    /**
      Return the @see Syncee data set, the entry belongs to.
    */
    Syncee *syncee()const;

    /**
      Set if the entry should be synced or not.
    */
    void setDontSync( bool );

    bool dontSync() const;

    bool mergeWith(SyncEntry *other);

    /**
       Returns the diffing algorithm which is used to present the differences between
       the two SyncEntries when a conflict occurs.
     */
    virtual KPIM::DiffAlgo* diffAlgo( SyncEntry*, SyncEntry* );

 protected:
    void setType(const QString&);

 protected:
    Merger* merger()const;

  private:
    int mState;
    int mSyncState;
    Syncee *mSyncee;
    bool mDontSync : 1;
    QString mType;

    class SyncEntryPrivate;
    SyncEntryPrivate *d;
};

}

#endif

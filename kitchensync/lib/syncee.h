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
#ifndef KSYNC_SYNCEE_H
#define KSYNC_SYNCEE_H

#include <qbitarray.h>
#include <qobject.h>
#include <qmap.h>
#include <qstring.h>
#include <qptrlist.h>

#include "kontainer.h"
#include "syncentry.h"

class KSimpleConfig;

namespace KSync {

/**
  @short A data set to be synced.
  @author Cornelius Schumacher, zecke
  @see SyncEntry, Syncer

  This class represents a data set of SyncEntries. During a syncing process,
  two or more Syncees are synced. After syncing they are  equal, that
  means they should contain the same set of SyncEntries. Choices by the user
  can lead to deviations from complete equality.

  The Syncee class provides an interface, which has to be implemented by
  concrete subclasses.

  Further a Syncee can store a BitMap on what a 'Filler' of the Syncee supports.
  For example Device B got a todolist but has only 3 Attributes.
     Attribute 1: Description
     Attribute 2: Completed
     Attribute 3: DueDate

  The KDE todolist got roughly 10-15 Attributes
  So when syncing B with KDE, where B would replace KDE Records would lead
  to loss of up to 12 attributes.
  This will be avoided by a merge before a replaceEntry operation.
  This way B will take presedence on the 3 Attributes but we won't lose
  the additional attributes.
  By default the support map of a Syncee is set to supports all..


  @ref Syncer operates on Syncee objects.
*/
class Syncee
{
  public:
    typedef QPtrList<Syncee> PtrList;
    enum SyncMode { MetaLess=0, MetaMode=2 };

    Syncee( uint supportSize = 0);
    virtual ~Syncee();

    /**
      Return the first @ref SyncEntry object of the data set. This function
      together with @ref nextEntry() is used to iterate through all entries of a
      Syncee data set.
    */
    virtual SyncEntry *firstEntry() = 0;

    /**
      Return the next @ref SyncEntry object of the data set. This function
      together with @ref firstEntry() is used to iterate through all entries of a
      Syncee data set.
    */
    virtual SyncEntry *nextEntry() = 0;

    /**
     * The type of the Syncee
     */
    virtual QString type() const = 0;
    /**
      Find an entry identified by a unique id. See @ref SyncEntry::id().
      @param id the Id to be found
    */
    virtual SyncEntry *findEntry(const QString &id);

    /**
      Add a @ref SyncEntry object to this data set. Ownership of the object
      remains with the caller.
    */
    virtual void addEntry(SyncEntry *) = 0;
    /**
      Remove a @ref SyncEntry. The entry is removed from the data set, but the
      object is not deleted.
    */
    virtual void removeEntry(SyncEntry *) = 0;

    /**
      Replace an entry of the data set by another. Ownership of the objects is
      handled as with the @ref addEntry() and @ref removeEntry() functions.
    */
    void replaceEntry(SyncEntry *oldEntry, SyncEntry *newEntry);

    /**
      Set the filename, the data set is read from and written to.
    */
    void setFilename(const QString &);
    /**
      Return the filename, the data set is read from and written to.
    */
    QString filename();

    /**
      Return the name of a config file, which is used to store status
      information about the data set.
    */
    QString statusLogName();

    /**
      Load the data set from the file with them name @ref filename().

      @return true, if loading is successfull, otherwise false.
    */
    bool load();
    /**
      Save the data set to the file with them name @ref filename().

      @return true, if loading is successfull, otherwise false.
    */
    bool save();

    /**
      Read the data set from disk from the file with the name @ref filename().
      This function has to be reimplemented by concrete subclasses to provide
      the actual reading from disk.

      @return true, if reading is successfull, otherwise false.
    */
    virtual bool read() = 0;

    /**
      Write the data set to disk to the file with the name @ref filename().
      This function has to be reimplemented by concrete subclasses to provide
      the actual writing to disk.

      @return true, if writing is successfull, otherwise false.
    */
    virtual bool write() = 0;

    /**
      Write the status log file with the name @ref statusLogName().
    */
    void writeLog();

    /**
      Return, if the given @ref SyncEntry has changed since the last syncing.
      This information is retrieved by comparing the timestamps from the log
      file and the freshly read data set.
    */
    bool hasChanged(SyncEntry *);

    /**
      Returns if hasChanged and the state of change
      Undefined, Added, Modified,Removed
    */
    virtual int modificationState( SyncEntry * entry) const;

    /**
      Returns the syncMode of this Syncee. The syncMode determines the later
      used synchronisation algorithm for the best results.
    */
    virtual int syncMode() const;

    /**
      Sets the syncMode of this Syncee.
    */
    virtual void setSyncMode( int mode = MetaLess );

    /**
      Set if it's syncing for the first time.
    */
    virtual void setFirstSync( bool firstSync = true );

    /**
     If it is syncing for the first time.
    */
    virtual bool firstSync() const;

    /**
      For Meta Syncing you easily know what was changed
      from one sync to another. The gathering of these informations
      can be made by Syncee itself or by what the developer wants
      The following three methods are convience functions to make things
      more smooth later
    */

    /**
      What was added?
    */
    virtual SyncEntry::PtrList added() = 0;

    /**
      What was modified?
    */
    virtual SyncEntry::PtrList modified() = 0;

    /**
      What was removed?
    */
    virtual SyncEntry::PtrList removed() = 0;

    /**
      For some parts of memory management it would be good to
      deal with clones. This creates a direct clone of the Syncee
    */
    virtual Syncee *clone() = 0;

    /**
       A KSyncEntry is able to store the relative ids
       @param  type The type of the id for example todo, kalendar...
       @param  konnectorId The original id of the Entry on konnector side
       @param  kdeId Is the id KDE native classes are assigning
       Example:
       type = todo
       konnector id  = -1345678
       KDE ID = KORG-234575464
    */
    void insertId( const QString &type,
                   const QString &konnectorId,
                   const QString &kdeId );


    /**
      When dealing with special uid Konnector-
      You might want a new uid to be generated. To later find
      an Entry again you'll need this map
    */
    virtual QString newId()const;
    /**
      @param type The type for the ids to returned
      @return the ids as QValueList
    */
    Kontainer::ValueList ids(const QString &type )const;

    /**
      @return all ids
    */
    QMap<QString,  Kontainer::ValueList > ids()const;

    /**
      Set what the Syncee supports.
    */
    virtual void setSupports( const QBitArray& );

    /**
      Returns attributes supported by the Device.
    */
    virtual QBitArray bitArray()const;


    /**
      Set the source of this Syncee. The string may be presented to the user by
      the conflict resolver
    */
    void setSource( const QString &src );

    /**
      Eeturns the source of this syncee or QString::null if not set.
    */
    QString source() const;


    /**
      Convenience function to figure if a specific attribute is supported.
    */
    inline bool isSupported( uint Attribute ) const;


    // a bit hacky
    /**
     * When syncing two iCalendar the UIDs are garantuued to be global
     * and you may not change these values at all.
     * But there are cases in firstSync where you would like to create
     * a bound between one id and another
     */
    virtual bool trustIdsOnFirstSync() const;

  private:
    QMap<QString,  Kontainer::ValueList > mMaps;
    int mSyncMode;
    bool mFirstSync : 1;
    QString mFilename;
    KSimpleConfig *mStatusLog;
    QBitArray mSupport;
    QString mName;
    class SynceePrivate;
    SynceePrivate *d;
};

}

#endif

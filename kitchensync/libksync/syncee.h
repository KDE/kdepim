/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002,2004 Holger Hans Peter Freyther <freyther@kde.org>

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

#include <kontainer.h>
#include <syncentry.h>

class KSimpleConfig;

namespace KSync {

class Merger;
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

  Further you can set a Merger on the  Syncee to show which attributes are known
  to you.
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

  @see Merger
  @ref Syncer operates on Syncee objects.
*/
class Syncee
{
  public:
    /**
     * Normally firstSync is on
     */
    Syncee( Merger* merger );
    virtual ~Syncee();

    /**
      Reset Syncee to initial state. This is called when the data the Syncee
      operates on is changed externally, i.e. without using the Syncees
      addEntry() removeEntry(), replaceEntry() methods.
    */
    // TODO: It might be better if the Syncee would transparently operate on the
    // underlying data without requiring the reset() call.
    virtual void reset() {}

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
     * The type of the Syncee. 
     * Sometimes it is not possible to use dynamic_cast and this way a type is nice
     * to have;
     */   
    QString type() const;
    
    /**
      Find an entry identified by a unique id. See @ref SyncEntry::id().
      @param id the Id to be found
    */
    virtual SyncEntry *findEntry( const QString &id );

    /**
      Add a @ref SyncEntry object to this data set. Ownership of the object
      is transfered and the SyncEntry now belongs to this Syncee. Use
      KSync::SyncEntry::clone() to create an exact copy of a KSync::SyncEntry.
      
      \sa KSync::SyncEntry::clone
    */
    virtual void addEntry( SyncEntry * ) = 0;

    /**
      Remove a @ref SyncEntry. The entry is removed from the data set, but the
      object is not deleted.
    */
    virtual void removeEntry( SyncEntry * ) = 0;

    /**
      Replace an entry of the data set by another. Ownership of the objects is
      handled as with the @ref addEntry() and the old entry will be deleted
      internally.
    */
    void replaceEntry( SyncEntry *oldEntry, SyncEntry *newEntry );

    /**
      Set identifier which can be used to uniquely identify the Syncee. A Syncee
      with empty identifier is invalid. Without identifier the sync log can't be
      written.
    */
    void setIdentifier( const QString &identifier );

    /**
      Return the identifier which can be used to uniquely identify the Syncee
      object. As long as the identifier is empty the Syncee doesn't have valid
      data.
    */
    QString identifier()const;

    /**
      Return if the Syncee is valid. If a Syncee is invalid it means that it
      doesn't have any valid data, e.g. because the Konnector doesn't support
      this type of data.

      By default the Syncee isn't valid if the identifier is empty.
    */
    virtual bool isValid();

    /**
      Returns if hasChanged and the state of change
      Undefined, Added, Modified,Removed
    */
    virtual int modificationState( SyncEntry * entry) const;


    /**
      For Meta Syncing you easily know what was changed
      from one sync to another. The gathering of these informations
      can be made by Syncee itself or by what the developer wants
      The following three methods are convience functions to make things
      more smooth later
    */
    //{

    /**
      What was added? This uses firstEntry() nextEntry internally
      be aware of it.
    */
    virtual SyncEntry::PtrList added();
    /**
      What was modified? This uses firstEntry() nextEntry internally
      be aware of it.
    */
    virtual SyncEntry::PtrList modified();
    /**
      What was removed? This uses firstEntry() nextEntry internally
      be aware of it.
    */
    virtual SyncEntry::PtrList removed();

    //}
    /**
      For some parts of memory management it would be good to
      deal with clones. This creates a direct clone of the Syncee
    */
// Syncees are owned by the Konnectors, we won't allow to steal them by cloning.
//    virtual Syncee *clone() = 0;

    /**
       A SyncEntry is able to store the relative ids
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
    virtual QString generateNewId() const;
    /**
      @param type The type for the ids to returned
      @return the ids as QValueList
    */
    Kontainer::ValueList ids( const QString &type ) const;

    /**
      @return all ids
    */
    QMap<QString,Kontainer::ValueList> ids() const;

    /**
     * The Merger set in either the Constructor or by a call
     * to setMerger.
     * Merger could be null, this normally indicates that all 
     * attributes are supported.
     * 
     * @see Merger
     */
    Merger* merger()const;

    /**
     * Set the Merger. Ownership is not transfered and you can use the
     * same merger on many different Syncees. You can also unset it (passing 0l)
     */
    void setMerger( Merger *merger = 0 );
   
    /**
      Set the source of this Syncee. The string may be presented to the user by
      the conflict resolver
    */
    void setTitle( const QString &src );

    /**
      Returns the source of this syncee or QString::null if not set.
    */
    QString title() const;


   
    /**
     * When syncing two iCalendar the UIDs are garantuued to be global
     * and you may not change these values at all.
     * But there are cases in firstSync where you would like to create
     * a bound between one id and another
     */
    virtual bool trustIdsOnFirstSync() const;

    virtual bool writeBackup( const QString &filename ) = 0;
    
    virtual bool restoreBackup( const QString &filename ) = 0;

 protected:
    /**
     * The Syncee Implementation can set the type of the Syncee. This is needed
     * to identify and cast the syncee
     */
    void setType(const QString& type);

    SyncEntry::PtrList find(int state);

  private:
    QMap<QString,Kontainer::ValueList> mMaps;      
    QString mIdentifier;
    Merger* mMerger;
    QString mTitle;   
    QString mType;
    class SynceePrivate;
    SynceePrivate *d;
};

}

#endif

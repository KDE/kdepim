/*
    This file is part of ksync.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KSYNCER_H
#define KSYNCER_H

#include <qstring.h>
#include <qptrlist.h>
#include <kdemacros.h>

class KSimpleConfig;

class KSyncUi;
class KSyncee;

/**
  @short An entry of a dataset which is to be synced.
  @author Cornelius Schumacher
  @see Ksyncee, KSyncer
  
  The KSyncEntry class represents the basic unit of syncing. It provides an
  interface for identifying and comparing entries, which has to be implemented
  by concrete subclasses.
  
  KSyncEntry objects are collected by @ref KSyncee objects.
*/
class KSyncEntry
{
  public:
    KSyncEntry();
    virtual ~KSyncEntry();
  
    /**
      Return a string describing this entry. This is presented to the user as
      identifier for the entry, when user interaction is required.
    */
    virtual QString name() = 0;
    /**
      Return a unique id. This is used to uniquely identify the entry. Two
      entries having the same id are considered to be two variants of the same
      entry. No two entries of the same @ref KSyncee data set must have the same
      id.
    */
    virtual QString id() = 0;
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
    virtual bool equals(KSyncEntry *) = 0;

    /**
      Set the @ref KSyncee data set, the entry belongs to.
    */    
    void setSyncee(KSyncee *);
    /**
      Return the @ref KSyncee data set, the entry belongs to.
    */
    KSyncee *syncee();
    
  private:
    KSyncee *mSyncee;
};

/**
  @short A data set to be synced.
  @author Cornelius Schumacher
  @see KSyncEntry, KSyncer
  
  This class represents a data set of KSyncEntries. During a syncing process,
  two or more KSyncees are synced. After syncing they should be equal, that
  means they should contain the same set of KSyncEntries. Choices by the user
  can lead to deviations from complete equality.

  The KSyncee class provides an interface, which has to be implemented by
  concrete subclasses.
  
  @ref KSyncer operates on KSyncee objects.
*/
class KDE_EXPORT KSyncee
{
  public:
    KSyncee();
    virtual ~KSyncee();
    
    /**
      Return the first @ref KSyncEntry object of the data set. This function
      together with @ref nextEntry() is used to iterate through all entries of a
      KSyncee data set.
    */
    virtual KSyncEntry *firstEntry() = 0;
    /**
      Return the next @ref KSyncEntry object of the data set. This function
      together with @ref firstEntry() is used to iterate through all entries of a
      KSyncee data set.
    */    
    virtual KSyncEntry *nextEntry() = 0;
    
    /**
      Find an entry identified by a unique id. See @ref KSyncEntry::id().
    */
    virtual KSyncEntry *findEntry(const QString &id);
    
    /**
      Add a @ref KSyncEntry object to this data set. Ownership of the object
      remains with the caller.
    */
    virtual void addEntry(KSyncEntry *) = 0;
    /**
      Remove a @ref KSyncEntry. The entry is removed from the data set, but the
      object is not deleted.
    */
    virtual void removeEntry(KSyncEntry *) = 0;

    /**
      Replace an entry of the data set by another. Ownership of the objects is
      handled as with the @ref addEntry() and @ref removeEntry() functions.
    */
    void replaceEntry(KSyncEntry *oldEntry,KSyncEntry *newEntry);

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
      
      @return true, if loading is successful, otherwise false.
    */
    bool load();
    /**
      Save the data set to the file with them name @ref filename().
      
      @return true, if loading is successful, otherwise false.
    */
    bool save();

    /**
      Read the data set from disk from the file with the name @ref filename().
      This function has to be reimplemented by concrete subclasses to provide
      the actual reading from disk.

      @return true, if reading is successful, otherwise false.
    */
    virtual bool read() = 0;
    /**
      Write the data set to disk to the file with the name @ref filename().
      This function has to be reimplemented by concrete subclasses to provide
      the actual writing to disk.

      @return true, if writing is successful, otherwise false.
    */
    virtual bool write() = 0;

    /**
      Write the status log file with the name @ref statusLogName().
    */
    void writeLog();

    /**
      Return, if the given @ref KSyncEntry has changed since the last syncing.
      This information is retrieved by comparing the timestamps from the log
      file and the freshly read data set.
    */
    bool hasChanged(KSyncEntry *);
    
  private:
    QString mFilename;
    KSimpleConfig *mStatusLog;
};

/**
  @short This class provides syncing of sets of data entries.
  @author Cornelius Schumacher
  @see KSyncEntry, KSyncee, KSyncUi
  
  The KSyncer class provides the top level framework for syncing. It implements
  the actual syncing algorithm, which operates on data objects implementing the
  @ref KSyncee and @ref KSyncEntry interfaces. By this mechanism the syncing
  algorith is decoupled from the concrete type of data and has to be implemented
  only once.
  
  To perform a syncing process you have to create objects of the concrete
  subclasses of @ref KSyncee, representing the type of the data to be synced.
  They are added to KSyncer with the function @ref addSyncee(). When all data
  sets to be synced are added, call the sync() functions to perform the actual
  syncing.
  
  For conflict resolution, a user interface is needed. This has to be a subclass
  of @ref KSyncUi and is provided, when constructing a Ksyncer instance.
*/
class KDE_EXPORT KSyncer
{
  public:
    /**
      Create a KSyncer instance. You have to provide an instance of a conflict
      resolution user interface, which is used to resolve conflicts in the
      synced data, which cannot be resolved automatically. The UI does not
      necessarily have to be interactive.
    */
    KSyncer(KSyncUi *ui=0);
    virtual ~KSyncer();
    
    /**
      Add a data set, which is to be synced.
    */
    void addSyncee(KSyncee *);
    
    /**
      Sync all data sets. After execution of this functions all data sets,
      which have been added to KSyncer contain the same set of data.
      
      This function might call conflict resolution functions of the @ref
      KSyncUi object.
    */
    void sync();
    /**
      Sync all data sets with a target data sets. After execution of this
      function the target @ref KSyncee data set contains a combination of all
      data sets added to KSyncer. The added data sets are not changed.
      
      This function might call conflict resolution functions of the @ref
      KSyncUi object.
    */
    void syncAllToTarget(KSyncee *target,bool writeback=false);
    /**
      Sync one specific data set to a target data set. After execution of this
      function the target contains the combination of the two data sets. Only
      the target is changed.
      
      This function might call conflict resolution functions of the @ref
      KSyncUi object.
    */
    void syncToTarget(KSyncee *syncee, KSyncee *target, bool override=false);
    
  private:
    QPtrList<KSyncee> mSyncees;
    KSyncUi *mUi;
};

#endif

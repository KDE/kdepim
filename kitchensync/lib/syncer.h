#ifndef KSYNCER_H
#define KSYNCER_H
// $Id$

#include <qbitarray.h>
#include <qobject.h>
#include <qmap.h>
#include <qstring.h>
#include <qptrlist.h>

#include "kontainer.h"

class KSimpleConfig;

namespace KSync {

class SyncAlgorithm;
class SyncUi;
class Syncee;

/**
  @short An entry of a dataset which is to be synced.
  @author Cornelius Schumacher
  @see Syncee, Syncer

  The SyncEntry class represents the basic unit of syncing. It provides an
  interface for identifying and comparing entries, which has to be
  implemented by concrete subclasses. This makes it possible to
  operate with one synchronisation
  algorithm on different Syncee's.

  SyncEntry objects are collected by a @ref Syncee objects.
*/
class SyncEntry
{
  public:
    typedef QPtrList<SyncEntry> PtrList;
    enum Equalness { Different =-1, Equal=0,  EqualButModifiedThis=1,
                     EqualButModifiedOther=2, EqualButModifiedBoth=3 };

    enum Status { Undefined =-1, Added = 0, Modified=1, Removed=2 };

    /**
     * This is the basic c'tor of a Syncee.
     * Every SyncEntry should have a parent Syncee
     * where it belongs to.
     */
    SyncEntry(Syncee* parent = 0);
    SyncEntry( const SyncEntry& );
    virtual ~SyncEntry();

    /**
     * Return a string describing the type of the entry
     */
    virtual QString type()const = 0;

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
    virtual bool equals(SyncEntry *) = 0;

    /**
     * For future versions match should try to match one SyncEntry
     * with another.
     * @param entry The entry to be matched
     * @return Return -1 if entry is from a different type than this entry
     * or the percentage of equality. Or -2 if not implemented
     */
    virtual int match(SyncEntry *entry );

    /**
     * Compares one SyncEntry to another. This functions differs from
     * equals and match in some ways.
     * it returns 0 if both are equal, -1 if not equal at all, or
     * the state of equalnes
     * Equal or EqualModifiedThis, EqualModifiedOther, EqualModifiedBoth
     * -2 if not implemented
     */
    virtual int compareTo( SyncEntry* entry );

    /**
     * The status of this SyncEntry
     * either Undefined, Added, Modified or Removed
     */
    virtual int state()const;

    /**
     * Convience functions for the state of an Entry
     */
    virtual bool wasAdded()const;

    /**
     * Convience function for the state of an Entry
     * modified
     */
    virtual bool wasModified()const;

    /**
     * Convience function for the state of an Entry
     */
    virtual bool wasRemoved()const;

    /**
     * Sets the stae of this SyncEntry
     */
    virtual void setState( int state = Undefined );

    /**
     * sometimes its nice to know if a Entry
     * was added or modefied during sync
     */
    virtual void setSyncState( int state = Undefined );

    /**
     * returns the sync state of
     * this entry
     */
    virtual int syncState()const;


    /**
     * Creates an exact copy of the this SyncEntry
     * deleting the original is save
     */
    virtual SyncEntry* clone() = 0;

    /**
       Set the @ref Syncee data set, the entry belongs to.
    */
    void setSyncee(Syncee *);

    /**
       Return the @ref KSyncee data set, the entry belongs to.
    */
    Syncee *syncee();

    /**
     * Merges two sync entries where ever one entry
     * does not support one specefic attribute
     */
    virtual bool mergeWith( SyncEntry* );



  private:
    int mState;
    int mSyncState;
    Syncee *mSyncee;
    class SyncEntryPrivate;
    SyncEntryPrivate *d;
};

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
     * Returns if hasChanged and the state of change
     * Undefined, Added, Modified,Removed
     */
    virtual int modificationState( SyncEntry* entry) const;

    /**
     * Returns the syncMode of this Syncee
     * The syncMode determines the later used
     * synchronisation algorithm for the best results.
     */
    virtual int syncMode()const;

    /**
     * Sets the syncMode of this Syncee
     */
    virtual void setSyncMode( int mode = MetaLess );

    /**
     * set if it's syncing for the first time
     */
    virtual void setFirstSync( bool firstSync = true );

    /**
     * if is syncing for the first time
     */
    virtual bool firstSync() const;

    /**
     * For Meta Syncing you easily know what was changed
     * from one sync to another. The gathering of these informations
     * can be made by Syncee itself or by what the developer wants
     * The following three methods are convience functions to make things
     * more smooth later
     */

    /**
     * What was added?
     */
    virtual SyncEntry::PtrList added() =0;

    /**
     * what was modified?
     */
    virtual SyncEntry::PtrList modified() = 0;

    /**
     * and what was removed?
     */
    virtual SyncEntry::PtrList removed() = 0;

    /**
     * For some parts of memory management it would be good to
     * deal with clones. This creates a direct clone of the Syncee
     */
    virtual Syncee* clone() = 0;

    /**
     * When dealing with special uid Konnector-
     * You might want a new uid to be generated. To later find
     * an Entry again you'll need this map
     */
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


    virtual QString newId()const;
    /**
     * @param type The type for the ids to returned
     * @return the ids as QValueList
     */
    Kontainer::ValueList ids(const QString &type )const;

    /**
     * @return all ids
     */
    QMap<QString,  Kontainer::ValueList > ids()const;

    /**
     * set what the Syncee supports
     */
    virtual void setSupports( const QBitArray& );

    /**
     * returns of the Device supported
     * Attributes
     */
    virtual QBitArray bitArray()const;


    /**
     * convience function to figure
     * if a specefic attribute is supported
     */
    inline bool isSupported( uint Attribute )const;


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
    class SynceePrivate;
    SynceePrivate* d;
};

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
class Syncer
{
  public:
    /**
      Create a Syncer instance. You have to provide an instance of a conflict
      resolution user interface, which is used to resolve conflicts in the
      synced data, which cannot be resolved automatically. The UI does not
      necessarily have to be interactive.
    */
    Syncer(SyncUi *ui=0, SyncAlgorithm *iface= 0 );
    virtual ~Syncer();

    /**
     * installs a different syncing implementation
     */
    void setSyncAlgorithm( SyncAlgorithm* );


    /**
      Add a data set, which is to be synced.
    */
    void addSyncee(Syncee *);

    /**
     * removes all Syncee added with addSyncee from the Syncer
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
    void syncAllToTarget(Syncee *target,bool writeback=false);

    /**
      Sync one specific data set to a target data set. After execution of this
      function the target contains the combination of the two data sets. Only
      the target is changed.

      This function might call conflict resolution functions of the @ref
      SyncUi object.
    */
    void syncToTarget(Syncee *syncee, Syncee *target, bool override=false);

  private:
    QPtrList<Syncee> mSyncees;
    SyncUi *mUi;
    SyncAlgorithm* mInterface;
    class SyncerPrivate;
    SyncerPrivate* d;
};

}

#endif

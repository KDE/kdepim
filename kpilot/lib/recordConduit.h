#ifndef _RECORD_CONDUIT_H
#define _RECORD_CONDUIT_H
/* record-conduit.h                           KPilot
**
** Copyright (C) 2004 by Reinhold Kainhofer
** Based on the addressbook conduit:
** Copyright (C) 2000,2001 by Dan Pilone
** Copyright (C) 2000 Gregory Stern
** Copyright (C) 2002-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include <plugin.h>
#include "pilotAppCategory.h"


typedef QValueList<recordid_t> RecordIDList;

/** This is a base class for record-based conduits. The data on the PC is represented 
 *  by an object derived from the RecordConduit::PCData class, and each entry on the PC by
 *  an object derived from RecordConduit::PCEntry. Only very few methods of this class 
 *  need to be implemented, mostly just the functions to copy a PC entry to a handheld 
 *  entry and vice versa. All the sync logic is already present in this class and does
 *  not need to be implemented by child classes. */
class RecordConduit : public ConduitAction
{
Q_OBJECT
public:
	RecordConduit( QString name, KPilotDeviceLink *o,const char *n = 0L,
		const QStringList &a = QStringList() );
	virtual ~RecordConduit();

	/** This class is just a wrapper around an entry on the PC, which provides
	 *  a uniform interface for all different kinds of entries like addressbook,
	 *  calendar, todo, diary entries, etc.
	 */
	class PCEntry
	{
	public:
		PCEntry() {}
		virtual ~PCEntry() {}
		/** Return the uid of the entry on the PC */
		virtual QString uid() const = 0;
		/** Return the palm record id uid of the entry (this record id should be 
		 *  stored somewhere together with the entry on the pc to make it possible to
		 *  link an entry on the handheld and one on the PC together. */
		virtual recordid_t recid() const = 0;
		/** Store a palm record id with the entry on the PC */
		virtual void setRecid( recordid_t recid ) = 0;
		/** Check if the PC entry is actually empty. */
		virtual bool isEmpty() const = 0;
		/** Return if the entry is maked as archived and should not be copied to the handheld. */
		virtual bool isArchived() const = 0;
		/** Mark the entry as archived so it isn't copied to the handheld */
		virtual bool makeArchived() = 0;
		/** Add a category to the entry */
		virtual bool insertCategory( QString cat ) = 0;
	};
	/** This class is just a wrapper around the data on the PC, which provides
	 *  a uniform interface for all different kinds of data like addressbook,
	 *  calendar, todo list etc. Most method are pure-virtual, so they must
	 *  be implemented by each conduit.
	 */
	class PCData
	{
	public:
		class Iterator 
		{
		public:
			Iterator() {};
			virtual ~Iterator() {};
			/** Reimplement this to return the PCEntry at the current position. 
			Use new for this. Caller is responsible for deleting the object. */
			virtual PCEntry * operator*() { return 0; }
			virtual void operator++() {}
			virtual const Iterator* self() const { return this; }
		};
	public:
		PCData( RecordConduit *conduit ):mConduit( conduit ), mChanged( false ) {}
		virtual ~PCData() {}

		/*************************************************************
		 * pure virtuals, must be implemented by childclasses
		 *************************************************************/
		 
		/** Load the data from the PC (e.g. contacts from the addressbook).
		 *  @return true if successful, false if not */
		virtual bool loadData() = 0;
		/** Save the PC data (e.g. contacts to the addressbook).
		 *  @return true if successful, false if not */
		virtual bool saveData() = 0;
		/** Description of the data on the PC, e.g. the Addressbook, Calendar, etc.
		*/
		virtual QString description() const = 0;
		/** Return true if the data on the pc (e.g. addressbook, calendar etc.) is empty
		*/
		virtual bool isEmpty() const = 0;
		/** reset the data pointer to the beginning of the data, e.g. reset an 
		 *  iterator to begin()
		*/
		virtual Iterator begin() = 0;
		/** Return true if the pc data is at the end of its list.
		*/
		virtual bool atEnd( const Iterator & ) = 0;
		/** Return next entry in the data. 
		*/
		virtual bool increase( Iterator &it ) { ++it; return true; }
		/** Return next modified entry in the data. 
		*/
		virtual bool increaseNextModified( Iterator &it ) { return increase( it ); }
		/** Find the Palm Entry in the pc data 
		 *  @return the match of the record found in the data on the PC, 
		 *  or an initialized empty entry if no match is found on the PC */
		virtual PCEntry *findByUid( QString uid ) const = 0;
		virtual bool makeArchived( PCEntry *addr );
		virtual const QStringList uids() const = 0;
		/** Update the entry given. If it doesn't exist yet, add it
		 */
		virtual bool updateEntry( const PCEntry* entry ) = 0;
		/** Remove the entry given from the PC. 
		 */
		virtual bool removeEntry( const PCEntry* entry ) = 0;
		/** Remove the entry with the given id. The default implementation 
		 *  searches the entry, and then calls removeEntry( PCEntry* ). Child 
		 *  classes might reimplement this to speed things up.
		 */
		virtual bool removeEntry( const QString &uid ) { return removeEntry( findByUid( uid ) ); }

		/*************************************************************
		 * non-pure virtuals, might be reimplemented by childclasses
		 *************************************************************/
		 
		/** Clean up after the sync has been done and the changes were saved
		 *  @return true if cleanup was successful */
		virtual bool cleanup() { return true; }
		/** Builds the map which links record ids to uid's of PCEntry. This is the slow implementation,
		 *  that should always work. subclasses should reimplement it to speed things up.
		 */
		virtual bool mapContactsToPilot( QMap<recordid_t,QString> &idContactMap);

		/** Return whether the data was changed
		*/
		virtual bool changed() const { return mChanged; }
		/** Set the changed flag on the pc data. 
		 *  @return the previous state of the changed flag */
		virtual bool setChanged( bool changed ) { bool old = mChanged; mChanged = changed; return old; }
		
		/** Return the number of entries on the PC (-1 for unknown)
		 */
		virtual int count() const { return -1; }
		
	protected:
		RecordConduit *mConduit;
		bool mChanged;
	}; 
	

/*********************************************************************
                S Y N C   S T R U C T U R E
 *********************************************************************/
	virtual bool exec();
protected slots:
	void slotPalmRecToPC();
	void slotPCRecToPalm();
	void slotDeletedRecord();
	void slotDeleteUnsyncedPCRecords();
	void slotDeleteUnsyncedHHRecords();
	void slotCleanup();
        
protected: 
	static void setArchiveDeleted( bool archiveDeleted ) { mArchiveDeleted = archiveDeleted; }
	static bool archiveDeleted() { return mArchiveDeleted; }

	/************* INTERFACE for child classes ******************/
		
		/*************************************************************
		 * pure virtuals, must be implemented by childclasses
		 *************************************************************/
	
	/** Initialize the PC Data object, must be a child class of RecordConduit::PCData
	*/
	virtual PCData*initializePCData() = 0;
	/** return the name of the databases synced by this conduit 
	*/
	virtual QString dbName() const = 0;
	/** Create a Palm entry from the raw palm record 
	*/
	virtual PilotAppCategory *createPalmEntry( PilotRecord *rec ) = 0;
	/** Read the config. You need to call at least setArchiveDeleted(bool) 
	 *  and setConflictResolution(SyncAction::ConflictResolution)
	 */
	virtual void readConfig() = 0;
	
	
	/** Create a buffer for the appInfo and pack it inside. Also, remember to
	 *  set appLen to the size of the buffer! By default, nothing is done, and 
	 *  the return value will be 0 (appinfo not supported by default, you need
	 *  to implement this in your child class!).
	 *  @param appLen Receives the length of the allocated buffer */
	virtual unsigned char *doPackAppInfo( int */*appLen*/ ) { return 0; }
	/** Read the appInfo from the buffer provided to this method. By default, 
	 *  nothing is done, and the return value will be false (appinfo not supported 
	 *  by default, you need to implement this in your child class!).
	 *  @param buffer Buffer containing the appInfo in packed format
	 *  @param appLen specifies the length of buffer */
	virtual bool doUnpackAppInfo( unsigned char */*buffer*/, int /*appLen*/ ) { return false; }
	/** Return the n-th category on the handheld. By default a null string will
	 *  be returned. If you want your conduit to support categories, reimplement this
	 *  method in your childclass!
	 *  @param n # of the desired category name */
	virtual QString category( int /*n*/ ) const { return QString::null; }
	
		/* ******************************************************************* *
                   C O P Y   R E C O R D S
		 * ******************************************************************* */
 	/** Flags for the _equal() method, that specify which parts of the 
	 *  record need to be checked. Flags and category are pre-defined, 
	 *  but each conduit can define its own flags and reimplement findFlags().
	 */
	enum eqFlags {
		eqFlagsFlags=0x01,
		eqFlagsCategory=0x02,
		eqFlagsAll=0xFFFF,
		eqFlagsAlmostAll=eqFlagsAll && ~(eqFlagsFlags|eqFlagsCategory)
	};
	/** Check if the entry from the Handheld matches the entry from the PC (but 
	 *  check only those fields of the records that are specified by the flag 
	 *  argument. By default, all fields should be checked, but in some cases 
	 *  (like finding an approximately equal entry, reimplement findFlags() to 
	 *  use different flags in that case) only some parts should be checked.
	 *  @param palmEntry The entry from the handheld
	 *  @param pcEntry The entry on the PC side
	 *  @param flag Flags that specify which fields need to be checked.
	 *  @return Returns wheter the two entries match in all fields specified by flag.
	 */
	virtual bool _equal( const PilotAppCategory *palmEntry, const PCEntry *pcEntry, int flag = eqFlagsAll ) const = 0;
	/** Copy all fields from the PC entry to the handheld entry.
	 *  @param toPalmEntry Entry on the handheld that will receive the values.
	 *  @param fromPCEntry Entry on the PC that should be copied to the handheld entry.
	 *  @return wheter copying was successful (ie. both pointers are valid, and 
	 *  copying went fine.
	 */
	virtual bool _copy( PilotAppCategory *toPalmEntry, const PCEntry *fromPCEntry ) = 0;
	/** Copy all fields from the handheld entry to the PC entry.
	 *  @param toPalmEntry Entry on the handheld that should be copied to the PC entry. 
	 *  @param fromPCEntry Entry on the PC that will receive the values from the handheld entry.
	 *  @return wheter copying was successful (ie. both pointers are valid, and 
	 *  copying went fine.
	 */
	virtual bool _copy( PCEntry *toPCEntry, const PilotAppCategory *fromPalmEntry ) = 0;

		/* ******************************************************************* *
		     C O N F L I C T   R E S O L U T I O N   a n d   M E R G I N G
		 * ******************************************************************* */
 	/** The entry on the pc and on the handheld were both changed, and an automatic
	 *  conflict resolution is not possible, so do a suitable resolution (ask the 
	 *  user or apply some pre-defined method (e.g. one side always overrides etc).
	 */
	virtual bool smartMergeEntry( PCEntry *pcEntry, PilotAppCategory *backupEntry, 
		PilotAppCategory *palmEntry ) = 0;
	

		
		/*************************************************************
		 * non-pure virtuals, might be reimplemented by childclasses if
		 * the defaults are not suitable and better/faster ways are available
		 * or the conduit needs to do some special actions.
		 *************************************************************/
	
	/** Do the preperations before the sync algorithm starts. */
	virtual bool doPrepare() { return true; }
	/** Return the list of category names on the handheld. The default 
	 *  implementation is slow, because it just calls category(n) for 0<n<16.
	 *  This method should be reimplemented by child classes when a faster 
	 *  algorithm is available. For conduits that don't support categories at
	 *  all, this might also reimplemented to automatically return an empty list.
	 */
	virtual const QStringList categories() const;
   /** Specifies which parts of the record shall be checked when searching
	 *  for matches of the handheld data in the PC data. By default, everything
	 *  except flags and categories are checked. Child classes that desire 
	 *  other comparisons (i.e. compare just an enty date if only one entry per
	 *  date is allowed, like for diaries, etc) should reimplement this method.
	 */
	virtual int findFlags() const;
	/** Do some cleanup after the sync, e.g. write sync time to the config, 
	 *  write some version to the config, whatever you like in your conduit.
	 */
	virtual void doPostSync() {};

	
	
	
// ----------------------------------------------------------------------- //




	/* ****************************************************** */
	/*  Handle the configuration                              */
	/* ****************************************************** */

	/* Read the global KPilot config file for settings
	 * particular to the RecordConduit conduit. */
	static bool isDeleted( const PilotAppCategory *addr );
	static bool isArchived( const PilotAppCategory *addr );
	static bool isArchived( const PCEntry *addr ) { return addr->isArchived(); }


	/* ****************************************************** */
	/*  Handle special fields of the Es                       */
	/* ****************************************************** */
	QString getCatForHH( const QStringList cats, const QString curr ) const;
	void setCategory( PCEntry *pcEntry, QString cat );



	/* ****************************************************** */
	/*  Loading and saving the addressbook and database       */
	/* ****************************************************** */


protected:
	/* Do the preperations before doSync or doBackup.
	 * Load contacts, set the pilot */
	bool _prepare();
	void _getAppInfo();
	void _setAppInfo();
	static int compareStr ( const QString & str1, const QString & str2 );




/* ******************************************************************* *
              G E N E R A L   S Y N C   F U N C T I O N
         These functions modify the Handheld and the addressbook
 * ******************************************************************* */
protected:
	bool syncEntry( PCEntry *pcEntry, PilotAppCategory *backupEntry, 
		PilotAppCategory *palmEntry );
	virtual bool pcCopyToPalm( PCEntry *pcEntry, PilotAppCategory *backupEntry, 
		PilotAppCategory *palmEntry );
	virtual bool palmCopyToPC( PCEntry *pcEntry, PilotAppCategory *backupEntry, 
		PilotAppCategory *palmEntry );

/* ******************************************************************* *
                 l o w - l e v e l   f u n c t i o n s   f o r
                   adding / removing palm/pc records
 * ******************************************************************* */
	bool palmSaveEntry( PilotAppCategory *palmEntry, PCEntry *pcEntry );
	bool backupSaveEntry( PilotAppCategory *backup );
	bool pcSaveEntry( PCEntry *pcEntry, PilotAppCategory *backupEntry, 
		PilotAppCategory *palmEntry );
	bool pcDeleteEntry( PCEntry *pcEntry, PilotAppCategory *backupEntry, 
		PilotAppCategory *palmEntry );


/*********************************************************************
 C O N F L I C T   R E S O L U T I O N   a n d   M E R G I N G
 *********************************************************************/
	PCEntry *findMatch( PilotAppCategory *palmEntry ) const;


/********************************************************/
/*   D A T A   M E M B E R S ,   S E T T I N G S        */
/********************************************************/

protected:
	PCData* mPCData;
	PCData::Iterator mPCIter;

	int mPalmIndex;
	/** mEntryMap maps record ids to IDs of Es. This is used to speed up searching the local addressbook */
	QMap < recordid_t, QString > mEntryMap;
	RecordIDList mSyncedIds, mAllIds;
	static bool mArchiveDeleted;
} ;

#endif

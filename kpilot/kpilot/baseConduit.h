/* baseConduit.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the base class for all external conduits.
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
#ifndef _KPILOT_BASECONDUIT_H
#define _KPILOT_BASECONDUIT_H

#ifndef QOBJECT_H
#include <qobject.h>
#endif

#ifndef QSTRING_H
#include <qstring.h>
#endif

#ifndef QPIXMAP_H
#include <qpixmap.h>
#endif

#ifndef _KPILOT_PILOTRECORD_H
#include "pilotRecord.h"
#endif


class QWidget;
class PilotDatabase;
/** @brief Base class for all conduits.
 *
 *  A conduit is a seperate application that handles syncing with a
 *  palm pilot database.

 *  Any seperate application in the palm pilot is a database.
 *  So the calender is a database, the address book is a database,
 *  the memos, etc.  Each database is composed of records.
 *  Each record needs to be unpacked to extract it's information.
 *
 *  The unix library pilot-link provides the ability to unpack the records
 *  into structures.  The KPilot library contains some classes that
 *  wrap the structures into classes, such as PilotAddress.  It is
 *  recommended for any new conduit, that the unpacked record gets wrapped
 *  into a class, such as PilotAddress.
 *
 *  GSQA:
 *
 *  Need docs on conduit interaction with KConfig.  How does each conduit
 *  make it's own prefs?
 *
 *  How is a conduit registered.  What class handles the conduit regestration.
 *
 *  GSQA: What is the relationship between KPilotLink and BaseConduit.
 *        Right now getConfig() exists in KPilotLink as a static.  Can
 *        / should we move it to BaseConduit?  Why does BaseConduit inherit from QObject
 */
class BaseConduit : public QObject
{
  Q_OBJECT

public:
  enum eConduitMode { 
		Error=-1, 
		None=0, 
		HotSync, 
		Setup,
		Backup, 
		Test,
		DBInfo
	};

  enum DatabaseSource {
                Undefined=-1,
                ConduitSocket=0,
		Local
  };
      
	enum ConduitExitCode {
		Normal=0,
		ConduitMisconfigured=1,		// f.ex missing file
		DCOPError=2,			// generic DCOP error
		PeerApplicationMissing=3,
		InvalidLocalDBPath=4
		} ;
  /**
   * The mode that this conduit should be running in will be passed to the
   * constructor.   After the constructor returns the appropriate 
   * virtual method should be called (ie: setup, hotsync, backup, etc).
   *
   * The default database source will be assigned to Socket.
   */
  BaseConduit(eConduitMode);
  /**
   * Same as above constructor, but can specify the source
   */
  BaseConduit(eConduitMode mode, DatabaseSource source);
  virtual ~BaseConduit();

  /**
   *  Actually opens the appropriate database depending on the
   *  the setting for DatabaseSource (Local creates a  pilotLocalDatabase
   *  or ConduitSocket creates a pilotConduitDatabase).  This cannot
   *  be done in the constructor since the virtual function dbInfo()
   *  is needed for pilotLocalDatabase.
   *
   *  This method should be called inside the the ConduitApp function
   *  exec().
   */
  void init();
  
  /**
   *  This will be called to do the actual hotsync.  Users should override
   *  this to do a normal hotsync with the pilot.
   *
   *  In other words, check to see what's been modified on the pilot,
   *  copy it to the program, then check to see what's been modified in the
   *  program and copy it to the pilot.
   *
   * @see readNextModifiedRecord
   */
  virtual void doSync() = 0;
  
  /**
   * Called when use asks to do a full backup of pilot.  Should be
   * used to verify that whatever the target application is has all
   * the records currently on the pilot.  (Use readRecordByIndex())
   * Certain conduits will not need to use this.  (Such as popmail, etc)
   *
   * @see readRecordByIndex
   */
  virtual void doBackup() { }
  
  /**
   * Return an about dialog and any setup options.  
   * NOTE: The widget returned MUST delete itself when closed or else
   * the conduit will hang waiting for it!!
   */
  virtual QWidget* aboutAndSetup() = 0;

  /**
   * Return the text string Identifier of the DB you work with,
   * ie: MemoDB, DatebookDB, etc.
   */
  virtual const char* dbInfo() { return "<none>"; }

  /**
   * Run a test on this conduit, with full debugging turned on.
   * This may be reimplemented in some conduits to do actual
   * tests.
   */
  virtual void doTest() { } ;
  
  /**
   * Returns an icon for the window manager
   * when the conduit is in "setup" mode.
   */
  virtual QPixmap icon() const;
  
  const eConduitMode getMode() const { return fMode; } 

protected:

  /**
   * Returns 0L if no more modified records.  User must delete
   * the returned record when finished with it.
   */
  PilotRecord* readNextModifiedRecord();
 
  /**
   * Returns 0L if no more records in category.  User must delete
   * the returned record when finished with it.
   */
  PilotRecord* readNextRecordInCategory(int category);

  /**
   * Returns 0L if index is invalid.  User must delete the
   * returned record when finished with it.
   */
  PilotRecord* readRecordByIndex(int index);

  /**
   * Returns 0L if ID is invalid.  User must delete the
   * returned record when finished with it.
   */
  PilotRecord* readRecordById(recordid_t id);

  /** Call this function to read the application specific information,
   *  i.e. AddressAppInfo.  There should be a pilot library function
   *  to unpack from the buffer into the application struct
   */
  int readAppInfo(unsigned char *buffer);

  /**
   * Writes a record to the current database.  If rec->getID() == 0,
   * a new ID will be assigned and returned.  Else, rec->getID() is
   * returned.
   */
  recordid_t writeRecord(PilotRecord* rec); 
  
	/**
	* Adds a message to the sync log by talking to
	* the daemon; the message s should not contain
	* any `weird' characters, no \r or \n, and must be
	* null-terminated. In addition, s must be no longer
	* than 30 characters.
	* Returns 1 on success, 0 on failure.
	*/
  bool addSyncLogMessage(const char *s);

	/**
	* Conduits can have an additional Debug= line in their
	* config which may be read and ORed with the user-specified
	* debug level. This function does that. It's not automatic
	* because then BaseConduit would have to know the group
	* the conduit wants to read.
	*
	* @ret resulting debug level
	*/
	int getDebugLevel(KConfig&) ;

	/**
	* Conduits can have a "first time" setting that causes
	* additional actions to be taken. There is also a global
	* KPilot setting "force first time" that interferes here.
	*
	* Remember that conduits should set first time to false
	* in their code.
	*
	* @ret true for first time, false otherwise
	*/
	bool getFirstTime(KConfig&);

	/**
	* For consistent setting of "first time" we introduce this
	* convenience function.
	*/
	void setFirstTime(KConfig&,bool=false);

private:
  /**
   * Mode for this instance of the conduit
   */
  eConduitMode fMode;
  PilotDatabase *fDB;
  DatabaseSource fDBSource;

};


#else
#ifdef DEBUG
#warning "File doubly included"
#endif
#endif


// $Log$
// Revision 1.22  2001/03/30 17:11:31  stern
// Took out LocalDB for mode and added DatabaseSource enum in BaseConduit.  This the user can set the source for backup and sync
//
// Revision 1.21  2001/03/29 21:41:49  stern
// Added local database support in the command line for conduits
//
// Revision 1.20  2001/03/27 23:54:43  stern
// Broke baseConduit functionality out into PilotConduitDatabase and added support for local mode in BaseConduit
//
// Revision 1.19  2001/03/19 23:12:39  stern
// Made changes necessary for upcoming abbrowser conduit.
//
// Mainly, I added two public methods to PilotAddress that allow for easier
// setting and getting of phone fields.
//
// I also have added some documentation throughout as I have tried to figure
// out how everything works.
//
// Revision 1.18  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.17  2001/03/04 21:59:30  adridg
// Possible missed #include leading to incomplete types
//
// Revision 1.16  2001/03/02 16:59:35  adridg
// Added new protocol message READ_APP_INFO for conduit->daemon communication
//
// Revision 1.15  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//

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
#ifndef __BASE_CONDUIT_H
#define __BASE_CONDUIT_H

#include <qobject.h>
#include <qstring.h>
#include "pilotRecord.h"

class KSocket;

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

	enum ConduitExitCode {
		Normal=0,
		ConduitMisconfigured=1,		// f.ex missing file
		DCOPError=2,			// generic DCOP error
		PeerApplicationMissing=3
		} ;
  /**
   * The mode that this conduit should be running in will be passed to the
   * constructor.   After the constructor returns the appropriate 
   * virtual method should be called (ie: setup, hotsync, backup, etc).
   */
  BaseConduit(eConduitMode);
  virtual ~BaseConduit();

  /**
   *  This will be called to do the actual hotsync.  Users should override this to 
   * do a normal hotsync with the pilot.  In other words, check to see what's been
   * modified on the pilot, copy it to the program, then check to see what's been modified
   * in the program and copy it to the pilot.
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

	const eConduitMode getMode() const { return fMode; } ;

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
	int addSyncLogMessage(const char *s);

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

  KSocket* fDaemonSocket;
  PilotRecord* getRecord(KSocket*);
  void writeRecord(KSocket* theSocket, PilotRecord* rec);
};


#endif


// $Log:$

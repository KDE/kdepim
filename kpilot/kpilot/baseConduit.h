#ifndef __BASE_CONDUIT_H
#define __BASE_CONDUIT_H

#include <fstream.h>
#include <qstring.h>
#include "pilotRecord.h"

class PilotRecord;
class KSocket;

class BaseConduit : public QObject
{
  Q_OBJECT

public:
  enum eConduitMode { Error, HotSync, Setup, Backup, DBInfo };

  /**
   * The mode that this conduit should be running in will be passed to the
   * constructor.   After the constructor returns the appropriate virtual method will
   * be called (ie: setup, hotsync, backup, etc).
   */
  BaseConduit(eConduitMode mode);
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
   * Mode for this instance of the conduit
   */
  eConduitMode fMode;

private:
  KSocket* fDaemonSocket;
  PilotRecord* getRecord(KSocket*);
  void writeRecord(KSocket* theSocket, PilotRecord* rec);
};


#endif

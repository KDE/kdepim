// Conduit for KPilot <--> KOrganizer 
// (c) 1998 Dan Pilone, Preston Brown, Herwin Jan Steehouwer

#ifndef _VCALCONDUIT_H
#define _VCALCONDUIT_H

#include "baseConduit.h"
#include "vcc.h"
#include "pi-datebook.h"

class PilotRecord;

class VCalConduit : public BaseConduit
{
public:
  VCalConduit(eConduitMode mode);
  virtual ~VCalConduit();
  
  virtual void doSync();
  virtual void doBackup();
  virtual QWidget* aboutAndSetup();

  virtual const char* dbInfo() { return "DatebookDB"; }
  
	/**
	* Returns a string - internationalized already -
	* describing the conduit. This is used in window
	* captions and other version identifiers.
	*/
	static const char *version();

protected:
  void doLocalSync();
  PilotRecord *findEntryInDB(unsigned int id);
  VObject *findEntryInCalendar(unsigned int id);
  void deleteVObject(PilotRecord *rec);
  void updateVObject(PilotRecord *rec);
  void saveVCal();
  QString TmToISO(struct tm tm);
  struct tm ISOToTm(const QString &tStr);
  int numFromDay(const QString &day);
  int timeZone;
  VObject *fCalendar;

};

#endif

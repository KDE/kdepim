// Conduit for KPilot <--> KOrganizer 
// (c) 1998 Dan Pilone, Preston Brown, Herwin Jan Steehouwer

#ifndef _VCALCONDUIT_H
#define _VCALCONDUIT_H

#include "baseConduit.h"
#include "vcc.h"
#include "pi-datebook.h"

class PilotRecord;
class PilotDateEntry;

class VCalConduit : public BaseConduit
{
public:
  VCalConduit(BaseConduit::eConduitMode mode);
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

private:
	void getCalendar();

	/**
	* Set the event to repeat forever, with repeat
	* frequency @arg rFreq. This function also
	* warns the user that this is probably not
	* *quite* the behavior intented but there's
	* no fix for that.
	*/
	void repeatForever(PilotDateEntry *p,int rFreq,VObject *v=0L);

	QString calName;
};

#endif

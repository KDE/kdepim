// Conduit for KPilot <--> KOrganizer 
// (c) 1998 Dan Pilone, Preston Brown, Herwin Jan Steehouwer

#ifndef _TODOCONDUIT_H
#define _TODOCONDUIT_H

#include "baseConduit.h"
#include "vcc.h"
#include "pi-datebook.h"

class PilotRecord;

class TodoConduit : public BaseConduit
{
public:
  TodoConduit(eConduitMode mode);
  virtual ~TodoConduit();
  
  virtual void doSync();
  virtual void doBackup();
  virtual QWidget* aboutAndSetup();

  virtual const char* dbInfo() { return "TodoDB"; }
  
	static const char *version();


protected:
  void doLocalSync();
  PilotRecord *findEntryInDB(unsigned int id);
  VObject *findEntryInCalendar(unsigned int id);
  void deleteVObject(PilotRecord *rec);
  void updateVObject(PilotRecord *rec);
  void saveTodo();
  QString TmToISO(struct tm tm);
  struct tm ISOToTm(const QString &tStr);
  int numFromDay(const QString &day);
  int timeZone;
  VObject *fCalendar;

};

#endif

// Conduit for KPilot <--> POP3
// (c) 1998 Dan Pilone

#ifndef _NULL_CONDUIT_H
#define _NULL_CONDUIT_H

#include "baseConduit.h"

class PilotRecord;

class NullConduit : public BaseConduit
{
public:
  NullConduit(eConduitMode mode);
  virtual ~NullConduit();
  
  virtual void doSync();
  virtual QWidget* aboutAndSetup();

  virtual const char* dbInfo() { return NULL; }
  
};

#endif

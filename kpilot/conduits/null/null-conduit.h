// null-conduit.h
//
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$


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

  virtual const char* dbInfo() ; // { return NULL; }
};

// $Log:$
#endif

// conduitApp.hc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$


#ifndef __CONDUIT_APP_H
#define __CONDUIT_APP_H

#include <kapp.h>
#include "baseConduit.h"

// Debug level is set -- among other things --
// by the constructor for conduit apps.
//
//
extern int debug_level;

class ConduitApp : protected KApplication
{
  Q_OBJECT

public:
  ConduitApp(int& argc, char** argv);
  ConduitApp(int& argc, char** argv, const QString& rAppName);

  void setConduit(BaseConduit* conduit);
  void quit() { KApplication::quit(); }
  int exec();

  BaseConduit::eConduitMode getMode() { return fMode; }

protected:
	BaseConduit::eConduitMode handleOptions(int&,char**);
	void usage();

private:
  BaseConduit* fConduit;
  BaseConduit::eConduitMode fMode;
};

#endif

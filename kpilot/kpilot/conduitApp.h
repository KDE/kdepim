/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __CONDUIT_APP_H
#define __CONDUIT_APP_H

#include <kapp.h>
#include "baseConduit.h"

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

private:
  BaseConduit* fConduit;
  BaseConduit::eConduitMode fMode;
};

#endif

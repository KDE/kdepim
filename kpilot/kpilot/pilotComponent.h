/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __PILOT_COMPONENT_H
#define __PILOT_COMPONENT_H

/**
  * Base class for any module to KPilot
  */
#include <qwidget.h>

class KPilotLink;

class PilotComponent : public QWidget
    {
    Q_OBJECT

    public:
    PilotComponent(QWidget* parent);
      
      virtual void initialize() = 0; // Load data from files, etc.
      //    virtual bool doHotSync(KPilotLink*) = 0; // True if successful
      virtual void preHotSync(char* ) { } // Prepare for hotsync
      // and append any commands to send to the pilot onto the char* array
      virtual void postHotSync() { } // Reload data, etc, etc
    virtual bool saveData() = 0; // true if everything is saved
//     virtual bool hotSyncNeeded() = 0; // True if component wants to hot-sync
//     virtual void enableHotSync(bool) = 0; // If false, hotSyncNeeded() will always be false.
    };

#endif

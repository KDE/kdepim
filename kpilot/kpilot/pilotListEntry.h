/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __PILOT_LIST_ENTRY_H
#define __PILOT_LIST_ENTRY_H

#include <qlistbox.h>

class PilotListEntry : public QListBoxText
{

public:
  PilotListEntry(const char* s, int id)
    : QListBoxText(s), fId(id) { }

  int getId() const { return fId; }

private:
  int fId;
};
  
#endif

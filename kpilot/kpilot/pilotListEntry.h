/* pilotListEntry.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
**
** I have the sneaking suspicion that this class is obsolete
** and / or should be re-specialized to use more modern classes.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
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


// $Log:$

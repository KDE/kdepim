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
#ifndef _KPILOT_PILOTLISTENTRY_H
#define _KPILOT_PILOTLISTENTRY_H

#ifndef QLISTBOX_H
#include <qlistbox.h>
#endif

class PilotListEntry : public QListBoxText
{

public:
  PilotListEntry(const char* s, int id)
    : QListBoxText(s), fId(id) { }

  int getId() const { return fId; }

private:
  int fId;
};
  
#else
#warning "File doubly included"
#endif


// $Log$
// Revision 1.4  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//

/* pilotListMakerEntry.h	-*- C++ -*-		KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2002 by Reinhold Kainhofer
**
** See the .cc file for an explanation of what this file is for.
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
** Bug reports and questions can be sent to groot@kde.org
*/
#ifndef _KPILOT_PILOTListMakerENTRY_H
#define _KPILOT_PILOTListMakerENTRY_H

#include <time.h>
#include <string.h>
#include <qbitarray.h>

#include "options.h"
#include "pilotOrganizerEntry.h"


class PilotListMakerEntry : public PilotOrganizerEntry {
public:
	PilotListMakerEntry(void):PilotOrganizerEntry(){};
	PilotListMakerEntry(KCal::Todo*todo):PilotOrganizerEntry(todo) {};
//  PilotListMakerEntry(const PilotListMakerEntry &e);
	virtual PilotOrganizerEntry*createOrganizerEntry(KCal::Todo *todo=NULL){};
	PilotListMakerEntry(PilotRecord* rec);
	~PilotListMakerEntry() { }

//  PilotListMakerEntry& operator=(const PilotListMakerEntry &e);
  
protected:
	virtual void *pack(void *, int *);
	virtual void unpack(const void *, int = 0);
};



#else
#ifdef DEBUG
#warning "File doubly included"
#endif
#endif


// $Log$
// Revision 1.7  2002/04/07 11:56:19  reinhold
// Last version before moving to KDE CVS
//
// Revision 1.6  2002/04/06 00:51:08  reinhold
// Finally the conduit compiles again... Still have a lot of TODOS
//
// Revision 1.5  2002/04/05 21:17:01  reinhold
// *** empty log message ***
//
// Revision 1.4  2002/03/23 21:46:43  reinhold
// config  dlg works, but the last changes crash the plugin itself
//
// Revision 1.3  2002/03/10 23:58:33  reinhold
// Made the conduit compile...
//
// Revision 1.2  2002/03/10 16:06:43  reinhold
// Cleaned up the class hierarchy, implemented some more features (should be quite finished now...)
//
// Revision 1.1  2002/03/09 15:48:32  reinhold
// Added the classes for the different palm database formats
//

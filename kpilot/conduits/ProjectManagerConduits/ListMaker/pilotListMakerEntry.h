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



#endif


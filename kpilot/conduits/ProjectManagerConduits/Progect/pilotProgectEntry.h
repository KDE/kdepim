/* pilotProgectEntry.h	-*- C++ -*-		KPilot
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
#ifndef _KPILOT_PILOTProgectENTRY_H
#define _KPILOT_PILOTProgectENTRY_H

#include <time.h>
#include <string.h>

#ifndef QBITARRAY_H
#include <qbitarray.h>
#endif

#include "options.h"
#ifndef _KPILOT_PILOTOrganizerENTRY_H
#include "pilotOrganizerEntry.h"
#endif

typedef enum ProgectTypes {
	PERCENTAGE,
	TODO,
	NUMERIC,
	INFORMATIVE
}


class PilotProgectEntry : public PilotOrganizerEntry {
public:
	PilotProgectEntry(void):PilotOrganizerEntry(){};
	PilotProgectEntry(PilotRecord* rec);
	~PilotProgectEntry() { }
	PilotProgectEntry& operator=(const PilotProgectEntry &e);
  
protected:
	virtual void *pack(void *, int *);
	virtual void unpack(const void *, int = 0);
	int maxVal, numVal;
};



#endif


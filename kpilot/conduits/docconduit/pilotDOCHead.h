/* pilotDOCHead.h	-*- C++ -*-		KPilot
**
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to the mailinlist kde-pim@kde.org
*/
#ifndef _KPILOT_PILOTDOCHEAD_H
#define _KPILOT_PILOTDOCHEAD_H

#include <pilotAppCategory.h>

class PilotRecord;


class PilotDOCHead:public PilotAppCategory {
 private:
	static const int textRecordSize;

 public:
	int version;
	int spare;
	long int storyLen;
	int numRecords;
	int recordSize;
	long int position;

 public:
	 PilotDOCHead();
	 PilotDOCHead(PilotRecord * rec);
	 PilotDOCHead(const PilotDOCHead & e);
	~PilotDOCHead() {
	} PilotDOCHead & operator=(const PilotDOCHead & e);


 protected:
	void *pack_(void *, int *);
	void unpack(const void *, int = 0);
};



#endif


#ifndef _KPILOT_PILOTRECORD_H
#define _KPILOT_PILOTRECORD_H
/* pilotRecord.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <time.h>

#include <pi-file.h>

class PilotRecord 
{
public:
	PilotRecord(void* data, int len, int attrib, int cat, pi_uid_t uid);
	PilotRecord(PilotRecord* orig);
	~PilotRecord() { delete [] fData; fDeleted++; }

	PilotRecord& operator=(PilotRecord& orig);

	char* getData() const { return fData; }
	int   getLen() const { return fLen; }
	void setData(const char* data, int len);
	inline int   getAttrib() const { return fAttrib; }
	inline void  setAttrib(int attrib) { fAttrib = attrib; }

	int   getCat() const { return fCat; }
	void  setCat(int cat) { fCat = cat; }

	unsigned long getID() const { return fID; }
	void setID(unsigned long id) { fID = id; }

private:
	char* fData;
	int   fLen;
	int   fAttrib;
	int   fCat;
	unsigned long fID;

public:
	inline bool isDeleted() const { return fAttrib & dlpRecAttrDeleted; };
	inline bool isSecret() const { return fAttrib & dlpRecAttrSecret; } ;
	inline bool isArchived() const { return fAttrib & dlpRecAttrArchived; } ;
	inline void makeDeleted() { fAttrib |= dlpRecAttrDeleted; } ;
	inline void makeSecret() { fAttrib |= dlpRecAttrSecret; } ;

	/**
	* This is an interface for tracking down memory leaks
	* in the use of PilotRecords (for those without valgrind).
	* Count the number of allocations and deallocations.
	*/
public:
	static void allocationInfo();
private:
	static int fAllocated,fDeleted;
};

#endif

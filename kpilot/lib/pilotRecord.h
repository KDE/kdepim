#ifndef _KPILOT_PILOTRECORD_H
#define _KPILOT_PILOTRECORD_H
/* pilotRecord.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines the class PilotRecord, which is the lowest-
** denominator representation of the bits used in a Pilot-based
** database record. This can be converted into a PilotAppCategory
** (subclass) object, which is the interpreted form of the bits.
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
#include <unistd.h>
#include <stdio.h>

#include <pi-file.h>

class PilotRecord
{
public:
	// This constructor makes a copy of the data buffer
	PilotRecord(void* data, int len, int attrib, int cat, pi_uid_t uid);
#ifdef HANDOFF_BUFFERS
	// This constructor assumes ownership of the data buffer
	PilotRecord(int len, void *data, int attrib, int cat, pi_uid_t uid) :
		fData(data),fLen(len),fAttrib(attrib),
		fCat(cat),fID(uid) { fAllocated++; } ;
#endif
	PilotRecord(PilotRecord* orig);
	~PilotRecord() { delete [] fData; fDeleted++; }

	enum { APP_BUFFER_SIZE = 0xffff } ;

	PilotRecord& operator=(PilotRecord& orig);

	char* getData() const { return fData; }
	int   getLen() const { return fLen; }
	void setData(const char* data, int len);
	inline int   getAttrib() const { return fAttrib; }
	inline void  setAttrib(int attrib) { fAttrib = attrib; }

	int   getCat() const { return fCat; }
	void  setCat(int cat) { fCat = cat; }

	inline recordid_t id() const { return fID; }
	recordid_t getID() const KDE_DEPRECATED { return id(); }
	void setID(recordid_t id) { fID = id; }

private:
	char* fData;
	int   fLen;
	int   fAttrib;
	int   fCat;
	recordid_t fID;

public:
	inline bool isDeleted() const { return fAttrib & dlpRecAttrDeleted; };
	inline bool isSecret() const { return fAttrib & dlpRecAttrSecret; } ;
	inline bool isArchived() const { return fAttrib & dlpRecAttrArchived; } ;
	inline void setDeleted(bool d=true) {
		if (d) { fAttrib |= dlpRecAttrDeleted; }
		else   { fAttrib &= ~dlpRecAttrDeleted; } }
	inline void setSecret(bool s=true) {
		if (s) { fAttrib |= dlpRecAttrSecret; }
		else   { fAttrib &= ~dlpRecAttrSecret; } }
	inline void makeDeleted() KDE_DEPRECATED { setDeleted(true); }
	inline void makeSecret() KDE_DEPRECATED { setSecret(true); } ;

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

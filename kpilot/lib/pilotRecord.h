#ifndef _KPILOT_PILOTRECORD_H
#define _KPILOT_PILOTRECORD_H
/* pilotRecord.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
*/

/**
** @file This file defines the class PilotRecord, which is the lowest-
** denominator representation of the bits used in a Pilot-based
** database record. The PilotRecord is @em just a collection of
** bits, nothing more. It can be converted to an interpreted form
** using some other classes like PilotAppCategory.
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

struct pi_buffer_t;

#include <pi-dlp.h>
#include <pi-file.h>

/** An "uninterpreted" representation of the bits comprising a HH record.
* This class maintains a created and deleted count which can be requested
* using allocationInfo().
*/
class KDE_EXPORT PilotRecord
{
public:
	/** Constructor. Using the given @p data and @p length, create
	* a record. Give it the additional attributes and category numbers;
	* the UID is a HH unique ID for identifying records.
	*
	* This constructor makes a copy of the data buffer (and owns that buffer).
	*/
	PilotRecord(void* data, int length, int attrib, int cat, recordid_t uid);

#if PILOT_LINK_NUMBER >= PILOT_LINK_0_12_0
	/** Constructor. Using the given buffer @p buf (which carries its
	* own data and length), create a record. Otherwise much like the
	* above constructor @em except that this record assumes ownership
	* of the buffer, and doesn't make an additional copy
	* (In practice, this just saves copying around extra buffers).
	*/
	PilotRecord(pi_buffer_t *buf, int attrib, int cat, recordid_t uid) :
		fData((char *)buf->data),fLen(buf->used),fAttrib(attrib),
		fCat(cat),fID(uid),fBuffer(buf)
	{ fAllocated++; }
#endif

	/** Destructor. Dispose of the buffers in the right form. */
	~PilotRecord()
	{
#if PILOT_LINK_NUMBER >= PILOT_LINK_0_12_0
		if (fBuffer) { pi_buffer_free(fBuffer); } else
#endif
		{ delete [] fData; }
		fDeleted++;
	}

	/** Constructor. Copies the data from the @p orig record. */
	PilotRecord(PilotRecord* orig);

	/** Retrieve the data buffer for this record. Note that trying
	* to change this data is fraught with peril -- especially trying
	* to lengthen it.
	*
	* @see setData
	*/
	char *getData() const
	{
#if PILOT_LINK_NUMBER >= PILOT_LINK_0_12_0
		if (fBuffer) return (char *)(fBuffer->data); else
#endif
		return fData;
	}

	/** Returns the length of the data for this record. */
	int getLen() const
	{
#if PILOT_LINK_NUMBER >= PILOT_LINK_0_12_0
		if (fBuffer) return fBuffer->used; else
#endif
		return fLen;
	}

#if PILOT_LINK_NUMBER >= PILOT_LINK_0_12_0
	/** Returns the data buffer associated with this record. */
	const pi_buffer_t *buffer() const { return fBuffer; }

	/** Set the data for this record. Frees old data. Assumes
	* ownership of the passed in buffer @p b.
	*/
	void setData(pi_buffer_t *b)
	{
		if (fBuffer) { pi_buffer_free(fBuffer); }
		else { delete[] fData; } ;
		fData = 0L;
		fBuffer = b;
	}
#endif

	/** A constant, really left over from PalmOS 4 days, when records
	* could be 64k in size at most. It is used in various places to
	* dimension buffers, but should be considered deprecated.
	*/
	enum { APP_BUFFER_SIZE = 0xffff } ;

	/** Assignment operator. Makes a copy of the @p orig record. */
	PilotRecord& operator=(PilotRecord& orig);

	/** Sets the data for this record. Makes a copy of the data buffer. */
	void setData(const char* data, int len);


	inline int   getAttrib() const { return fAttrib; }
	inline void  setAttrib(int attrib) { fAttrib = attrib; }

	int   category() const { return fCat; }
	void  setCategory(int cat) { fCat = cat; }
	int   getCat() const KDE_DEPRECATED;
	void  setCat(int cat) KDE_DEPRECATED;

	inline recordid_t id() const { return fID; }
	recordid_t getID() const KDE_DEPRECATED;
	void setID(recordid_t id) { fID = id; }

private:
	char* fData;
	int   fLen;
	int   fAttrib;
	int   fCat;
	recordid_t fID;
#if PILOT_LINK_NUMBER >= PILOT_LINK_0_12_0
	pi_buffer_t *fBuffer;
#endif

public:
	inline bool isDeleted() const { return fAttrib & dlpRecAttrDeleted; };
	inline bool isSecret() const { return fAttrib & dlpRecAttrSecret; } ;
	inline bool isArchived() const { return fAttrib & dlpRecAttrArchived; } ;
	inline bool isDirty() const { return fAttrib & dlpRecAttrDirty; } ;
	inline void setDeleted(bool d=true) {
		if (d) { fAttrib |= dlpRecAttrDeleted; }
		else   { fAttrib &= ~dlpRecAttrDeleted; } }
	inline void setSecret(bool s=true) {
		if (s) { fAttrib |= dlpRecAttrSecret; }
		else   { fAttrib &= ~dlpRecAttrSecret; } }
	void makeDeleted() KDE_DEPRECATED;
	void makeSecret() KDE_DEPRECATED;

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

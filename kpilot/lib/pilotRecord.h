#ifndef _KPILOT_PILOTRECORD_H
#define _KPILOT_PILOTRECORD_H
/* pilotRecord.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
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

/**
* @file This file defines the lowest- denominator representation(s)
*of the bits used in a Pilot-based database record.
*/


#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <kdemacros.h>

#include "pilotLinkVersion.h"

struct pi_buffer_t;

#include <pi-dlp.h>
#include <pi-file.h>

#define CATEGORY_NAME_SIZE 16 // (sizeof(((struct CategoryAppInfo *)0)->name[0]))
#define CATEGORY_COUNT     16 // ( (sizeof(((struct CategoryAppInfo *)0)->name)) / CATEGORY_NAME_SIZE )


/** All entries in the Handheld -- whether interpreted or binary blobs --
* have some common characteristics, viz. an ID number, a category,
* and some attributes defined by the handheld. PilotRecordBase is
* a common base class collecting methods to manipulate those
* common characteristics.
*/
class KDE_EXPORT PilotRecordBase
{
public:
	/** Constructor. Initialize the characteristics to the
	* given values.
	* @param attrib Attributes (bitfield) for this entry.
	* @param cat Category for this entry. Should be between 0 and 16
	*        (CATEGORY_COUNT), but this is not enforced.
	* @param id Unique ID for this entry. May be 0 (non-unique) as well.
	*/
	PilotRecordBase(int attrib=0, int cat=0, recordid_t id=0) :
		fAttrib(attrib),fCat(cat),fID(id) {}

	/** Attributes of this record (deleted, secret, ...); it's a bitfield. */
	inline int   attributes() const { return fAttrib; }
	/** Set the attributes of this record. */
	inline void  setAttributes(int attrib) { fAttrib = attrib; }
	int KDE_DEPRECATED getAttrib() const { return attributes(); }
	void KDE_DEPRECATED setAttrib(int attrib) { setAttributes(attrib); }

	/** Returns the category number (0..15) of this record. */
	int   category() const { return fCat; }
	/** Sets the category number (0..15) of this record. Trying to set an illegal
	* category number files this one under "Unfiled" (which is 0).
	*/
	void  setCategory(int cat) { if ( (cat<0) || (cat>=CATEGORY_COUNT)) cat=0; fCat = cat; }
	int  KDE_DEPRECATED  getCat() const { return category(); }
	void KDE_DEPRECATED  setCat(int cat) { return setCategory(cat); }

	/** Returns the record ID for this record. Record IDs are unique for a given
	* handheld and database.
	*/
	inline recordid_t id() const { return fID; }
	/** Sets the record ID for this record. Use with caution -- you ca confuse
	* the handheld by doing weird things here.
	*/
	void setID(recordid_t id) { fID = id; }
	recordid_t KDE_DEPRECATED getID() const { return id(); }

	/** Accessor for one bit of the record's attributes. Is this record marked
	* deleted (on the handheld) ? Deleted records are not removed from the
	* database until a HotSync is done (which normally calls purge deleted
	* or so to really get rid of the records from storage.
	*/
	inline bool isDeleted() const { return fAttrib & dlpRecAttrDeleted; };
	/** Accessor for one bit of the record's attributes. Is this record secret?
	* Secret records are not displayed on the desktop by default.
	*/
	inline bool isSecret() const { return fAttrib & dlpRecAttrSecret; } ;
	/** Accessor for one bit of the record's attributes. Is this record a
	* to-be-archived record? When a record is deleted, it may be marked
	* as "archive on PC" which means the PC should keep a copy. The
	* PC data correspondng to an archived-but-deleted record must not
	* be deleted.
	*/
	inline bool isArchived() const { return fAttrib & dlpRecAttrArchived; } ;
	/** Accessor for one bit of the record's attributes. Is this record modified?
	* Modified records are those that have been modified since the last HotSync.
	*/
	inline bool isModified() const { return fAttrib & dlpRecAttrDirty; }
	inline bool KDE_DEPRECATED isDirty() const { return isModified(); } ;

#define SETTER(a) {\
		if (d) { fAttrib |= a; } \
		else   { fAttrib &= ~a; } }

	/** Mark a record as deleted (or not).*/
	inline void setDeleted(bool d=true) SETTER(dlpRecAttrDeleted)

	/** Mark a record as secret (or not). */
	inline void setSecret(bool d=true) SETTER(dlpRecAttrSecret)

	/** Mark a record as archived (or not). */
	inline void setArchived(bool d=true) SETTER(dlpRecAttrArchived)

	/** Mark a record as modified (or not). */
	inline void setModified(bool d=true) SETTER(dlpRecAttrDirty)

	void KDE_DEPRECATED makeDeleted() { setDeleted(true); }
	void KDE_DEPRECATED makeSecret() { setSecret(true); }
	void KDE_DEPRECATED makeArchived() { setArchived(true); }
#undef SETTER

private:
	int fAttrib, fCat;
	recordid_t fID;
} ;

/** An "uninterpreted" representation of the bits comprising a HH record.
* This binary blob only exposes the data via the data() and size() functions,
* and also exposes the common characteristics of all entries.
*/
class KDE_EXPORT PilotRecord : public PilotRecordBase
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
		PilotRecordBase(attrib,cat,uid),
		fData((char *)buf->data),
		fLen(buf->used),
		fBuffer(buf)
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
	char *data() const
	{
#if PILOT_LINK_NUMBER >= PILOT_LINK_0_12_0
		if (fBuffer) return (char *)(fBuffer->data); else
#endif
		return fData;
	}
	char *KDE_DEPRECATED getData() const { return data(); }

	/** Returns the size of the data for this record. */
	int size() const
	{
#if PILOT_LINK_NUMBER >= PILOT_LINK_0_12_0
		if (fBuffer) return fBuffer->used; else
#endif
		return fLen;
	}
	int KDE_DEPRECATED getLen() const { return size(); }

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
		fData = (char *)b->data;
		fLen = b->used;
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

private:
	char* fData;
	int   fLen;
#if PILOT_LINK_NUMBER >= PILOT_LINK_0_12_0
	pi_buffer_t *fBuffer;
#endif

public:
	/**
	* This is an interface for tracking down memory leaks
	* in the use of PilotRecords (for those without valgrind).
	* Count the number of allocations and deallocations.
	*/
	static void allocationInfo();
private:
	static int fAllocated,fDeleted;
};

#endif

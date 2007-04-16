#ifndef _KPILOT_PILOTRECORD_H
#define _KPILOT_PILOTRECORD_H
/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "pilot.h"

/**
* @file This file defines the lowest- denominator representation(s)
*of the bits used in a Pilot-based database record.
*/


/**
* All entries in the Handheld -- whether interpreted or binary blobs --
* have some common characteristics, viz. an ID number, a category,
* and some attributes defined by the handheld. PilotRecordBase is
* a common base class collecting methods to manipulate those
* common characteristics.
*/
class KDE_EXPORT PilotRecordBase
{
public:
	/** Constructor. Initialize the characteristics to the
	*   given values.
	*
	* @param attrib Attributes (bitfield) for this entry.
	* @param cat Category for this entry. Should be in the
	*        range 0 <= cat < Pilot::CATEGORY_COUNT . Using an
	*        invalid category means 0 (unfiled) is used.
	* @param id Unique ID for this entry. May be 0 (non-unique) as well.
	*/
	PilotRecordBase(int attrib=0, int cat=0, recordid_t id=0) :
		fAttrib(attrib),fCat(0),fID(id)
	{
		setCategory(cat);
	}

	/** Constructor. Initializes the characteristics from
	 *  the values the given record @p b has. When @p b is
	 *  NULL (which is allowed), everything is assumed zero.
	 *  No ownership is transferred.
	 *
	 *  @param b Record to take characteristics from.
	 */
	PilotRecordBase( const PilotRecordBase *b ) :
		fAttrib( b ? b->attributes() : 0 ),
		fCat( 0 ),
		fID( b ? b->id() : 0 )
	{
		if (b)
		{
			setCategory( b->category() );
		}
	}

	/** Destructor. Nothing to do for it. */
	virtual ~PilotRecordBase() { } ;

	/** Attributes of this record (deleted, secret, ...);
	* it's a bitfield.
	*/
	inline int attributes() const
	{
		return fAttrib;
	}

	/** Set the attributes of this record. */
	inline void  setAttributes(int attrib)
	{
		fAttrib = attrib;
	}

	/** Returns the category number [ 0 .. Pilot::CATEGORY_COUNT-1]
	* of this record.
	*/
	inline int   category() const
	{
		return fCat;
	}

	/** Sets the category number [ 0 .. Pilot::CATEGORY_COUNT-1]
	* of this record.
	* Trying to set an illegal category number files this one under
	* "Unfiled" (which is 0).
	*/
	inline void  setCategory(int cat)
	{
		if ( (cat<0) || (cat>=(int)Pilot::CATEGORY_COUNT))
		{
			cat=0;
		}
		fCat = cat;
	}

	/** Sets the category number by looking up the string @p label
	* in the category table @p info . Leaves the category unchanged
	* if no match is found and returns @c false.
	*
	* @param info AppInfo structure containing the labels (in handheld
	*        native encoding).
	* @param label The label to look for.
	*
	* @return @c true on success, @c false on failure
	*/
	bool setCategory(const struct CategoryAppInfo *info, const QString &label)
	{
		if (!info)
		{
			return false;
		}

		int cat = Pilot::findCategory( info, label, false );
		if ( (cat<0) || (cat>=(int)Pilot::CATEGORY_COUNT) )
		{
			return false;
		}
		else
		{
			setCategory( cat );
			return true;
		}
	}

	/** Returns the record ID for this record. Record IDs are unique for a given
	* handheld and database.
	*/
	inline recordid_t id() const
	{
		return fID;
	}

	/** Sets the record ID for this record. Use with caution -- you ca confuse
	* the handheld by doing weird things here.
	*/
	void setID(recordid_t id)
	{
		fID = id;
	}

	/** Accessor for one bit of the record's attributes. Is this record marked
	* deleted (on the handheld) ? Deleted records are not removed from the
	* database until a HotSync is done (which normally calls purge deleted
	* or so to really get rid of the records from storage.
	*/
	inline bool isDeleted() const
	{
		return fAttrib & dlpRecAttrDeleted;
	}

	/** Accessor for one bit of the record's attributes. Is this record secret?
	* Secret records are not displayed on the desktop by default.
	*/
	inline bool isSecret() const
	{
		return fAttrib & dlpRecAttrSecret;
	}

	/** Accessor for one bit of the record's attributes. Is this record a
	* to-be-archived record? When a record is deleted, it may be marked
	* as "archive on PC" which means the PC should keep a copy. The
	* PC data correspondng to an archived-but-deleted record must not
	* be deleted.
	*/
	inline bool isArchived() const
	{
		return fAttrib & dlpRecAttrArchived;
	}

	/** Accessor for one bit of the record's attributes. Is this record modified?
	* Modified records are those that have been modified since the last HotSync.
	*/
	inline bool isModified() const
	{
		return fAttrib & dlpRecAttrDirty;
	}

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

#undef SETTER

	/** Returns a text representation of this record. */
	virtual QString textRepresentation() const;

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
	PilotRecord(void* data, int length, int attrib, int cat, recordid_t uid) KDE_DEPRECATED;

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
	{
		fAllocated++;
	}

	/** Constructor. Like the above, only take the attributes, category
	* and id from the given @p entry.
	*/
	PilotRecord( pi_buffer_t *buf, const PilotRecordBase *entry ) :
		PilotRecordBase( entry ),
		fData((char *)buf->data),
		fLen(buf->used),
		fBuffer(buf)
	{
		fAllocated++;
	}

	/** Destructor. Dispose of the buffers in the right form. */
	virtual ~PilotRecord()
	{
		if (fBuffer)
		{
			pi_buffer_free(fBuffer);
		}
		else
		{
			delete [] fData;
		}
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
		if (fBuffer)
		{
			return (char *)(fBuffer->data);
		}
		else
		{
			return fData;
		}
	}

	/** Returns the size of the data for this record. */
	int size() const
	{
		if (fBuffer) return fBuffer->used; else
		return fLen;
	}

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

	/** Assignment operator. Makes a copy of the @p orig record. */
	PilotRecord& operator=(PilotRecord& orig);

	/** Sets the data for this record. Makes a copy of the data buffer. */
	void setData(const char* data, int len);

	/** Returns a text representation of this record. */
	virtual QString textRepresentation() const;

private:
	char* fData;
	int   fLen;
	pi_buffer_t *fBuffer;

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

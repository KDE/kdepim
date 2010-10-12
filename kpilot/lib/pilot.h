#ifndef _KPILOT_PILOT_H
#define _KPILOT_PILOT_H
/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2003-2006 Adriaan de Groot <groot@kde.org>
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

#include <sys/types.h>

#include <pi-appinfo.h>
#include <pi-buffer.h>
#include <pi-dlp.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "pilotLinkVersion.h"


/** @file
* These are some base structures that reside on the
* handheld device -- strings and binary data.
*/

class PilotDatabase;     // A database
class PilotRecord;       // ... has records
class PilotCategoryInfo; // ... and category information

#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))

/**
* The Pilot namespace holds constants that are global for
* the handheld data structures. Also contains some global
* functions that deal with pilot-link structures as well
* as mapping user-visible strings from UTF8 (KDE side) to
* the encoding used on the handheld.
*/
namespace Pilot
{
	/** Maximum size of an AppInfo block, taken roughly from the pilot-link source. */
	static const int MAX_APPINFO_SIZE=8192;

	/** Maximum number of categories the handheld has */
	static const unsigned int CATEGORY_COUNT=16;

	/** Maximum size of a category label */
	static const unsigned int CATEGORY_SIZE=16;

	/** Category number for unfiled records */
	static const int Unfiled = 0;

	/** Maximum size (in bytes) of a record's data */
	static const int MAX_RECORD_SIZE = 65535;

	typedef QValueList<recordid_t> RecordIDList;

	/** Static translation function that maps handheld native (8 bit,
	* usually latin1 but sometimes someting else) encoded data to
	* a Unicode string. Converts the @p len characters in @p c
	* to a Unicode string.
	*/
	QString fromPilot( const char *c, int len );

	/** Static translation function mapping a NUL-terminated
	* string from the handheld's encoding to UTF-8.
	* @param c the NUL-terminated string to decode
	* @return QString (UTF-8) value of @p c
	* @note NUL-terminated strings are rare on the handheld.
	*/
	QString fromPilot( const char *c );

	/** Static translation function that maps a QString onto the
	* native 8 bit encoding of the handheld. Writes the result into
	* the buffer @p buf which has size @p len. Returns the length
	* of the result. Zero-fills the buffer as needed.
	*/
	int toPilot( const QString &s, char *buf, int len);
	int toPilot( const QString &s, unsigned char *buf, int len);

	/** Static translation function that maps a QString onto the
	* native 8 bit encoding of the handheld.
	*
	* @param s String to encode
	* @return Encoded string in a QCString
	*/
	QCString toPilot( const QString &s );

	/** Create a codec for translating handheld native 8 bit to Unicode,
	* using the given codec @p name -- this will often be latin1, but
	* might be something else for, say, Russian-language Pilots.
	* If @p name is empty, use latin1.
	*
	* @return @c true on success, @c false otherwise
	*/
	bool setupPilotCodec(const QString &name);

	/** Returns the name of the codec being used. */
	QString codecName();

	/** For debugging, display category names for the given AppInfo
	* structure. Called by dump(). You must pass a valid reference.
	*/
	void dumpCategories(const struct CategoryAppInfo *info);

	/** Check that a given category number is valid. This
	* restricts the range of integers to [0..CATEGORY_COUNT-1]
	* (i.e. [0..15]) which is what the handheld supports.
	*/
	inline bool validCategory(int c)
	{
		if (c<0)
		{
			return false;
		}
		return ((unsigned int)c<CATEGORY_COUNT);
	}

	/** Returns the QString for the requested category @p i
	* in the category structure @p info. Returns @c QString::null
	* on error (bad pointer or bad category number). May also
	* return @c QString::null if the category name is empty.
	*/
	inline QString categoryName(const struct CategoryAppInfo *info, unsigned int i)
	{
		if ( ( i < CATEGORY_COUNT ) && ( info->name[i][0] ) )
		{
			/*
			 * Seems to be important that we try to pass the real length here
			 * to the codec.
			 */
			return fromPilot( info->name[i], MIN(strlen(info->name[i]),CATEGORY_SIZE) );
		}
		else
		{
			return QString::null;
		}
	}

	/** Returns a list of all the category names available on the
	*  handheld. This list is neither ordered nor does it contain
	*  all sixteen categories -- empty category names on the
	*  handheld are skipped.
	*/
	inline QStringList categoryNames(const struct CategoryAppInfo *info)
	{
		QStringList l;
		if (!info)
		{
			return l;
		}
		for (unsigned int i=0; i<CATEGORY_COUNT; ++i)
		{
			QString s = categoryName(info,i);
			if (!s.isEmpty())
			{
				l.append(s);
			}
		}
		return l;
	}

	/** Search for the given category @p name in the list
	* of categories; returns the category number. If @p unknownIsUnfiled
	* is true, then map unknown categories to Unfiled instead of returning
	* an error number.
	*
	* @return >=0   is a specific category based on the text-to-
	*               category number mapping defined by the Pilot,
	*               where 0 is always the 'unfiled' category.
	*  @return -1   means unknown category selected when
	*               @p unknownIsUnfiled is false.
	*  @return  0   == Unfiled means unknown category selected when
	*               @p unknownIsUnfiled is true.
	*
	*/
	int findCategory(const struct CategoryAppInfo *info, const QString &name, bool unknownIsUnfiled);

	/** Search for the given category @p name in the list
	* of categories; returns the category number. If @p unknownIsUnfiled
	* is @c true, then map unknown categories to Unfiled.
	* If @p unknownIsUnfiled is @c false, insert a @em new
	* category into the structure and return the category
	* number of the new category. Return -1 if (and only if)
	* @p unknownIsUnfiled is false and the category structure
	* is already full.
	*
	* @return >=0   is a specific category based on the text-to-
	*               category number mapping defined by the Pilot,
	*               where 0 is always the 'unfiled' category.
	* @return 0     Unknown category and @p unknownIsUnfiled is @c true
	* @return -1    means unknown category selected when
	*               @p unknownIsUnfiled is false and categories
	*               are all full.
	*
	*/
	int insertCategory(struct CategoryAppInfo *info, const QString &label, bool unknownIsUnfiled);

	/** The handheld also holds data about each database
	* in a DBInfo structure; check if the database described
	* by this structure is a resource database.
	*/
	static inline bool isResource(struct DBInfo *info)
	{
		return (info->flags & dlpDBFlagResource);
	}


/** @section Binary blob handling
*
* For reading and writing binary blobs -- which has to happen to
* pack data into the format that the handheld needs -- it is important
* to remember that the handheld has only four data types (as far
* as I can tell: byte, short (a 2 byte integer), long (a 4 byte integer)
* and string (NUL terminated). The sizes of the types on the handheld
* do not necessarily correspond to the sizes of the same-named types
* on the desktop. This means that 'reading a long' from a binary
* blob must always be 4 bytes -- not sizeof(long).
*
* The following templates help out in manipulating the blobs.
* Instantiate them with the type @em name you need (char, short, long or
* char *) and you get a ::size enum specifying the number of bytes
* (where applicable) and ::append and ::read methods for appending
* a value of the given type to a pi_buffer_t or reading one from
* the buffer, respectively.
*
* The usage of ::read and ::append is straightforward:
*
* append(pi_buffer_t *b, TYPE_VALUE v) Appends the type value @p v to the
* buffer @p b , extending the buffer as needed.
*
* TYPE_VALUE read(pi_buffer_t *b, unsigned int &offset) Read a value from
* the buffer @p b at position @p offset and return it. The offset value
* is increased by the number of bytes read from the buffer.
*
* To write a binary blob, a sequence of ::append calls constructs the
* blob. To read the same blob, a sequence of ::read calls with the
* @em same type parameters is sufficient.
*
* The calls may vary a little: the exact interface differs depending
* on the needs of the type of data to be written to the blob.
*/
template<typename t> struct dlp { } ;

template<> struct dlp<char>
{
	enum { size = 1 };

	static void append(pi_buffer_t *b, char v)
	{
		pi_buffer_append(b,&v,size);
	}

	/**
	* Returns next byte from buffer or 0 on error (0 is also a
	* valid return value, though).
	*/
	static char read(const pi_buffer_t *b, unsigned int &offset)
	{
		if (offset+size > b->used)
		{
			return 0;
		}
		char c = b->data[offset];
		offset+=size;
		return c;
	}
} ;

template<> struct dlp<short>
{
	enum { size = 2 };

	static void append(pi_buffer_t *b, short v)
	{
		char buf[size];
		set_short(buf,v);
		pi_buffer_append(b,buf,size);
	}

	/**
	* Returns the next short (2 byte) value from the buffer, or
	* -1 on error (which is also a valid return value).
	*/
	static int read(const pi_buffer_t *b, unsigned int &offset)
	{
		if (offset+size > b->used)
		{
			return -1;
		}
		else
		{
			int r = get_short(b->data + offset);
			offset+=size;
			return r;
		}
	}

	/**
	* Overload to read from a data buffer instead of a real pi_buffer;
	* does no bounds checking.
	*/
	static int read(const unsigned char *b, unsigned int &offset)
	{
		int r = get_short(b+offset);
		offset+=size;
		return r;
	}
} ;

template<> struct dlp<long>
{
	enum { size = 4 };

	static void append(pi_buffer_t *b, int v)
	{
		char buf[size];
		set_long(buf,v);
		pi_buffer_append(b,buf,size);
	}

	/**
	* Returns the next long (4 byte) value from the buffer or
	* -1 on error (which is also a valid value).
	*/
	static int read(const pi_buffer_t *b, unsigned int &offset)
	{
		if (offset+size > b->used)
		{
			return -1;
		}
		else
		{
			int r = get_long(b->data + offset);
			offset+=size;
			return r;
		}
	}

	/**
	* Overload to read a long value from a data buffer; does
	* no bounds checking.
	*/
	static int read(const unsigned char *b, unsigned int &offset)
	{
		int r = get_long(b+offset);
		offset+=size;
		return r;
	}
} ;

template<> struct dlp<char *>
{
	// No size enum, doesn't make sense
	// No append, use pi_buffer_append
	/**
	* Read a fixed-length string from the buffer @p b into data buffer
	* @p v which has size (including terminating NUL) of @p s.
	* Returns the number of bytes read (which will normally  be @p s
	* but will be less than @p s on error).
	*/
	static int read(const pi_buffer_t *b,
		unsigned int &offset,
		unsigned char *v,
		size_t s)
	{
		if ( s+offset > b->used )
		{
			s = b->allocated - offset;
		}
		memcpy(v, b->data + offset, s);
		offset+=s;
		return s;
	}

	/** Overload for signed char. */
	inline static int read(const pi_buffer_t *b, unsigned int &offset, char *v, size_t s)
	{
		return read(b,offset,(unsigned char *)v,s);
	}
} ;

}

#endif


#ifndef _KPILOT_PILOT_H
#define _KPILOT_PILOT_H
/* pilot.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2003-2006 Adriaan de Groot <groot@kde.org>
**
** These are the base class structures that reside on the
** handheld device -- databases and their parts.
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

class QTextCodec;

class PilotDatabase;     // A database
class PilotRecord;       // ... has records
class PilotCategoryInfo; // ... and category information

#include "pilotLinkVersion.h"

#include <stdio.h>

#include <pi-dlp.h>
#include <pi-file.h>
#include <pi-appinfo.h>
#include <pi-buffer.h>

#include <qstring.h>
#include <qvaluelist.h>

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

	/** Returns the QString for the requested category @p i
	* in the category structure @p info. Returns @c QString::null
	* on error (bad pointer or bad category number). May also
	* return @c QString::null if the category name is empty.
	*/
	inline QString categoryName(const struct CategoryAppInfo *info, unsigned int i)
	{
		if ( i < CATEGORY_COUNT )
		{
			return fromPilot( info->name[i], CATEGORY_SIZE );
		}
		else
		{
			return QString::null;
		}
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
}


template<typename t> struct dlp { } ;
template<> struct dlp<short> 
{ 
	enum { size = 2 }; 

	static void append(pi_buffer_t *b, short v)
	{
		char buf[size];
		set_short(buf,v);
		pi_buffer_append(b,buf,size);
	}

	static int read(const pi_buffer_t *b, unsigned int &offset)
	{
		if ((offset>=b->used) || (offset>=b->allocated))
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

	static int read(const pi_buffer_t *b, unsigned int &offset)
	{
		if ((offset>=b->used) || (offset>=b->allocated))
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
	static int read(const pi_buffer_t *b, unsigned int &offset, unsigned char *v, size_t s)
	{
		if ( s+offset > b->allocated )
		{
			s = b->allocated - offset;
		}
		memcpy(v, b->data + offset, s);
		offset+=s;
		return s;
	}

	inline static int read(const pi_buffer_t *b, unsigned int &offset, char *v, size_t s)
	{
		return read(b,offset,(unsigned char *)v,s);
	}
} ;

#endif


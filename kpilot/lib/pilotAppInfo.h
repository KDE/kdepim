#ifndef _KPILOT_PILOTAPPINFO_H
#define _KPILOT_PILOTAPPINFO_H
/* pilotAppInfo.h			KPilot
**
** Copyright (C) 2005-2006 Adriaan de Groot <groot@kde.org>
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

#include "pilotLinkVersion.h"

#include "pilot.h"
#include "pilotDatabase.h"

/**
* A database on the handheld has an "AppInfo" block at the beginning
* with some database-specific information and a common part.
* This base class deals with the common part, the categories.
*
* Most data in the handheld is stored in @em categories ; every
* record in every database, for instance, has a category assigned
* to it (perhaps "Unfiled", but that's just another category).
*
* Every database has a category table assigning labels to the
* categories that exist. There are CATEGORY_COUNT (16) categories
* available for each database; labels may vary per database.
*
* This class encapsulates the basic category table manipulations.
*/
class KDE_EXPORT PilotAppInfoBase
{
protected:
	/** Initialize class members after reading header, to alias data elsewhere.
	* Only for use by the (derived) template classes below.
	*/
	void init(struct CategoryAppInfo *c, int len)
	{
		fC = c;
		fLen = len ;
	} ;

public:
	/** Constructor. This is for use by derived classes (using the template below
	* only, and says that the category info in the base class aliases data in
	* the derived class. Remember to call init()!
	*/
	PilotAppInfoBase() : fC(0L), fLen(0), fOwn(false) { } ;

	/** Constructor, intended for untyped access to the AppInfo only. This throws
	* away everything but the category information. In this variety, the
	* CategoryAppInfo structure is owned by the PilotAppInfoBase object.
	*/
	PilotAppInfoBase(PilotDatabase *d);

	/** Destructor. */
	virtual ~PilotAppInfoBase();

	/** Retrieve the most basic part of the AppInfo block -- the category
	* information which is guaranteed to be the first 240-odd bytes of
	* a database.
	*/
	struct CategoryAppInfo *categoryInfo()
	{
		return fC;
	} ;

	/** Const version of the above function. */
	inline const struct CategoryAppInfo *categoryInfo() const
	{
		return fC;
	} ;

	/** Returns the length of the (whole) AppInfo block. */
	inline PI_SIZE_T length() const
	{
		return fLen;
	} ;

	/** @see findCategory(const QString &name, bool unknownIsUnfiled, struct CategoryAppInfo *info). */
	inline int findCategory(const QString &name, bool unknownIsUnfiled = false) const
	{
		return Pilot::findCategory(fC,name,unknownIsUnfiled);
	} ;

	/** Gets a single category name. Returns QString::null if there is no
	* such category number @p i . */
	inline QString categoryName(unsigned int i) const
	{
		return Pilot::categoryName(fC,i);
	}

	/** Sets a category name. @return true if this succeeded. @return false
	* on failure, e.g. the index @p i was out of range or the category name
	* was invalid. Category names that are too long are truncated to 15 characters.
	*/
	bool setCategoryName(unsigned int i, const QString &s);

	/** For debugging, display all the category names */
	inline void dump() const
	{
		Pilot::dumpCategories(fC);
	};

protected:
	struct CategoryAppInfo *fC;
	PI_SIZE_T fLen;

	bool fOwn;
} ;

/** A template class for reading and interpreting AppInfo blocks;
* the idea is that it handles all the boilerplate code for reading
* the app block, converting it to the right kind, and then unpacking
* it. Template parameters are the type (struct, from pilot-link probably)
* of the interpreted appinfo, and the pack and unpack functions for it
* (again, from pilot-link).
*/
template <typename appinfo,
#if PILOT_LINK_IS(0,12,2)
	/* There are additional consts introduced in 0.12.2 */
	int(*unpack)(appinfo *, const unsigned char *, PI_SIZE_T),
	int(*pack)(const appinfo *, unsigned char *, PI_SIZE_T)
#else
	int(*unpack)(appinfo *, unsigned char *, PI_SIZE_T),
	int(*pack)(appinfo *, unsigned char *, PI_SIZE_T)
#endif
	>
class PilotAppInfo : public PilotAppInfoBase
{
public:
	/** Constructor. Read the appinfo from database @p d and
	* interpret it.
	*/
	PilotAppInfo(PilotDatabase *d) : PilotAppInfoBase()
	{
		int appLen = Pilot::MAX_APPINFO_SIZE;
		unsigned char buffer[Pilot::MAX_APPINFO_SIZE];

		memset(&fInfo,0,sizeof(fInfo));
		if (d && d->isOpen())
		{
			appLen = d->readAppBlock(buffer,appLen);
			(*unpack)(&fInfo, buffer, appLen);
			// fInfo is just a struct, so we can point to it anyway.
			init(&fInfo.category,appLen);
		}
		else
		{
			delete fC;
			fC = 0L;
			fLen = 0;
			init(&fInfo.category,sizeof(fInfo));
		}
	} ;

	PilotAppInfo()
	{
		memset(&fInfo,0,sizeof(fInfo));
		init(&fInfo.category,sizeof(fInfo));
	}


	/** Write this appinfo block to the database @p d; returns
	* the number of bytes written or -1 on failure. This
	* function is robust when called with a NULL database @p d.
	*/
	int writeTo(PilotDatabase *d)
	{
		unsigned char buffer[Pilot::MAX_APPINFO_SIZE];
		if (!d || !d->isOpen())
		{
			return -1;
		}
		int appLen = (*pack)(&fInfo, buffer, length());
		if (appLen > 0)
		{
			d->writeAppBlock(buffer,appLen);
		}
		return appLen;
	} ;

	/** Returns a (correctly typed) pointer to the interpreted
	* appinfo block.
	*/
	appinfo *info() { return &fInfo; } ;
	/** Returns a const (correctly typed) pointer to the interpreted
	* appinfo block.
	*/
	const appinfo *info() const { return &fInfo; } ;

protected:
	appinfo fInfo;
} ;


#endif

#ifndef _KPILOT_PILOTAPPCATEGORY_H
#define _KPILOT_PILOTAPPCATEGORY_H
/* pilotAppCategory.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** The class PilotAppCategory is the base class for "interpretations"
** of a PilotRecord. This is where the records change from a collection
** of bits to something with meaning. Subclasses of PilotAppCategory
** give specific meaning to records from specific databases.
**
** Almost everything is inline; as a crufty hack, the non-inline
** part of this class lives in pilotRecord.cc.
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

// #include <pi-macros.h>

#include <qstring.h>
#include <pi-appinfo.h>

#include "pilotRecord.h"

class QTextCodec;

class KDE_EXPORT PilotAppCategory
{
protected:			// Use protected since we will be subclassed
	int fAttrs;	        // Attributes on this record
	recordid_t fId;		// The unique ID this record was assigned

	/**
	* This field stores the category this record belongs to.  It will be
	* whatever the pilot said it was.  Note that if you change the categories
	* for the app by calling addCategory or removeCategory in the appInfo_t
	* class, these fields will become invalid.  We have no way to mark them
	* invalid automatically though.  If you're going to change category names
	* in your program, you can't use this field safely.
	*/
	int fCategory;		// The category ID this record belongs to

	/**
	* Pack whatever data the interpreted record holds into the given
	* buffer, of length @p size; return NULL to indicate failure,
	* otherwise @p buf. Set @p size to the actual size of data returned.
	* (all of this is dictated by the pilot-link interfaces).
	*/
	virtual void *pack(void *buf, int *size) = 0;
	virtual void unpack(const void *, int = 0) = 0;


public:
	PilotAppCategory(void) :
		fAttrs(0),
		fId(0),
		fCategory(0)
	{} ;

	PilotAppCategory(int a, recordid_t i, int c) :
		fAttrs(a),
		fId(i),
		fCategory(c)
	{} ;

	PilotAppCategory(const PilotRecord* rec) :
		fAttrs((rec)?rec->getAttrib():0),
		fId((rec)?rec->id():0),
		fCategory((rec)?rec->category():0)
	{} ;

	PilotAppCategory(const PilotAppCategory &copyFrom) :
		fAttrs(copyFrom.fAttrs),
		fId(copyFrom.fId),
		fCategory(copyFrom.fCategory)
	{} ;

	PilotAppCategory& operator=( const PilotAppCategory &r )
	{
		fAttrs = r.fAttrs;
		fId = r.fId;
		fCategory = r.fCategory;
		return *this;
	} ;

	bool operator==(const PilotAppCategory &compareTo)
	{
		return (fAttrs==compareTo.fAttrs && fId==compareTo.fId && fCategory==compareTo.fCategory);
	} ;

	virtual ~PilotAppCategory(void) {};

	/** @return a PilotRecord that contains all of the info of the
	*  subclass.  Remember to delete the PilotRecord when finished.
	*/
	virtual PilotRecord* pack()
	{
		int len = PilotRecord::APP_BUFFER_SIZE;
		void* buff = new unsigned char[len];
		pack(buff, &len);
		PilotRecord* rec =  new PilotRecord(buff, len, getAttrib(), getCat(), id());
		delete [] (unsigned char*)buff;
		return rec;
	}

	int getAttrib(void) const { return fAttrs; }
	int getCat(void) const { return fCategory; }
	void setCat(int cat) { fCategory = cat; }
	recordid_t id(void) const { return fId; }
	recordid_t getID() { return id(); } // Just for compatability's sake
	recordid_t getID() const { return id(); } // Just for compatability's sake
	void setID(recordid_t id) { fId = id; }
	void setAttrib(int attrib) { fAttrs = attrib; }

	virtual QString getTextRepresentation(bool=false) {return i18n("Unknown record type");};

public:
	bool isSecret() const { return fAttrs & dlpRecAttrSecret ; } ;
	bool isDeleted() const { return fAttrs & dlpRecAttrDeleted ; } ;
	bool isArchived() const { return fAttrs & dlpRecAttrArchived ; } ;
	void makeSecret() { fAttrs |= dlpRecAttrSecret; } ;
	void makeDeleted() { fAttrs |= dlpRecAttrDeleted ; } ;
	void makeArchived() { fAttrs |= dlpRecAttrArchived ; } ;
	bool isModified() const { return fAttrs & dlpRecAttrDirty; }

protected:
	static QTextCodec *pilotCodec;
public:
	static QTextCodec *codec()
		{ if (pilotCodec) return pilotCodec; else return setupPilotCodec(QString::null); } ;
	static QTextCodec *setupPilotCodec(const QString &);
	static QString codecName();


public:
	bool setCat(struct CategoryAppInfo &info,const QString &label);

#ifdef DEBUG
	static void dumpCategories(const struct CategoryAppInfo &info)
	{
		FUNCTIONSETUP;
		DEBUGCONDUIT << fname << " lastUniqueId"
			<< info.lastUniqueID << endl;
		for (int i = 0; i < 16; i++)
		{
			if (!info.name[i][0]) continue;
			DEBUGCONDUIT << fname << " cat " << i << " =" <<
				info.name[i] << endl;
		}
	}
#endif
};


#endif

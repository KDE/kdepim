#ifndef _KPILOT_PILOTAPPCATEGORY_H
#define _KPILOT_PILOTAPPCATEGORY_H
/* pilotAppCategory.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

// #include <pi-macros.h>

#include "pilotRecord.h"

class PilotAppCategory
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

	virtual void *pack(void *, int *) = 0;
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

	PilotAppCategory(PilotRecord* rec) : 
		fAttrs((rec)?rec->getAttrib():0), 
		fId((rec)?rec->getID():0), 
		fCategory((rec)?rec->getCat():0) 
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

	virtual ~PilotAppCategory(void) {};

	/** @return a PilotRecord that contains all of the info of the
	*  subclass.  Remember to delete the PilotRecord when finished.
	*/
	virtual PilotRecord* pack()
	{ 
		int len = 0xffff; 
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

public:
	bool isSecret() const { return fAttrs & dlpRecAttrSecret ; } ;
	bool isDeleted() const { return fAttrs & dlpRecAttrDeleted ; } ;
	void makeSecret() { fAttrs |= dlpRecAttrSecret; } ;
	void makeDeleted() { fAttrs |= dlpRecAttrDeleted ; } ;
	bool isModified() const { return fAttrs & dlpRecAttrDirty; }
};



// $Log$
// Revision 1.1  2001/10/10 22:01:24  adridg
// Moved from ../kpilot/, shared files
//
// Revision 1.14  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.13  2001/06/11 07:35:19  adridg
// Cleanup before the freeze
//
// Revision 1.12  2001/05/01 06:03:20  stern
// Fixed bug in isModified method
//
// Revision 1.11  2001/04/30 20:48:10  stern
// Added comments
//
// Revision 1.10  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.9  2001/04/04 21:19:38  stern
// Added copy constructor and equals operator
//
// Revision 1.8  2001/03/30 17:11:31  stern
// Took out LocalDB for mode and added DatabaseSource enum in BaseConduit.  This the user can set the source for backup and sync
//
// Revision 1.7  2001/03/19 23:12:39  stern
// Made changes necessary for upcoming abbrowser conduit.
//
// Mainly, I added two public methods to PilotAddress that allow for easier
// setting and getting of phone fields.
//
// I also have added some documentation throughout as I have tried to figure
// out how everything works.
//
// Revision 1.6  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.5  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
#endif

/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __KPILOT_APP_CAT_H
#define __KPILOT_APP_CAT_H

#include <pi-macros.h>
#include "pilotRecord.h"

class PilotAppCategory
    {
    protected:			// Use protected since we will be subclassed
    int fAttrs;		        // Attributes on this record
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
    
//     virtual void *internalPack(unsigned char *) = 0;
//     virtual void *pack(int *) = 0;
    virtual void *pack(void *, int *) = 0;
    virtual void unpack(void *, int = 0) = 0;
    
    
    public:
    PilotAppCategory(void) : fAttrs(0), fId(0), fCategory(0) { }
    PilotAppCategory(int a, recordid_t i, int c) : fAttrs(a), fId(i), fCategory(c) { }
    PilotAppCategory(PilotRecord* rec) : fAttrs(rec->getAttrib()), fId(rec->getID()), fCategory(rec->getCat()) { }

    virtual ~PilotAppCategory(void) {}
    
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
    void setID(recordid_t id) { fId = id; }
    void setAttrib(int attrib) { fAttrs = attrib; }

public:
	bool isSecret() const { return fAttrs & dlpRecAttrSecret ; } ;
	bool isDeleted() const { return fAttrs & dlpRecAttrDeleted ; } ;
	void makeSecret() { fAttrs |= dlpRecAttrSecret; } ;
	void makeDeleted() { fAttrs |= dlpRecAttrDeleted ; } ;
    	
    };

#endif
